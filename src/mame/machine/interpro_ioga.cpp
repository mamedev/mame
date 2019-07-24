// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the IOGA (I/O Gate Array) devices found in Intergraph
 * InterPro family systems. There is no public documentation for these devices,
 * so the implementation is being built to follow the logic of the system boot
 * ROM and diagnostic tests.
 *
 * The device handles most of the I/O for the system, including timers, DMA,
 * interrupts, and target device interfacing.
 *
 * TODO
 *   - tidy up dma control flags
 *   - timer 2 and 3
 *   - correct serial dma per-channel interrupt handling
 *   - multi-channel Ethernet?
 *   - save/restore state
 */

#include "emu.h"
#include "interpro_ioga.h"

// enables hacks which allow iogadiag tests to complete but break scsi dma
#define IOGA_DMA_DIAG_HACK 0

#define LOG_GENERAL   (1U << 0)
#define LOG_NMI       (1U << 1)
#define LOG_INT       (1U << 2)
#define LOG_DMA       (1U << 3)

#define LOG_FLOPPY    (1U << 4)
#define LOG_SCSI      (1U << 5)
#define LOG_NETWORK   (1U << 6)
#define LOG_MOUSE     (1U << 7)
#define LOG_SERIALDMA (1U << 8)

#define LOG_TIMER0    (1U << 0x10)
#define LOG_TIMER1    (1U << 0x11)
#define LOG_TIMER2    (1U << 0x12)
#define LOG_TIMER3    (1U << 0x13)
#define LOG_TIMERRD   (1U << 0x14)
#define LOG_TIMERS    (LOG_TIMER0 | LOG_TIMER1 | LOG_TIMER2 | LOG_TIMER3)

//#define VERBOSE (LOG_GENERAL | LOG_INT | LOG_NMI | LOG_DMA | LOG_SERIALDMA | LOG_NETWORK)
//#define VERBOSE_IRQ ((1 << IRQ_SCSI) | (1 << IRQ_FLOPPY) | (1 << IRQ_MOUSE) | (1 << IRQ_SERDMA) | (1 << IRQ_SERIAL) | (1 << IRQ_ETHERNET))
//#define VERBOSE_DMA ((1 << DMA_PLOTTER) | (1 << DMA_SCSI) | (1 << DMA_FLOPPY))

#define VERBOSE     0
#define VERBOSE_IRQ 0
#define VERBOSE_DMA 0
#include "logmacro.h"

#define LOGIRQ(irq, ...) do { if (VERBOSE_IRQ & (1U << irq)) (LOG_OUTPUT_FUNC)(__VA_ARGS__); } while (false)
#define LOGDMA(dma, ...) do { if (VERBOSE_DMA & (1U << dma)) (LOG_OUTPUT_FUNC)(__VA_ARGS__); } while (false)

void interpro_ioga_device::map(address_map &map)
{
	map(0x0c, 0x1b).rw(FUNC(interpro_ioga_device::dma_plotter_r), FUNC(interpro_ioga_device::dma_plotter_w));
	map(0x1c, 0x1f).rw(FUNC(interpro_ioga_device::dma_plotter_eosl_r), FUNC(interpro_ioga_device::dma_plotter_eosl_w));
	map(0x20, 0x2f).rw(FUNC(interpro_ioga_device::dma_scsi_r), FUNC(interpro_ioga_device::dma_scsi_w));
	map(0x30, 0x3f).rw(FUNC(interpro_ioga_device::dma_floppy_r), FUNC(interpro_ioga_device::dma_floppy_w));
	map(0x40, 0x43).rw(FUNC(interpro_ioga_device::serial_dma0_addr_r), FUNC(interpro_ioga_device::serial_dma0_addr_w));
	map(0x44, 0x47).rw(FUNC(interpro_ioga_device::serial_dma0_ctrl_r), FUNC(interpro_ioga_device::serial_dma0_ctrl_w));
	map(0x48, 0x4b).rw(FUNC(interpro_ioga_device::serial_dma1_addr_r), FUNC(interpro_ioga_device::serial_dma1_addr_w));
	map(0x4c, 0x4f).rw(FUNC(interpro_ioga_device::serial_dma1_ctrl_r), FUNC(interpro_ioga_device::serial_dma1_ctrl_w));
	map(0x50, 0x53).rw(FUNC(interpro_ioga_device::serial_dma2_addr_r), FUNC(interpro_ioga_device::serial_dma2_addr_w));
	map(0x54, 0x57).rw(FUNC(interpro_ioga_device::serial_dma2_ctrl_r), FUNC(interpro_ioga_device::serial_dma2_ctrl_w));

	map(0x84, 0x87).rw(FUNC(interpro_ioga_device::mouse_status_r), FUNC(interpro_ioga_device::mouse_status_w));
	map(0x88, 0x8b).rw(FUNC(interpro_ioga_device::prescaler_r), FUNC(interpro_ioga_device::prescaler_w));
	map(0x8c, 0x8f).rw(FUNC(interpro_ioga_device::timer0_r), FUNC(interpro_ioga_device::timer0_w));
	map(0x90, 0x93).rw(FUNC(interpro_ioga_device::timer1_r), FUNC(interpro_ioga_device::timer1_w));
	map(0x94, 0x97).r(FUNC(interpro_ioga_device::error_address_r));
	map(0x98, 0x9b).r(FUNC(interpro_ioga_device::error_businfo_r));
	map(0x9c, 0x9d).rw(FUNC(interpro_ioga_device::arbctl_r), FUNC(interpro_ioga_device::arbctl_w));
}

void emerald_ioga_device::map(address_map &map)
{
	interpro_ioga_device::map(map);

	map(0x00, 0x03).rw(FUNC(emerald_ioga_device::eth_base_r), FUNC(emerald_ioga_device::eth_base_w));
	map(0x04, 0x05).rw(FUNC(emerald_ioga_device::eth_control_r), FUNC(emerald_ioga_device::eth_control_w));

	map(0x60, 0x83).rw(FUNC(interpro_ioga_device::hardint_r), FUNC(interpro_ioga_device::hardint_w));
	map(0x82, 0x82).rw(FUNC(interpro_ioga_device::softint_r), FUNC(interpro_ioga_device::softint_w));
	map(0x83, 0x83).rw(FUNC(interpro_ioga_device::nmictrl_r), FUNC(interpro_ioga_device::nmictrl_w));
}

void turquoise_ioga_device::map(address_map &map)
{
	interpro_ioga_device::map(map);

	map(0x00, 0x03).rw(FUNC(turquoise_ioga_device::eth_base_r), FUNC(turquoise_ioga_device::eth_base_w));
	map(0x04, 0x05).rw(FUNC(turquoise_ioga_device::eth_control_r), FUNC(turquoise_ioga_device::eth_control_w));

	map(0x60, 0x83).rw(FUNC(turquoise_ioga_device::hardint_r), FUNC(turquoise_ioga_device::hardint_w));
	map(0x82, 0x82).rw(FUNC(turquoise_ioga_device::softint_r), FUNC(turquoise_ioga_device::softint_w));
	map(0x83, 0x83).rw(FUNC(turquoise_ioga_device::nmictrl_r), FUNC(turquoise_ioga_device::nmictrl_w));

	//map(0x9e, 0x9f).rw(FUNC(turquoise_ioga_device::?), FUNC(turquoise_ioga_device::?)); // ip2000 boot code writes 0x7f18
}

void sapphire_ioga_device::map(address_map &map)
{
	interpro_ioga_device::map(map);

	map(0x00, 0x03).rw(FUNC(sapphire_ioga_device::eth_remap_r), FUNC(sapphire_ioga_device::eth_remap_w));
	map(0x04, 0x07).rw(FUNC(sapphire_ioga_device::eth_mappg_r), FUNC(sapphire_ioga_device::eth_mappg_w));
	map(0x08, 0x0b).rw(FUNC(sapphire_ioga_device::eth_control_r), FUNC(sapphire_ioga_device::eth_control_w));

	// 5a - sib control?
	//map(0x58, 0x5b).rw(FUNC(interpro_ioga_device::sib_r), FUNC(interpro_ioga_device::sib_w));

	map(0x5c, 0x83).rw(FUNC(interpro_ioga_device::hardint_r), FUNC(interpro_ioga_device::hardint_w));
	map(0x82, 0x82).rw(FUNC(interpro_ioga_device::softint_r), FUNC(interpro_ioga_device::softint_w));
	map(0x83, 0x83).rw(FUNC(interpro_ioga_device::nmictrl_r), FUNC(interpro_ioga_device::nmictrl_w));

	map(0xa0, 0xa3).rw(FUNC(sapphire_ioga_device::timer2_count_r), FUNC(sapphire_ioga_device::timer2_count_w));
	map(0xa4, 0xa7).rw(FUNC(sapphire_ioga_device::timer2_value_r), FUNC(sapphire_ioga_device::timer2_value_w));
	map(0xa8, 0xab).rw(FUNC(sapphire_ioga_device::timer3_r), FUNC(sapphire_ioga_device::timer3_w));
	map(0xac, 0xaf).rw(FUNC(sapphire_ioga_device::bus_timeout_r), FUNC(sapphire_ioga_device::bus_timeout_w)); // boot writes 0x64

	map(0xb0, 0xbf).rw(FUNC(sapphire_ioga_device::softint_vector_r), FUNC(sapphire_ioga_device::softint_vector_w));

	//c0, c4, c8 -ethernet address a,b,c?
}

DEFINE_DEVICE_TYPE(EMERALD_IOGA, emerald_ioga_device, "ioga_e", "I/O Gate Array (Emerald)")
DEFINE_DEVICE_TYPE(TURQUOISE_IOGA, turquoise_ioga_device, "ioga_t", "I/O Gate Array (Turquoise)")
DEFINE_DEVICE_TYPE(SAPPHIRE_IOGA, sapphire_ioga_device, "ioga_s", "I/O Gate Array (Sapphire)")

interpro_ioga_device::interpro_ioga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_memory_space(*this, finder_base::DUMMY_TAG, -1, 32)
	, m_memory(nullptr)
	, m_out_nmi_func(*this)
	, m_out_irq_func(*this)
	, m_out_irq_vector_func(*this)
	, m_fdc_tc_func(*this)
	, m_eth_ca_func(*this)
	, m_dma_channel{
		{ 0,0,0,0,CLEAR_LINE, {*this}, {*this}, ARBCTL_BGR_PLOT, DMA_PLOTTER, "plotter" },
		{ 0,0,0,0,CLEAR_LINE, {*this}, {*this}, ARBCTL_BGR_SCSI, DMA_SCSI, "scsi" },
		{ 0,0,0,0,CLEAR_LINE, {*this}, {*this}, ARBCTL_BGR_FDC, DMA_FLOPPY, "floppy" } }
	, m_serial_dma_channel{
		{ 0,0,CLEAR_LINE, {*this}, {*this}, ARBCTL_BGR_SER0, 0, "serial0" },
		{ 0,0,CLEAR_LINE, {*this}, {*this}, ARBCTL_BGR_SER1, 1, "serial1" },
		{ 0,0,CLEAR_LINE, {*this}, {*this}, ARBCTL_BGR_SER2, 2, "serial2" } }
{
}

emerald_ioga_device::emerald_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_ioga_device(mconfig, EMERALD_IOGA, tag, owner, clock)
{
}

turquoise_ioga_device::turquoise_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_ioga_device(mconfig, TURQUOISE_IOGA, tag, owner, clock)
{
}

sapphire_ioga_device::sapphire_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_ioga_device(mconfig, SAPPHIRE_IOGA, tag, owner, clock)
{
}

void interpro_ioga_device::device_start()
{
	m_memory = m_memory_space->cache<2, 0, ENDIANNESS_LITTLE>();

	// resolve callbacks
	m_out_nmi_func.resolve();
	m_out_irq_func.resolve();
	m_out_irq_vector_func.resolve();
	m_fdc_tc_func.resolve();
	m_eth_ca_func.resolve();

	for (dma_channel_t &dma_channel : m_dma_channel)
	{
		dma_channel.device_r.resolve();
		dma_channel.device_w.resolve();
	}

	for (serial_dma_channel_t &dma_channel : m_serial_dma_channel)
	{
		dma_channel.device_r.resolve();
		dma_channel.device_w.resolve();
	}

	// allocate timers
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::interrupt_check), this));
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::dma), this));
	m_serial_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::serial_dma), this));

	m_timer0 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::timer0), this));
	m_timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::timer1), this));

	m_timer_60hz = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::timer_60hz), this));

	m_eth_reset_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(interpro_ioga_device::eth_reset), this));
}

void sapphire_ioga_device::device_start()
{
	interpro_ioga_device::device_start();

	m_timer2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sapphire_ioga_device::timer2), this));
	m_timer3 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sapphire_ioga_device::timer3), this));
}

void interpro_ioga_device::device_reset()
{
	// initialise interrupt state
	m_active_interrupt_type = INT_NONE;
	m_nmi_state = CLEAR_LINE;
	m_irq_state = CLEAR_LINE;
	m_irq_vector = 0;
	m_line_state = 0;

	m_hwicr = std::make_unique<u16[]>(get_int_count());

	// initialise dma state
	for (dma_channel_t &dma_channel : m_dma_channel)
		dma_channel.drq_state = CLEAR_LINE;

	for (serial_dma_channel_t &dma_channel : m_serial_dma_channel)
		dma_channel.drq_state = CLEAR_LINE;

	// disable timers
	m_interrupt_timer->enable(false);
	m_dma_timer->enable(false);
	m_serial_dma_timer->enable(false);

	m_timer0->enable(false);
	m_timer1->enable(false);

	m_eth_reset_timer->enable(false);

	m_timer0_count = 0;
	m_timer1_count = 0;

	m_timer_60hz->adjust(attotime::zero, 0, attotime::from_hz(60));

	m_prescaler = 0;
}

void sapphire_ioga_device::device_reset()
{
	interpro_ioga_device::device_reset();

	m_timer2->enable(false);
	m_timer3->enable(false);

	m_timer2_count = 0;
	m_timer2_value = 0;
	m_timer3_count = 0;
}

/*
 * Interrupts
 */
WRITE32_MEMBER(interpro_ioga_device::bus_error)
{
	LOG("bus_error address 0x%08x businfo 0x%08x\n", data, offset);

	m_error_address = data;
	m_error_businfo = offset;

	set_nmi_line(ASSERT_LINE);
}

void interpro_ioga_device::set_nmi_line(int state)
{
	LOGMASKED(LOG_NMI, "nmi: %s (%s)\n", state ? "asserted" : "cleared", machine().describe_context());

	if (state == ASSERT_LINE)
	{
		// check if nmi is enabled
		if (((m_nmictrl & NMI_IE) == NMI_IE) || ((m_nmictrl & (NMI_ALL | NMI_ENABLE1)) == (NMI_ALL | NMI_ENABLE1)))
		{
			LOGMASKED(LOG_NMI, "nmi: asserting output nmi line\n");

			// if level triggered, disable input from pin
			if ((m_nmictrl & NMI_EDGE) == 0)
				m_nmictrl &= ~NMI_ENABLE2;

			nmi(ASSERT_LINE);
		}
	}
	else
		nmi(CLEAR_LINE);
}

void interpro_ioga_device::nmi(int state)
{
	if (m_nmi_state != state)
	{
		m_nmi_state = state;
		m_out_nmi_func(state);
	}
}

void interpro_ioga_device::set_int_line(int number, int state)
{
	const u8 offset = get_reg_offset(number);

	LOGIRQ(number, "irq: hard interrupt %d state %d\n", number, state);

	if (state == CLEAR_LINE)
	{
		m_line_state &= ~(1 << number);
		return;
	}

	// check already pending
	if (m_hwicr[offset] & IRQ_PENDING)
		return;

	// record line state for level triggered interrupts
	if (!(m_hwicr[offset] & IRQ_EDGE))
		m_line_state |= (1 << number);

	// check enabled
	if ((number != IRQ_SERDMA && !(m_hwicr[offset] & (IRQ_ENABLE | IRQ_ENABLE_INT)))
	|| ((number == IRQ_SERDMA && !(m_hwicr[offset] & IRQ_ENABLE_SERDMA))))
		return;

	// set pending
	if (BIT(m_line_state, number) || (m_hwicr[offset] & IRQ_EDGE))
	{
		LOGIRQ(number, "irq: hard interrupt %d pending\n", number);
		m_hwicr[offset] |= IRQ_PENDING;

		// schedule interrupt check
		m_interrupt_timer->adjust(attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(interpro_ioga_device::interrupt_check)
{
	// find highest priority pending interrupt
	u16 irq_vector = get_irq_vector();

	// hard interrupts
	for (u8 i = 0; i < get_int_count(); i++)
	{
		// test interrupt pending
		if (m_hwicr[i] & IRQ_PENDING)
		{
			// check priority
			if (m_active_interrupt_type == INT_NONE || (m_hwicr[i] & IRQ_VECTOR) < irq_vector)
			{
				m_active_interrupt_type = INT_HARD;
				m_active_interrupt_number = i;

				irq_vector = m_hwicr[i] & IRQ_VECTOR;
			}
		}
	}

	// soft interrupts
	if (m_softint)
	{
		for (u8 i = 0; i < 8; i++)
		{
			// test interrupt pending
			if (m_softint & (1 << i))
			{
				// check priority
				if ((m_active_interrupt_type == INT_NONE) || (0x8f + i * 0x10) < irq_vector)
				{
					m_active_interrupt_type = INT_SOFT;
					m_active_interrupt_number = i;

					irq_vector = 0x8f + i * 0x10;
				}
			}
		}
	}

	// assert irq and ivec
	if (m_active_interrupt_type != INT_NONE)
		irq(ASSERT_LINE, irq_vector);
}

TIMER_CALLBACK_MEMBER(sapphire_ioga_device::interrupt_check)
{
	// find highest priority pending interrupt
	u16 irq_vector = get_irq_vector();

	// hard interrupts
	for (u8 i = 0; i < get_int_count(); i++)
	{
		// test interrupt pending
		if (m_hwicr[i] & IRQ_PENDING)
		{
			// check priority
			if (m_active_interrupt_type == INT_NONE || (m_hwicr[i] & IRQ_VECTOR) < irq_vector)
			{
				m_active_interrupt_type = INT_HARD;
				m_active_interrupt_number = i;

				irq_vector = m_hwicr[i] & IRQ_VECTOR;
			}
		}
	}

	// soft interrupts (low type)
	if (m_softint)
	{
		for (u8 i = 0; i < 8; i++)
		{
			// test interrupt pending
			if (m_softint & (1 << i))
			{
				// check priority
				if (m_active_interrupt_type == INT_NONE || (0x8f + i * 0x10) < irq_vector)
				{
					m_active_interrupt_type = INT_SOFT;
					m_active_interrupt_number = i;

					irq_vector = 0x8f + i * 0x10;
				}
			}
		}
	}

	// soft interrupts (high type)
	for (u8 i = 0; i < 8; i++)
	{
		// test interrupt pending
		if (m_swicr[i] & IRQ_PENDING)
		{
			// check priority
			if (m_active_interrupt_type == INT_NONE || (m_swicr[i] & IRQ_VECTOR) < irq_vector)
			{
				m_active_interrupt_type = INT_SOFT;
				m_active_interrupt_number = i + 8;

				irq_vector = m_swicr[i] & IRQ_VECTOR;
			}
		}
	}

	// assert irq and ivec
	if (m_active_interrupt_type != INT_NONE)
		irq(ASSERT_LINE, irq_vector);
}

void interpro_ioga_device::irq(int state, u8 irq_vector)
{
	if (irq_vector != m_irq_vector)
	{
		LOGIRQ(m_active_interrupt_number, "irq: setting irq vector 0x%02x\n", irq_vector);

		m_irq_vector = irq_vector;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(interpro_ioga_device::set_irq_vector), this));
	}

	if (m_irq_state != state)
	{
		LOGIRQ(m_active_interrupt_number, "irq: %s interrupt type %d number %d\n",
			state ? "asserting" : "clearing", m_active_interrupt_type, m_active_interrupt_number);

		m_irq_state = state;
		m_out_irq_func(state);

		if (state == CLEAR_LINE)
			m_active_interrupt_type = INT_NONE;
	}
}

IRQ_CALLBACK_MEMBER(interpro_ioga_device::acknowledge_interrupt)
{
	switch (irqline)
	{
	case INPUT_LINE_IRQ0:
		LOGIRQ(m_active_interrupt_number, "irq: interrupt type %d number %d acknowledged\n",
			m_active_interrupt_type, m_active_interrupt_number);

		// clear pending
		switch (m_active_interrupt_type)
		{
		case INT_HARD:
			m_hwicr[m_active_interrupt_number] &= ~IRQ_PENDING;
			break;

		case INT_SOFT:
			m_softint &= ~(1 << m_active_interrupt_number);
			break;

		default:
			// can't happen
			fatalerror("irq: interrupt acknowledged without active interrupt\n");
			break;
		}

		// clear irq and irq vector
		irq(CLEAR_LINE, 0);
		break;

	case INPUT_LINE_NMI:
		nmi(CLEAR_LINE);
		break;
	}

	// schedule interrupt check
	m_interrupt_timer->adjust(attotime::zero);

	return 0;
}

IRQ_CALLBACK_MEMBER(sapphire_ioga_device::acknowledge_interrupt)
{
	switch (irqline)
	{
	case INPUT_LINE_IRQ0:
		LOGIRQ(m_active_interrupt_number, "irq: interrupt type %d number %d acknowledged\n",
			m_active_interrupt_type, m_active_interrupt_number);

		// clear pending
		switch (m_active_interrupt_type)
		{
		case INT_HARD:
			m_hwicr[m_active_interrupt_number] &= ~IRQ_PENDING;
			break;

		case INT_SOFT:
			if (m_active_interrupt_number < 8)
				m_softint &= ~(1 << m_active_interrupt_number);
			else
				m_swicr[m_active_interrupt_number - 8] &= ~IRQ_PENDING;
			break;

		default:
			// can't happen
			fatalerror("irq: interrupt acknowledged without active interrupt\n");
			break;
		}

		// clear irq and irq vector
		irq(CLEAR_LINE, 0);
		break;

	case INPUT_LINE_NMI:
		nmi(CLEAR_LINE);
		break;
	}

	// schedule interrupt check
	m_interrupt_timer->adjust(attotime::zero);

	return 0;
}

WRITE16_MEMBER(interpro_ioga_device::hardint_w)
{
	const int number = get_int_number(offset);
	bool force = false;

	LOGIRQ(offset, "irq: interrupt %d offset 0x%02x data 0x%04x mem_mask 0x%04x (%s)\n", number, offset, data, mem_mask, machine().describe_context());

	if (!(m_hwicr[offset] & IRQ_PENDING))
	{
		// handle forcing
		if (data & IRQ_PENDING)
		{
			LOGIRQ(number, "irq: interrupt %d force pending\n", number);

			// set force pending
			m_force_state |= (1 << number);
		}
		else if (BIT(m_force_state, number))
		{
			LOGIRQ(number, "irq: interrupt %d forced\n", number);

			// clear force pending
			m_force_state &= ~(1 << number);

			// force the interrupt
			force = true;
		}
	}

	// store the data
	mem_mask &= ~IRQ_PENDING;
	COMBINE_DATA(&m_hwicr[offset]);

	if (force)
	{
		// force an interrupt
		set_int_line(number, ASSERT_LINE);
		set_int_line(number, CLEAR_LINE);
	}
	else if (m_line_state & (1 << number))
		// re-assert line
		set_int_line(number, ASSERT_LINE);
}

WRITE8_MEMBER(interpro_ioga_device::softint_w)
{
	// store the written value
	m_softint |= data;

	// schedule interrupt check
	m_interrupt_timer->adjust(attotime::zero);
}

WRITE8_MEMBER(interpro_ioga_device::nmictrl_w)
{
	// check for a forced nmi (NMI_NEGPOL written from 1 to 0 with NMI_IE set)
	const bool forced = (m_nmictrl & NMI_NEGPOL) && ((data & (NMI_NEGPOL | NMI_IE)) == NMI_IE);

	LOGMASKED(LOG_NMI, "nmi: nmictrl 0x%02x (%s)\n", data, machine().describe_context());
	m_nmictrl = data;

	if (forced)
	{
		LOGMASKED(LOG_NMI, "nmi: forced\n");

		set_nmi_line(ASSERT_LINE);
	}
}

WRITE16_MEMBER(sapphire_ioga_device::softint_vector_w)
{
	const int number = offset + 8;

	LOG("irq: soft interrupt %d data 0x%04x\n", number, data);

	if (!(m_swicr[offset] & IRQ_PENDING))
	{
		// handle forcing
		if (data & IRQ_PENDING)
		{
			LOG("irq: soft interrupt %d force pending\n", number);

			// set force pending
			m_force_state |= (1 << (offset + 24));

			mem_mask &= ~IRQ_PENDING;
		}
		else if (BIT(m_force_state, offset + 24))
		{
			LOG("irq: soft interrupt %d forced\n", number);

			// clear force pending
			m_force_state &= ~(1 << (offset + 24));

			// set interrupt pending
			data |= IRQ_PENDING;
		}
	}

	// update the register
	COMBINE_DATA(&m_swicr[offset]);

	// schedule interrupt check
	m_interrupt_timer->adjust(attotime::zero);
}

/*
 * DMA
 */
TIMER_CALLBACK_MEMBER(interpro_ioga_device::dma)
{
	for (dma_channel_t &dma_channel : m_dma_channel)
	{
		// check if the channel is enabled
		if (!(dma_channel.control & DMA_CTRL_ENABLE))
			continue;

		// check if the device is requesting a transfer
		if (dma_channel.drq_state == CLEAR_LINE)
			continue;

		// check if there's something to transfer
		if (dma_channel.transfer_count == 0)
			continue;

		// check if the bus is available
		if (!(m_arbctl & dma_channel.arb_mask))
			continue;

		if (dma_channel.control & DMA_CTRL_BERR)
			continue;

		// transfer from the memory to device or device to memory
		while (dma_channel.transfer_count && dma_channel.drq_state)
		{
			// transfer from the memory to device or device to memory
			if (dma_channel.control & DMA_CTRL_WRITE)
				dma_channel.device_w(m_memory->read_byte(dma_channel.real_address));
			else
				m_memory->write_byte(dma_channel.real_address, dma_channel.device_r());

			// increment address and decrement count
			dma_channel.real_address++;
			dma_channel.transfer_count--;

			// check for page wrap
			if ((dma_channel.real_address & 0xfff) == 0 && dma_channel.transfer_count)
			{
				LOGDMA(dma_channel.channel, "dma: wrapped to next memory page\n");

				// translate virtual address
				if (dma_channel.control & DMA_CTRL_VIRTUAL)
				{
					const u32 ptde = m_memory->read_dword(dma_channel.virtual_address);

					// FIXME: ignore the page fault flag?
					dma_channel.real_address = ptde & ~0xfff;

					LOGDMA(dma_channel.channel, "dma: translated virtual 0x%08x real 0x%08x\n",
						dma_channel.virtual_address, dma_channel.real_address);

					dma_channel.virtual_address += 4;
				}
			}
		}

		// check if the transfer is complete
		if (dma_channel.transfer_count == 0)
		{
			LOGDMA(dma_channel.channel, "dma: transfer %s device ended channel %d control 0x%08x real address 0x%08x virtual address 0x%08x count 0x%08x\n",
				(dma_channel.control & DMA_CTRL_WRITE) ? "to" : "from", dma_channel.channel, dma_channel.control, dma_channel.real_address, dma_channel.virtual_address, dma_channel.transfer_count);

			if (dma_channel.channel == DMA_FLOPPY)
			{
				LOGDMA(dma_channel.channel, "dma: asserting fdc terminal count line\n");

				m_fdc_tc_func(ASSERT_LINE);
				m_fdc_tc_func(CLEAR_LINE);
			}

			// set transfer count zero flag
			dma_channel.control |= DMA_CTRL_TCZERO;

			// disable the channel
			dma_channel.control &= ~DMA_CTRL_ENABLE;
		}

#if IOGA_DMA_DIAG_HACK
#define TAG ((dma_channel.control & DMA_CTRL_TAG) >> 3)

				// hacks for forced dma bus error diagnostic tests
				if ((dma_channel.control & 0xfe000000 && dma_channel.control & 0xe00) || ((dma_channel.control & DMA_CTRL_WMASK) == 0x41000000))
				if (dma_channel.real_address & 0xff000000 || dma_channel.real_address == 0)
				{
					LOGDMA(dma_channel.channel, "dma: forced bus error hack, control 0x%08x\n", dma_channel.control);

					// (7.0267) trigger an interrupt
					m_hwicr[dma_channel.channel + 1] |= IRQ_PENDING;

					// (7.0268) set bus error bit
					dma_channel.control |= DMA_CTRL_BERR;

					// 7.0269, 7.0276, 7.0281, 7.0289: set error address from virtual or real dma address
					// HACK: don't set error address for 7.0276 special case
					if (!(dma_channel.control == 0x65400600 && dma_channel.real_address != 0))
						m_error_address = dma_channel.control & DMA_CTRL_VIRTUAL ? dma_channel.virtual_address : dma_channel.real_address;

					// compute bus error cycle type from control register
					u8 cycle_type = 0x30;
					switch ((dma_channel.control >> 24) & 0x8c)
					{
					case 0x00: cycle_type |= 2; break;
					case 0x04: cycle_type |= 1; break;
					case 0x08: cycle_type |= 3; break;
					case 0x80: cycle_type |= 4; break;
					case 0x84: cycle_type |= 8; break;
					}

					switch (dma_channel.control & ~DMA_CTRL_BERR)
					{
					case 0x61000800: // VIRTUAL | WRITE | TAG(3)
						// (7.0266) trigger an nmi
						m_nmi_pending = true;

						// (7.0270) set error cycle type 0x52f0: SNAPOK | BERR | BG(IOD) | TAG(0c0) | CT(30)
						m_error_businfo = BINFO_SNAPOK | BINFO_BERR | BINFO_BG_IOD | 0xf0;
						break;

					case 0x65000600: // VIRTUAL | WRITE | X | TAG(4)
						if (dma_channel.real_address != 0)
						{
							// (7.0275) control register expect 0x64400800
							dma_channel.control &= ~0x600;
							dma_channel.control |= 0x800;

							// (7.0277) set error cycle type 0x5331: SNAPOK | BERR | BG(IOD) | TAG(100) | CT(31)
							m_error_businfo = BINFO_SNAPOK | BINFO_BERR | BINFO_BG_IOD | TAG | cycle_type;
						}
						else
						{
							// (7.0287) set error cycle type 0x62f0: SNAPOK | MMBE | BG(IOD) | TAG(0c0) | CT(30)
							m_error_businfo = BINFO_SNAPOK | BINFO_MMBE | BINFO_BG_IOD | TAG | 0x30;
						}
						break;

					default:
						m_error_businfo = BINFO_SNAPOK | BINFO_BERR | BINFO_BG_IOD | TAG | cycle_type;
						break;
					}

					dma_channel.state = COMPLETE;
				}
#endif
	}
}

void interpro_ioga_device::drq(int state, int channel)
{
	dma_channel_t &dma_channel = m_dma_channel[channel];

	dma_channel.drq_state = state;

	// log every 256 bytes
	if ((dma_channel.transfer_count & 0xff) == 0)
		LOGDMA(channel, "dma: drq for channel %d %s transfer_count 0x%08x\n",
			channel, state ? "asserted" : "deasserted", dma_channel.transfer_count);

	if (state)
		m_dma_timer->adjust(attotime::zero);
}

u32 interpro_ioga_device::dma_r(address_space &space, offs_t offset, u32 mem_mask, dma_channel channel) const
{
	const dma_channel_t &dma_channel = m_dma_channel[channel];

	switch (offset)
	{
	case 0:
		return dma_channel.real_address;

	case 1:
		return dma_channel.virtual_address;

	case 2:
		return dma_channel.transfer_count;

	case 3:
		return dma_channel.control & ~DMA_CTRL_VIRTUAL;

	default:
		logerror("dma_r: unknown dma register %d\n", offset);
		return 0;
	}
}

void interpro_ioga_device::dma_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, dma_channel channel)
{
	dma_channel_t &dma_channel = m_dma_channel[channel];

	switch (offset)
	{
	case 0:
		LOGDMA(channel, "dma: channel %d real address 0x%08x mem_mask 0x%08x (%s)\n",
			channel, data, mem_mask, machine().describe_context());
		COMBINE_DATA(&dma_channel.real_address);
		break;

	case 1:
		LOGDMA(channel, "dma: channel %d virtual address 0x%08x mem_mask 0x%08x (%s)\n",
			channel, data, mem_mask, machine().describe_context());
		COMBINE_DATA(&dma_channel.virtual_address);
		dma_channel.virtual_address &= ~0x3;
		break;

	case 2:
		LOGDMA(channel, "dma: channel %d transfer count 0x%08x mem_mask 0x%08x (%s)\n",
			channel, data, mem_mask, machine().describe_context());
		COMBINE_DATA(&dma_channel.transfer_count);

		dma_channel.control &= ~DMA_CTRL_TCZERO;
		break;

	case 3:
		LOGDMA(channel, "dma: channel %d control 0x%08x mem_mask 0x%08x (%s)\n",
			channel, data, mem_mask, machine().describe_context());

		dma_channel.control = (data & mem_mask & (DMA_CTRL_WMASK|DMA_CTRL_VIRTUAL)) | (dma_channel.control & (~mem_mask | ~(DMA_CTRL_WMASK|DMA_CTRL_VIRTUAL)));

		// translate virtual address
		if (data & DMA_CTRL_VIRTUAL)
		{
			const u32 ptde = m_memory->read_dword(dma_channel.virtual_address);

			// FIXME: ignore the page fault flag?
			dma_channel.real_address = (ptde & ~0xfff) | (dma_channel.real_address & 0xfff);

			LOGDMA(dma_channel.channel, "dma: translated virtual 0x%08x real 0x%08x\n",
				dma_channel.virtual_address, dma_channel.real_address);

			dma_channel.virtual_address += 4;
		}

		// (7.0272) if bus error flag is written, clear existing bus error (otherwise retain existing state)
		if (data & DMA_CTRL_BERR)
			dma_channel.control &= ~DMA_CTRL_BERR;

		break;
	}
}

TIMER_CALLBACK_MEMBER(interpro_ioga_device::serial_dma)
{
	LOGMASKED(LOG_SERIALDMA, "dma: serial_dma()\n");

	for (serial_dma_channel_t &dma_channel : m_serial_dma_channel)
	{
		// check if there's something to transfer
		if ((dma_channel.control & SDMA_COUNT) == 0)
			continue;

		// check if the device is requesting a transfer
		if (dma_channel.drq_state == CLEAR_LINE)
			continue;

		// check if the bus is available
		if ((m_arbctl & dma_channel.arb_mask) == 0)
			continue;

		// transfer from the memory to device or device to memory
		while ((dma_channel.control & SDMA_COUNT) && dma_channel.drq_state)
		{
			if (dma_channel.control & SDMA_WRITE)
			{
				u8 data = m_memory->read_byte(dma_channel.address++);

				LOGMASKED(LOG_SERIALDMA, "dma: writing byte 0x%02x to serial channel %d\n",
					data, dma_channel.channel);

				dma_channel.device_w(data);
			}
			else
			{
				u8 data = dma_channel.device_r();

				LOGMASKED(LOG_SERIALDMA, "dma: reading byte 0x%02x from serial channel %d\n",
					data, dma_channel.channel);

				m_memory->write_byte(dma_channel.address++, data);
			}

			// decrement transfer count
			dma_channel.control = (dma_channel.control & ~SDMA_COUNT) | ((dma_channel.control & SDMA_COUNT) - 1);
		}

		if ((dma_channel.control & SDMA_COUNT) == 0)
		{
			// transfer count zero
			dma_channel.control |= SDMA_TCZERO;
			dma_channel.control &= ~SDMA_ENABLE;

			// raise an interrupt
			// FIXME: assume edge-triggered?
			set_int_line(IRQ_SERDMA, ASSERT_LINE);
			set_int_line(IRQ_SERDMA, CLEAR_LINE);
		}
	}
}

void interpro_ioga_device::serial_drq(int state, int channel)
{
	serial_dma_channel_t &dma_channel = m_serial_dma_channel[channel];

	dma_channel.drq_state = state;

	LOGMASKED(LOG_SERIALDMA, "dma: drq for serial channel %d %s count 0x%04x\n",
		channel, state ? "asserted" : "deasserted", dma_channel.control & SDMA_COUNT);

	if (state && (dma_channel.control & SDMA_ENABLE))
		m_serial_dma_timer->adjust(attotime::zero);
}

void interpro_ioga_device::serial_dma_addr_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, int channel)
{
	LOGMASKED(LOG_SERIALDMA, "dma: serial channel %d address 0x%08x mask 0x%08x (%s)\n",
		channel, data, mem_mask, machine().describe_context());

	COMBINE_DATA(&m_serial_dma_channel[channel].address);
}

void interpro_ioga_device::serial_dma_ctrl_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, int channel)
{
	serial_dma_channel_t &dma_channel = m_serial_dma_channel[channel];

	LOGMASKED(LOG_SERIALDMA, "dma: serial channel %d control 0x%08x mask 0x%08x (%s)\n",
		channel, data, mem_mask, machine().describe_context());

	COMBINE_DATA(&dma_channel.control);

	if (dma_channel.control & SDMA_ENABLE)
		m_serial_dma_timer->adjust(attotime::zero);
}

/*
 * Bus arbitration and control
 */
WRITE16_MEMBER(interpro_ioga_device::arbctl_w)
{
	LOGMASKED(LOG_DMA | LOG_SERIALDMA, "dma: arbctl = 0x%04x (%s)\n",
		data, machine().describe_context());

	m_arbctl = data;

	// trigger serial dma waiting for bus access
	if (m_arbctl & (ARBCTL_BGR_SER0 | ARBCTL_BGR_SER1 | ARBCTL_BGR_SER2))
		m_serial_dma_timer->adjust(attotime::zero);
}

READ32_MEMBER(interpro_ioga_device::error_businfo_r)
{
	const u32 result = m_error_businfo;

	// clear register after reading
	if (!machine().side_effects_disabled())
		m_error_businfo = 0;

	return result;
}

/*
 * Timers
 */
READ32_MEMBER(interpro_ioga_device::timer0_r)
{
	LOGMASKED(LOG_TIMERRD, "timer0_r data 0x%08x mask 0x%08x (%s)\n",
		m_timer0_count, mem_mask, machine().describe_context());

	return m_timer0_count;
}

WRITE32_MEMBER(interpro_ioga_device::timer0_w)
{
	LOGMASKED(LOG_TIMER0, "timer0_w data 0x%08x mask 0x%08x prescaler 0x%08x (%s)\n",
		data, mem_mask, m_prescaler, machine().describe_context());

	// store the timer count value
	m_timer0_count = data;

	// restart the timer
	attotime period = attotime::from_ticks(m_prescaler ? m_prescaler & 0xffff : 0x10000, 10_MHz_XTAL);

	m_timer0->adjust(period, 0, period);
}

TIMER_CALLBACK_MEMBER(interpro_ioga_device::timer0)
{
	m_timer0_count++;

	// check if the timer has expired
	if (m_timer0_count == 0)
	{
		// stop the timer
		m_timer0->enable(false);

		// raise an interrupt
		set_int_line(IRQ_TIMER0, ASSERT_LINE);
		set_int_line(IRQ_TIMER0, CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(interpro_ioga_device::timer_60hz)
{
	set_int_line(IRQ_60HZ, ASSERT_LINE);
	set_int_line(IRQ_60HZ, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(sapphire_ioga_device::timer_60hz)
{
	set_int_line(IRQ_TIMER0, ASSERT_LINE);
	set_int_line(IRQ_TIMER0, CLEAR_LINE);
}

READ32_MEMBER(interpro_ioga_device::timer1_r)
{
	u32 result = m_timer1_count & TIMER1_COUNT;

	// set the start bit if the timer is currently enabled
	if (m_timer1->enabled())
		result |= TIMER1_START;
	else if (m_timer1->param())
		result |= TIMER1_EXPIRED;

	LOGMASKED(LOG_TIMERRD, "timer1_r data 0x%08x mask 0x%08x (%s)\n",
		result, mem_mask, machine().describe_context());

	return result;
}

WRITE32_MEMBER(interpro_ioga_device::timer1_w)
{
	// disable the timer
	m_timer1->enable(false);

	// store the timer count value
	m_timer1_count = data & TIMER1_COUNT;

	// start the timer if necessary
	if (data & TIMER1_START)
	{
		LOGMASKED(LOG_TIMER1, "timer1_w data 0x%08x mask 0x%08x prescaler 0x%08x (%s)\n",
			data, mem_mask, m_prescaler, machine().describe_context());

		attotime period = attotime::from_ticks(m_prescaler ? m_prescaler & 0xffff : 0x10000, 10_MHz_XTAL);

		m_timer1->adjust(period, 0, period);
	}
}

TIMER_CALLBACK_MEMBER(interpro_ioga_device::timer1)
{
	// decrement timer count value
	m_timer1_count--;

	// check if timer has expired
	if (m_timer1_count == 0)
	{
		LOGMASKED(LOG_TIMER1, "timer1 expired\n");

		// disable timer and set the zero flag
		m_timer1->enable(false);
		m_timer1->set_param(1);

		// raise an interrupt
		set_int_line(IRQ_TIMER1, ASSERT_LINE);
		set_int_line(IRQ_TIMER1, CLEAR_LINE);
	}
}

READ32_MEMBER(sapphire_ioga_device::timer2_count_r)
{
	LOGMASKED(LOG_TIMERRD, "timer2_count_r data 0x%08x mask 0x%08x (%s)\n",
		m_timer2_count, mem_mask, machine().describe_context());

	return m_timer2_count;
}

WRITE32_MEMBER(sapphire_ioga_device::timer2_count_w)
{
	m_timer2_count = data;

	LOGMASKED(LOG_TIMER2, "timer2_count_w data 0x%08x mask 0x%08x (%s)\n",
		data, mem_mask, machine().describe_context());
}

READ32_MEMBER(sapphire_ioga_device::timer2_value_r)
{
	LOGMASKED(LOG_TIMERRD, "timer2_value_r data 0x%08x mask 0x%08x (%s)\n",
		m_timer2_value, mem_mask, machine().describe_context());

	return m_timer2_value;
}

WRITE32_MEMBER(sapphire_ioga_device::timer2_value_w)
{
	m_timer2_value = data;
	m_timer2_count = data; // ?

	LOGMASKED(LOG_TIMER2, "timer2_value_w data 0x%08x mask 0x%08x (%s)\n",
		data, mem_mask, machine().describe_context());
}

READ32_MEMBER(sapphire_ioga_device::timer3_r)
{
	u32 result = m_timer3_count & TIMER3_COUNT;

	if (m_timer3->enabled())
		result |= TIMER3_START;
	else if (m_timer3->param())
		result |= TIMER3_EXPIRED;

	LOGMASKED(LOG_TIMERRD, "timer3_r data 0x%08x mask 0x%08x (%s)\n",
		result, mem_mask, machine().describe_context());

	return result;
}

WRITE32_MEMBER(sapphire_ioga_device::timer3_w)
{
	// stop the timer so it won't trigger while we're fiddling with it
	m_timer3->enable(false);

	// write the new value to the timer register
	m_timer3_count = data & TIMER3_COUNT;

	// start the timer if necessary
	if (data & TIMER3_START)
	{
		LOGMASKED(LOG_TIMER3, "timer3_w data 0x%08x mask 0x%08x (%s)\n",
			data, mem_mask, machine().describe_context());

		// theory: timer 3 is 12.5MHz (typical value of 12500 giving a delay of 1ms)
		m_timer3->adjust(attotime::zero, false, attotime::from_hz(XTAL(12'500'000)));
	}
}

TIMER_CALLBACK_MEMBER(sapphire_ioga_device::timer3)
{
	// decrement timer count value
	m_timer3_count--;

	// check for expiry
	if (m_timer3_count == 0)
	{
		LOGMASKED(LOG_TIMER3, "timer3 expired\n");

		// disable timer and set the zero flag
		m_timer3->enable(false);
		m_timer3->set_param(true);

		// raise an interrupt
		set_int_line(IRQ_TIMER3, ASSERT_LINE);
	}
}

READ32_MEMBER(interpro_ioga_device::prescaler_r)
{
	return (m_prescaler ^ 0xffff0000) - 0x10000;
}

WRITE32_MEMBER(interpro_ioga_device::prescaler_w)
{
	LOGMASKED(LOG_TIMER0 | LOG_TIMER1, "prescaler_w data 0x%08x mask 0x%08x (%s)\n",
		data, mem_mask, machine().describe_context());

	COMBINE_DATA(&m_prescaler);
}

/*
 * Mouse
 */
READ32_MEMBER(interpro_ioga_device::mouse_status_r)
{
	const u32 result = m_mouse_status ^ MOUSE_BUTTONS;

	LOGMASKED(LOG_MOUSE, "mouse_status_r status 0x%08x mask 0x%08x (%s)\n",
		result, mem_mask, machine().describe_context());

	// clear xpos and ypos fields and interrupt
	if (!machine().side_effects_disabled())
	{
		if (mem_mask & MOUSE_XPOS)
			m_mouse_status &= ~(MOUSE_XPOS);
		if (mem_mask & MOUSE_YPOS)
			m_mouse_status &= ~(MOUSE_YPOS);

		set_int_line(IRQ_MOUSE, CLEAR_LINE);
	}

	return result;
}

WRITE32_MEMBER(interpro_ioga_device::mouse_status_w)
{
	LOGMASKED(LOG_MOUSE, "mouse_status_w status 0x%08x mask 0x%08x\n",
		data, mem_mask);

	COMBINE_DATA(&m_mouse_status);

	set_int_line(IRQ_MOUSE, ASSERT_LINE);
}

/*
 * Ethernet
 */
WRITE32_MEMBER(emerald_ioga_device::eth_base_w)
{
	LOGMASKED(LOG_NETWORK, "eth: base_w 0x%08x mem_mask 0x%08x (%s)\n",
		data, mem_mask, machine().describe_context());

	m_eth_base = ((m_eth_base & ~mem_mask) | (data & mem_mask)) & ETH_BASE_MASK;
}

WRITE16_MEMBER(emerald_ioga_device::eth_control_w)
{
	LOGMASKED(LOG_NETWORK, "eth: control_w 0x%04x mem_mask 0x%04x (%s)\n",
		data, mem_mask, machine().describe_context());

	m_eth_control = data;

	// ethernet device reset (active low)
	if ((data & ETH_RESET) == 0)
	{
		// reset the ethernet device
		siblingdevice("eth")->reset();

		// clear the reset flag (the 250ns delay is long enough to pass diagnostic tests)
		m_eth_reset_timer->adjust(attotime::from_nsec(250));
	}

	// ethernet channel attention
	if (data & ETH_CA)
	{
		m_eth_ca_func(ASSERT_LINE);
		m_eth_ca_func(CLEAR_LINE);
	}
}

WRITE16_MEMBER(emerald_ioga_device::eth_w)
{
	const u32 address = m_eth_base | ((offset << 1) & ~ETH_BASE_MASK);

	LOGMASKED(LOG_NETWORK, "eth_w address 0x%08x mask 0x%04x data 0x%04x\n",
		address, mem_mask, data);
	m_memory->write_word(address, data, mem_mask);
}

READ16_MEMBER(emerald_ioga_device::eth_r)
{
	const u32 address = m_eth_base | ((offset << 1) & ~ETH_BASE_MASK);

	const u16 data = m_memory->read_word(address, mem_mask);
	LOGMASKED(LOG_NETWORK, "eth_r 0x%08x mask 0x%04x data 0x%04x\n",
		address, mem_mask, data);

	return data;
}

TIMER_CALLBACK_MEMBER(emerald_ioga_device::eth_reset)
{
	LOGMASKED(LOG_NETWORK, "eth: reset flag cleared\n");

	// clear ethernet reset flag
	m_eth_control &= ~ETH_RESET;
}

WRITE32_MEMBER(turquoise_ioga_device::eth_base_w)
{
	LOGMASKED(LOG_NETWORK, "eth: base_w 0x%08x mem_mask 0x%08x (%s)\n",
		data, mem_mask, machine().describe_context());

	m_eth_base = ((m_eth_base & ~mem_mask) | (data & mem_mask)) & ETH_BASE_MASK;
}

WRITE16_MEMBER(turquoise_ioga_device::eth_control_w)
{
	LOGMASKED(LOG_NETWORK, "eth: control_w 0x%04x mem_mask 0x%04x (%s)\n",
		data, mem_mask, machine().describe_context());

	m_eth_control = data;

	// ethernet device reset (active low)
	if ((data & ETH_RESET) == 0)
	{
		// reset the ethernet device
		siblingdevice("eth")->reset();

		// clear the reset flag (the 250ns delay is long enough to pass diagnostic tests)
		m_eth_reset_timer->adjust(attotime::from_nsec(250));
	}

	// ethernet channel attention
	if (data & ETH_CA)
	{
		m_eth_ca_func(ASSERT_LINE);
		m_eth_ca_func(CLEAR_LINE);
	}
}

WRITE16_MEMBER(turquoise_ioga_device::eth_w)
{
	const u32 address = m_eth_base | ((offset << 1) & ~ETH_BASE_MASK);

	LOGMASKED(LOG_NETWORK, "eth_w address 0x%08x mask 0x%04x data 0x%04x\n",
		address, mem_mask, data);
	m_memory->write_word(address, data, mem_mask);
}

READ16_MEMBER(turquoise_ioga_device::eth_r)
{
	const u32 address = m_eth_base | ((offset << 1) & ~ETH_BASE_MASK);

	const u16 data = m_memory->read_word(address, mem_mask);
	LOGMASKED(LOG_NETWORK, "eth_r 0x%08x mask 0x%04x data 0x%04x\n",
		address, mem_mask, data);

	return data;
}

TIMER_CALLBACK_MEMBER(turquoise_ioga_device::eth_reset)
{
	LOGMASKED(LOG_NETWORK, "eth: reset flag cleared\n");

	// clear ethernet reset flag
	m_eth_control &= ~ETH_RESET;
}

WRITE32_MEMBER(sapphire_ioga_device::eth_remap_w)
{
	LOGMASKED(LOG_NETWORK, "eth: remap = 0x%08x (%s)\n",
		data, machine().describe_context());

	m_eth_remap = data & ~0xf;
}

WRITE32_MEMBER(sapphire_ioga_device::eth_mappg_w)
{
	LOGMASKED(LOG_NETWORK, "eth: map page = 0x%08x (%s)\n",
		data, machine().describe_context());

	m_eth_mappg = data & ~0xf;
}

READ32_MEMBER(sapphire_ioga_device::eth_control_r)
{
	LOGMASKED(LOG_NETWORK, "eth: control_r 0x%08x (%s)\n",
		m_eth_control, machine().describe_context());

	return m_eth_control;
}

WRITE32_MEMBER(sapphire_ioga_device::eth_control_w)
{
	LOGMASKED(LOG_NETWORK, "eth: control_w 0x%08x mem_mask 0x%08x (%s)\n",
		data, mem_mask, machine().describe_context());

	m_eth_control = data & ETH_MASK;

	// ethernet device reset
	if ((data & ETH_RESET) == 0)
	{
		// reset the ethernet device
		siblingdevice("eth")->reset();

		// clear the reset flag (the 250ns delay is long enough to pass diagnostic tests)
		m_eth_reset_timer->adjust(attotime::from_nsec(250));
	}

	// ethernet channel attention
	if (data & ETH_CA)
	{
		m_eth_ca_func(ASSERT_LINE);
		m_eth_ca_func(CLEAR_LINE);
	}
}

WRITE16_MEMBER(sapphire_ioga_device::eth_w)
{
	// top two bits give channel (0=A, 4=B, 8=C, f=?)
	const int channel = offset >> 29;
	u32 address = (offset << 1) & 0x3fffffff;

	if ((m_eth_control & ETH_MAPEN) && (address & ETH_MAPPG) == (m_eth_mappg & ETH_MAPPG))
	{
		address &= ~(m_eth_mappg & ETH_MAPPG);
		address |= (m_eth_remap & ETH_REMAP_ADDR);

		LOGMASKED(LOG_NETWORK, "eth_w address 0x%08x remapped 0x%08x\n",
			offset << 1, address);
	}

	LOGMASKED(LOG_NETWORK, "eth_w channel %c address 0x%08x mask 0x%08x data 0x%04x\n",
		channel + 'A', address, mem_mask, data);
	m_memory->write_word(address, data, mem_mask);
}

READ16_MEMBER(sapphire_ioga_device::eth_r)
{
	// top two bits give channel (0=A, 4=B, 8=C, f=?)
	const int channel = offset >> 29;
	u32 address = (offset << 1) & 0x3fffffff;

	if ((m_eth_control & ETH_MAPEN) && (address & ETH_MAPPG) == (m_eth_mappg & ETH_MAPPG))
	{
		address &= ~(m_eth_mappg & ETH_MAPPG);
		address |= (m_eth_remap & ETH_REMAP_ADDR);
		address &= 0x3fffffff;

		LOGMASKED(LOG_NETWORK, "eth_r address 0x%08x remapped 0x%08x\n",
			offset << 1, address);
	}

	u16 data = m_memory->read_word(address, mem_mask);
	LOGMASKED(LOG_NETWORK, "eth_r channel %c address 0x%08x mask 0x%08x data 0x%04x\n",
		channel + 'A', address, mem_mask, data);
	return data;
}

TIMER_CALLBACK_MEMBER(sapphire_ioga_device::eth_reset)
{
	LOGMASKED(LOG_NETWORK, "eth: reset flag cleared\n");

	// clear ethernet reset flag
	m_eth_control &= ~ETH_RESET;
}
