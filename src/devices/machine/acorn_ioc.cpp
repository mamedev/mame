// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller, Sandro Ronco
/**************************************************************************************************

        Acorn RISC Machine Input/Output Controller (IOC)

        TODO:
        - support IOEB used in the ARM250 (partially implemented in aristmk5.cpp)

**************************************************************************************************/

#include "emu.h"
#include "acorn_ioc.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ACORN_IOC, acorn_ioc_device, "ioc", "Acorn IOC")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

acorn_ioc_device::acorn_ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_IOC, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_peripherals_r(*this)
	, m_peripherals_w(*this)
	, m_giop_r(*this)
	, m_giop_w(*this)
	, m_irq_w(*this)
	, m_fiq_w(*this)
	, m_kout_w(*this)
	, m_baud_w(*this)
{
}

void acorn_ioc_device::device_resolve_objects()
{
	m_peripherals_r.resolve_all_safe(0xffffffff);
	m_peripherals_w.resolve_all_safe();
	m_giop_r.resolve_all_safe(1);
	m_giop_w.resolve_all_safe();
	m_irq_w.resolve_safe();
	m_fiq_w.resolve_safe();
	m_kout_w.resolve_safe();
	m_baud_w.resolve();
}

void acorn_ioc_device::device_start()
{
	for (int i=0; i <4; i++)
		m_timers[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(acorn_ioc_device::timer_tick), this));

	save_item(NAME(m_ir));
	save_item(NAME(m_if));
	save_item(NAME(m_baud));
	save_item(NAME(m_timercnt));
	save_item(NAME(m_timerout));
	save_item(NAME(m_regs));
}

void acorn_ioc_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_regs[IRQ_STATUS_A] = 0x10 | 0x80; // set up POR (Power On Reset) and Force IRQ at start-up
	m_regs[IRQ_STATUS_B] = 0x40;        // set up KART Tx empty
	m_regs[FIQ_STATUS]   = 0x80;        // set up Force FIQ

	m_ir   = CLEAR_LINE;
	m_if   = CLEAR_LINE;
	m_baud = CLEAR_LINE;

	// KART interface
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rate(31250);

	for (int i = 0; i < 6; i++)
		m_giop_w[i](1);

	for (int i=0; i < 2; i++)
	{
		m_timercnt[i] = 0;
		m_timerout[i] = 0;
		set_timer(i);
	}

	transmit_register_reset();
	receive_register_reset();
}

void acorn_ioc_device::map(address_map &map)
{
	// Typical configuration
	// IOA[2:6]   --> A[2:6]   Internal registers address
	// IOA[16:18] --> B[0:2]   Peripherals select
	// IOA[19:20] --> T[0:1]   Peripherals access timing
	// IOA[21]    --> CS       Chip select
	map(0x00200000, 0x0020007f).mirror(0x0018ff80).rw(FUNC(acorn_ioc_device::registers_r), FUNC(acorn_ioc_device::registers_w));
	map(0x00210000, 0x0021ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<1>), FUNC(acorn_ioc_device::periph_w<1>));
	map(0x00220000, 0x0022ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<2>), FUNC(acorn_ioc_device::periph_w<2>));
	map(0x00230000, 0x0023ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<3>), FUNC(acorn_ioc_device::periph_w<3>));
	map(0x00240000, 0x0024ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<4>), FUNC(acorn_ioc_device::periph_w<4>));
	map(0x00250000, 0x0025ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<5>), FUNC(acorn_ioc_device::periph_w<5>));
	map(0x00260000, 0x0026ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<6>), FUNC(acorn_ioc_device::periph_w<6>));
	map(0x00270000, 0x0027ffff).select(0x00180000).rw(FUNC(acorn_ioc_device::periph_r<7>), FUNC(acorn_ioc_device::periph_w<7>));
}

TIMER_CALLBACK_MEMBER(acorn_ioc_device::timer_tick)
{
	// all timers always run
	set_timer(param);

	// but only timers 0 and 1 generate IRQs
	switch (param)
	{
		case 0:
			change_interrupt(IRQ_STATUS_A, 0x20, ASSERT_LINE);
			break;

		case 1:
			change_interrupt(IRQ_STATUS_A, 0x40, ASSERT_LINE);
			break;

		case 2:
			m_baud ^= 1;
			m_baud_w(m_baud ? ASSERT_LINE : CLEAR_LINE);
			break;
	}
}

void acorn_ioc_device::change_interrupt(int reg, uint8_t mask, int state)
{
	if (state)
		m_regs[reg] |= mask;
	else
		m_regs[reg] &= ~mask;

	update_interrups();
}

void acorn_ioc_device::tra_complete()
{
	change_interrupt(IRQ_STATUS_B, 0x40, ASSERT_LINE);  // KART Rx empty
}

void acorn_ioc_device::rcv_complete()
{
	receive_register_extract();
	m_regs[KART] = get_received_char();
	change_interrupt(IRQ_STATUS_B, 0x80, ASSERT_LINE);  // KART Rx full
}

void acorn_ioc_device::tra_callback()
{
	m_kout_w(transmit_register_get_data_bit());
}

void acorn_ioc_device::set_timer(int tmr)
{
	double freq = 0;

	switch (tmr)
	{
		case 0: // Timers
		case 1:
			if (m_timercnt[tmr] == 0)
				m_timers[tmr]->adjust(attotime::never, tmr);
			else
				m_timers[tmr]->adjust(attotime::from_usec(m_timercnt[tmr] / 2), tmr); // TODO: ARM timings are quite off there, it should be latch and not latch/2
			break;

		case 2: // Baud generator
			freq = (double)clock() / 8 / (double)(m_timercnt[tmr] + 1);
			if (!m_baud_w.isnull())
				m_timers[tmr]->adjust(attotime::from_usec(freq), tmr);
			break;

		case 3: // KART clock
			freq = (double)clock() / 8 / (double)((m_timercnt[tmr] + 1) * 16);
			set_rate((int)freq);
			break;
	}
}

void acorn_ioc_device::latch_timer_cnt(int tmr)
{
	// find out how many 2 MHz ticks have gone by
	m_timerout[tmr] = m_timercnt[tmr] - (uint32_t)m_timers[tmr]->elapsed().as_ticks(clock() / 4);
}

WRITE_LINE_MEMBER(acorn_ioc_device::if_w)
{
	// set on falling edge
	if (m_if && !state)
		change_interrupt(IRQ_STATUS_A, 0x04, ASSERT_LINE);

	m_if = state;
}

WRITE_LINE_MEMBER(acorn_ioc_device::ir_w)
{
	// set on rising edge
	if (!m_ir && state)
		change_interrupt(IRQ_STATUS_A, 0x08, ASSERT_LINE);

	m_ir = state;
}

void acorn_ioc_device::update_interrups()
{
	if ((m_regs[IRQ_STATUS_A] & m_regs[IRQ_MASK_A]) || (m_regs[IRQ_STATUS_B] & m_regs[IRQ_MASK_B]))
		m_irq_w(ASSERT_LINE);
	else
		m_irq_w(CLEAR_LINE);

	if (m_regs[FIQ_STATUS] & m_regs[FIQ_MASK])
		m_fiq_w(ASSERT_LINE);
	else
		m_fiq_w(CLEAR_LINE);
}

uint32_t acorn_ioc_device::registers_r(offs_t offset, uint32_t mem_mask)
{
	LOG("%s: IOC R %02x = %02x\n", machine().describe_context(), offset, m_regs[offset]);

	uint8_t data = 0;
	switch (offset & 0x1f)
	{
	case CONTROL:
		// x--- ----  IR line
		// -x-- ----  IF line
		// --xx xxxx  GPIO (C0-C5)

		for (int i = 0; i < 6; i++)
			data |= m_giop_r[i]() << i;

		data |= m_if << 6;
		data |= m_ir << 7;
		return data;

	case KART:
		if (!machine().side_effects_disabled())
			change_interrupt(IRQ_STATUS_B, 0x80, CLEAR_LINE);
		return m_regs[KART];

	case IRQ_STATUS_A:
		// x--- ---- Always 1 (force IRQ)
		// -x-- ---- Timer 1
		// --x- ---- Timer 0
		// ---x ---- POR line
		// ---- x--  IR line
		// ---- -x-- IF line
		// ---- --x- IL7 line
		// ---- ---x IL6 line

		return m_regs[IRQ_STATUS_A];

	case IRQ_REQUEST_A:
		return m_regs[IRQ_STATUS_A] & m_regs[IRQ_MASK_A];

	case IRQ_MASK_A:
		return m_regs[IRQ_MASK_A];

	case IRQ_STATUS_B:
		// x--- ---- KART Rx full
		// -x-- ---- KART Tx empty
		// --xx xxxx IL0-IL5 lines

		return m_regs[IRQ_STATUS_B];

	case IRQ_REQUEST_B:
		return m_regs[IRQ_STATUS_B] & m_regs[IRQ_MASK_B];

	case IRQ_MASK_B:
		return m_regs[IRQ_MASK_B];

	case FIQ_STATUS:
		// x--- ---- Always 1 (force FIQ)
		// -x-- ---- IL0 line
		// --xx x--- C5, C4 and C3 lines
		// ---- -x-- IF line
		// ---- --xx FH0 and FH1 lines

		return m_regs[FIQ_STATUS];

	case FIQ_REQUEST:
		return m_regs[FIQ_STATUS] & m_regs[FIQ_MASK];

	case FIQ_MASK:
		return m_regs[FIQ_MASK];

	case T0_LATCH_LO:
		return m_timerout[0] & 0xff;

	case T0_LATCH_HI:
		return (m_timerout[0] >> 8) & 0xff;

	case T1_LATCH_LO:
		return m_timerout[1] & 0xff;

	case T1_LATCH_HI:
		return (m_timerout[1] >> 8) & 0xff;

	case T2_LATCH_LO:
		return m_timerout[2] & 0xff;

	case T2_LATCH_HI:
		return (m_timerout[2] >> 8) & 0xff;

	case T3_LATCH_LO:
		return m_timerout[3] & 0xff;

	case T3_LATCH_HI:
		return (m_timerout[3] >> 8) & 0xff;

	default:
		return m_regs[offset & 0x1f];
	}
}

void acorn_ioc_device::registers_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: IOC W %02x = %02x\n", machine().describe_context(), offset, data);

	// IOC uses the data bus lines D16-D23 as inputs, this also works with byte store (STRB)
	// because the ARM CPU repeats the byte four times across the data bus.
	if (ACCESSING_BITS_16_31)
		data >>= 16;

	switch (offset & 0x1f)
	{
	case CONTROL:
		for (int i = 0; i < 6; i++)
			m_giop_w[i](BIT(data, i));
		break;

	case KART:
		change_interrupt(IRQ_STATUS_B, 0x40, CLEAR_LINE);
		m_regs[KART] = data;
		transmit_register_setup(data);
		break;

	case IRQ_REQUEST_A:
		m_regs[IRQ_STATUS_A] &= ~(data & 0x7c);
		update_interrups();     // check pending irqs
		break;

	case IRQ_MASK_A:
		m_regs[IRQ_MASK_A] = data;
		update_interrups();
		break;

	case IRQ_MASK_B:
		m_regs[IRQ_MASK_B] = data;
		update_interrups();
		break;

	case FIQ_MASK:
		m_regs[FIQ_MASK] = data;
		update_interrups();
		break;

	case T0_LATCH_LO:       case T0_LATCH_HI:
	case T1_LATCH_LO:       case T1_LATCH_HI:
	case T2_LATCH_LO:       case T2_LATCH_HI:
	case T3_LATCH_LO:       case T3_LATCH_HI:
		m_regs[offset] = data;
		break;

	case T0_LATCH:  // Timer 0 latch
		latch_timer_cnt(0);
		break;

	case T1_LATCH:  // Timer 1 latch
		latch_timer_cnt(1);
		break;

	case T2_LATCH:  // Timer 2 latch
		latch_timer_cnt(2);
		break;

	case T3_LATCH:  // Timer 3 latch
		latch_timer_cnt(3);
		break;

	case T0_GO:     // Timer 0 start
		m_timercnt[0] = m_regs[T0_LATCH_HI] << 8 | m_regs[T0_LATCH_LO];
		set_timer(0);
		break;

	case T1_GO:     // Timer 1 start
		m_timercnt[1] = m_regs[T1_LATCH_HI] << 8 | m_regs[T1_LATCH_LO];
		set_timer(1);
		break;

	case T2_GO:     // Timer 2 start
		m_timercnt[2] = m_regs[T2_LATCH_HI] << 8 | m_regs[T2_LATCH_LO];
		set_timer(2);
		break;

	case T3_GO:     // Timer 3 start
		m_timercnt[3] = m_regs[T3_LATCH_HI] << 8 | m_regs[T3_LATCH_LO];
		set_timer(3);
		break;

	default:
		m_regs[offset & 0x1f] = data;
		break;
	}
}
