// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the IOGA device found on Intergraph InterPro family workstations. There is no
 * public documentation on this device, so the implementation is being built to follow the logic of the
 * system boot ROM and its diagnostic tests.
 *
 * The device handles most of the I/O for the system, including timers, interrupts, DMA and target device
 * interfacing. There remains a significant amount of work to be completed before the boot diagnostics will
 * pass without errors, let alone successfully booting CLIX.
 *
 * Please be aware that code in here is not only broken, it's likely wrong in many cases.
 *
 * TODO
 *   - too long to list
 */
#include "emu.h"
#include "interpro_ioga.h"

#define VERBOSE 0
#if VERBOSE
#define LOG_TIMER_MASK 0xff
#define LOG_TIMER(timer, ...) if (LOG_TIMER_MASK & (1 << timer)) logerror(__VA_ARGS__)
#define LOG_INTERRUPT(...) logerror(__VA_ARGS__)
#define LOG_IOGA(...) logerror(__VA_ARGS__)
#define LOG_DMA(...) logerror(__VA_ARGS__)
#else
#define LOG_TIMER_MASK 0x00
#define LOG_TIMER(timer, ...)
#define LOG_INTERRUPT(...)
#define LOG_IOGA(...)
#define LOG_DMA(...)
#endif

DEVICE_ADDRESS_MAP_START(map, 32, interpro_ioga_device)
	AM_RANGE(0x0c, 0x1b) AM_READWRITE(dma_plotter_r, dma_plotter_w)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE(dma_plotter_eosl_r, dma_plotter_eosl_w)
	AM_RANGE(0x20, 0x2f) AM_READWRITE(dma_scsi_r, dma_scsi_w)
	AM_RANGE(0x30, 0x3f) AM_READWRITE(dma_floppy_r, dma_floppy_w)

	AM_RANGE(0x5c, 0x7f) AM_READWRITE16(icr_r, icr_w, 0xffffffff)
	AM_RANGE(0x80, 0x83) AM_READWRITE16(icr18_r, icr18_w, 0x0000ffff)
	AM_RANGE(0x80, 0x83) AM_READWRITE8(softint_r, softint_w, 0x00ff0000)
	AM_RANGE(0x80, 0x83) AM_READWRITE8(nmictrl_r, nmictrl_w, 0xff000000)

	AM_RANGE(0x88, 0x8b) AM_READWRITE(timer_prescaler_r, timer_prescaler_w)
	AM_RANGE(0x8c, 0x8f) AM_READWRITE(timer0_r, timer0_w)
	AM_RANGE(0x90, 0x93) AM_READWRITE(timer1_r, timer1_w)
	AM_RANGE(0x94, 0x97) AM_READ(error_address_r)
	AM_RANGE(0x98, 0x9b) AM_READ(error_businfo_r)
	AM_RANGE(0x9c, 0x9f) AM_READWRITE16(arbctl_r, arbctl_w, 0x0000ffff)

	AM_RANGE(0xa8, 0xab) AM_READWRITE(timer3_r, timer3_w)

	AM_RANGE(0xb0, 0xbf) AM_READWRITE16(softint_vector_r, softint_vector_w, 0xffffffff)
ADDRESS_MAP_END

const device_type INTERPRO_IOGA = device_creator<interpro_ioga_device>;

interpro_ioga_device::interpro_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_IOGA, "InterPro IOGA", tag, owner, clock, "ioga", __FILE__),
	m_out_nmi_func(*this),
	m_out_int_func(*this),
	m_memory_space(nullptr),
	m_dma_channel{
		{ 0,0,0,0,false, 0, {*this}, {*this} },
		{ 0,0,0,0,false, 0, {*this}, {*this} },
		{ 0,0,0,0,false, 0, {*this}, {*this} },
		{ 0,0,0,0,false, 0, {*this}, {*this} } },
	m_fdc_tc_func(*this)
{
}

void interpro_ioga_device::device_start()
{
	// resolve callbacks
	m_out_nmi_func.resolve();
	m_out_int_func.resolve();

	// TODO: parameterise the cammu name and space number
	// grab the main memory space from the mmu so we can do DMA to/from it
	device_memory_interface *mmu;
	siblingdevice("mmu")->interface(mmu);
	m_memory_space = &mmu->space(AS_0);

	for (int i = 0; i < IOGA_DMA_CHANNELS; i++)
	{
		m_dma_channel[i].device_r.resolve_safe(0xff);
		m_dma_channel[i].device_w.resolve();
	}

	m_fdc_tc_func.resolve();

	// allocate ioga timers
	m_timer[0] = timer_alloc(IOGA_TIMER_0);
	m_timer[1] = timer_alloc(IOGA_TIMER_1);
	m_timer[2] = timer_alloc(IOGA_TIMER_2);
	m_timer[3] = timer_alloc(IOGA_TIMER_3);

	for (auto & elem : m_timer)
		elem->enable(false);

	// allocate timer for DMA controller
	m_dma_timer = timer_alloc(IOGA_TIMER_DMA);
	m_dma_timer->adjust(attotime::never);
}

void interpro_ioga_device::device_reset()
{
	m_nmi_pending = false;

	m_interrupt_active = 0;
	m_irq_forced = 0;

	// configure timer 0 at 60Hz
	m_timer_reg[0] = 0;
	m_timer[0]->adjust(attotime::zero, IOGA_TIMER_0, attotime::from_hz(60));
}

/******************************************************************************
  Timers
******************************************************************************/
READ32_MEMBER(interpro_ioga_device::timer1_r)
{
	uint32_t result = m_timer1_count & IOGA_TIMER1_VMASK;

	// set the start bit if the timer is currently enabled
	if (m_timer[1]->enabled())
		result |= IOGA_TIMER1_START;
	else if (m_timer[1]->param())
		result |= IOGA_TIMER1_EXPIRED;

	return result;
}

READ32_MEMBER(interpro_ioga_device::timer3_r)
{
	uint32_t result = m_timer3_count & IOGA_TIMER3_VMASK;

	if (m_timer[3]->enabled())
		result |= IOGA_TIMER3_START;
	else if (m_timer[3]->param())
		result |= IOGA_TIMER3_EXPIRED;

	return result;
}

void interpro_ioga_device::write_timer(int timer, uint32_t value, device_timer_id id)
{
	switch (id)
	{
	case IOGA_TIMER_1:
		// disable the timer
		m_timer[timer]->enable(false);

		// store the timer count value
		m_timer1_count = value;

		// start the timer if necessary
		if (value & IOGA_TIMER1_START)
		{
			LOG_TIMER(1, "timer 1: started prescaler %d value %d\n", m_prescaler & 0x7fff, value & IOGA_TIMER1_VMASK);

			// FIXME: this division by 50 is sufficient to pass iogadiag timer 1 tests
			m_timer[timer]->adjust(attotime::zero, false, attotime::from_usec((m_prescaler & 0x7fff) / 50));
		}
		break;

	case IOGA_TIMER_3:
		// stop the timer so it won't trigger while we're fiddling with it
		m_timer[timer]->enable(false);

		// write the new value to the timer register
		m_timer3_count = value & IOGA_TIMER3_VMASK;

		// start the timer if necessary
		if (value & IOGA_TIMER3_START)
		{
			LOG_TIMER(3, "timer 3: started value %d\n", value & IOGA_TIMER3_VMASK);

			m_timer[timer]->adjust(attotime::zero, false, attotime::from_hz(XTAL_25MHz));
		}
		break;

	default:
		// save the value
		m_timer_reg[timer] = value;

		// timer_set(attotime::from_usec(500), id);

		LOG_TIMER(0xf, "timer %d: set to 0x%x (%d)\n", timer, m_timer_reg[timer], m_timer_reg[timer]);
		break;
	}
}

void interpro_ioga_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case IOGA_TIMER_0:
		m_timer_reg[0]++;
		set_irq_line(IOGA_TIMER0_IRQ, ASSERT_LINE);
		break;

	case IOGA_TIMER_1:
		// decrement timer count value
		m_timer1_count--;

		// check if timer has expired
		if (m_timer1_count == 0)
		{
			LOG_TIMER(1, "timer 1: stopped\n");

			// disable timer and set the zero flag
			timer.enable(false);
			timer.set_param(true);

			// throw an interrupt
			set_irq_line(IOGA_TIMER1_IRQ, ASSERT_LINE);
		}
		break;

	case IOGA_TIMER_3:
		// decrement timer count value
		m_timer3_count--;

		// check for expiry
		if (m_timer3_count == 0)
		{
			LOG_TIMER(3, "timer 3: stopped\n");

			// disable timer and set the zero flag
			timer.enable(false);
			timer.set_param(true);

			// throw an interrupt
			set_irq_line(IOGA_TIMER3_IRQ, ASSERT_LINE);
		}
		break;

	case IOGA_TIMER_DMA:
		// transfer data between device and main memory

		// TODO: figure out what indicates dma write (memory -> device)
		// TODO: implement multiple dma channels
		// TODO: virtual memory?

		if (!m_dma_channel[param].dma_active)
		{
			LOG_DMA("dma: transfer started, channel = %d, control 0x%08x, real address 0x%08x count 0x%08x\n",
				param, m_dma_channel[param].control, m_dma_channel[param].real_address, m_dma_channel[param].transfer_count);
			m_dma_channel[param].dma_active = true;
		}

		// while the device is requesting a data transfer and the transfer count is not zero
		while (m_dma_channel[param].drq_state &&  m_dma_channel[param].transfer_count)
		{
			// transfer a byte between device and memory
			if (true)
				m_memory_space->write_byte(m_dma_channel[param].real_address, m_dma_channel[param].device_r());
			else
				m_dma_channel[param].device_w(m_memory_space->read_byte(m_dma_channel[param].real_address));

			// increment addresses and decrement count
			m_dma_channel[param].real_address++;
			m_dma_channel[param].virtual_address++;
			m_dma_channel[param].transfer_count--;
		}

		// if there are no more bytes remaining, terminate the transfer
		if (m_dma_channel[param].transfer_count == 0)
		{
			LOG_DMA("dma: transfer stopped, control 0x%08x, real address 0x%08x count 0x%08x\n",
				m_dma_channel[param].control, m_dma_channel[param].fdc_real_address, m_dma_channel[param].transfer_count);

			if (param == IOGA_DMA_FLOPPY)
			{
				LOG_DMA("dma: asserting fdc terminal count line\n");

				m_fdc_tc_func(ASSERT_LINE);
				m_fdc_tc_func(CLEAR_LINE);
			}

			m_dma_channel[param].dma_active = false;
		}
	break;
	}
}

/******************************************************************************
 Interrupts
******************************************************************************/

static const uint16_t irq_enable_mask[IOGA_INTERRUPT_COUNT] =
{
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL | IOGA_INTERRUPT_ENABLE_INTERNAL, // external interrupt 0: SCSI
	IOGA_INTERRUPT_ENABLE_EXTERNAL | IOGA_INTERRUPT_ENABLE_INTERNAL, // external interrupt 1: floppy
	IOGA_INTERRUPT_ENABLE_EXTERNAL | IOGA_INTERRUPT_ENABLE_INTERNAL, // external interrupt 2: plotter
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,

	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,

	// internal interrupt 5: serial DMA - one interrupt enable per DMA channel
	IOGA_INTERRUPT_ENABLE_EXTERNAL << 0 | IOGA_INTERRUPT_ENABLE_EXTERNAL << 1 | IOGA_INTERRUPT_ENABLE_EXTERNAL << 2,
	IOGA_INTERRUPT_ENABLE_EXTERNAL,
	IOGA_INTERRUPT_ENABLE_EXTERNAL | IOGA_INTERRUPT_ENABLE_INTERNAL // external interrupt 12: Ethernet
};

void interpro_ioga_device::set_nmi_line(int state)
{
	switch (state)
	{
	case ASSERT_LINE:

		LOG_INTERRUPT("nmi: ctrl = 0x%02x\n", m_nmictrl);

		if ((m_nmictrl & IOGA_NMI_ENABLE) == IOGA_NMI_ENABLE)
		{
			// if edge triggered mode, clear enable in
			if (m_nmictrl & IOGA_NMI_EDGE)
				m_nmictrl &= ~IOGA_NMI_ENABLE_IN;

			m_nmi_pending = true;
			update_interrupt(ASSERT_LINE);
		}
		break;

	case CLEAR_LINE:
		m_nmi_pending = false;
		update_interrupt(ASSERT_LINE);
		break;
	}
}

void interpro_ioga_device::set_irq_line(int irq, int state)
{
	LOG_INTERRUPT("set_irq_line(%d, %d)\n", irq, state);
	switch (state)
	{
	case ASSERT_LINE:
		if (m_int_vector[irq] & irq_enable_mask[irq])
		{
			// set pending bit
			m_int_vector[irq] |= IOGA_INTERRUPT_PENDING;

			// update irq line state
			update_interrupt(state);
		}
		else
			LOG_INTERRUPT("received disabled interrupt irq %d vector 0x%04x\n", irq, m_int_vector[irq]);
		break;

	case CLEAR_LINE:
		// clear pending bit
		m_int_vector[irq] &= ~IOGA_INTERRUPT_PENDING;

		// update irq line state
		update_interrupt(state);
		break;
	}
}

void interpro_ioga_device::set_irq_soft(int irq, int state)
{
	LOG_INTERRUPT("set_irq_soft(%d, %d)\n", irq, state);
	switch (state)
	{
	case ASSERT_LINE:
		// set pending bit
		if (irq < 8)
			m_softint |= 1 << irq;
		else
			m_softint_vector[irq - 8] |= IOGA_INTERRUPT_PENDING;

		update_interrupt(state);
		break;

	case CLEAR_LINE:
		// clear pending bit
		if (irq < 8)
			m_softint &= ~(1 << irq);
		else
			m_softint_vector[irq - 8] &= ~IOGA_INTERRUPT_PENDING;

		// update irq line state
		update_interrupt(state);
		break;
	}
}

IRQ_CALLBACK_MEMBER(interpro_ioga_device::inta_cb)
{
	switch (irqline)
	{
	case INPUT_LINE_IRQ0:
		// FIXME: clear pending bit - can't rely on device callbacks
		switch (m_interrupt_active)
		{
		case IOGA_INTERRUPT_INTERNAL:
		case IOGA_INTERRUPT_EXTERNAL:
			m_int_vector[m_irq_current] &= ~IOGA_INTERRUPT_PENDING;
			break;

		case IOGA_INTERRUPT_SOFT_LO:
			m_softint &= ~(1 << m_irq_current);
			break;

		case IOGA_INTERRUPT_SOFT_HI:
			m_softint_vector[m_irq_current] &= ~IOGA_INTERRUPT_PENDING;
			break;
		}

		// clear irq line
		update_interrupt(CLEAR_LINE);

		// fall through to return interrupt vector
	case -1:
		// return vector for current interrupt without clearing irq line
		switch (m_interrupt_active)
		{
		case IOGA_INTERRUPT_EXTERNAL:
		case IOGA_INTERRUPT_INTERNAL:
			return m_int_vector[m_irq_current] & 0xff;

		case IOGA_INTERRUPT_SOFT_LO:
			return 0x8f + m_irq_current * 0x10;

		case IOGA_INTERRUPT_SOFT_HI:
			return m_softint_vector[m_irq_current] & 0xff;
		}
		break;

	case INPUT_LINE_NMI:
		// clear pending flag
		m_nmi_pending = false;

		// clear line
		update_interrupt(CLEAR_LINE);

		// return vector
		return 0;
	}

	return 0;
}

void interpro_ioga_device::update_interrupt(int state)
{
	switch (state)
	{
	case CLEAR_LINE:
		if (m_interrupt_active)
		{
			// the cpu has acknowledged the active interrupt, deassert the nmi/irq line
			m_interrupt_active == IOGA_INTERRUPT_NMI ? m_out_nmi_func(CLEAR_LINE) : m_out_int_func(CLEAR_LINE);

			// clear the active status
			m_interrupt_active = 0;
		}
		// fall through to handle any pending interrupts

	case ASSERT_LINE:
		// if an interrupt is currently active, don't do anything
		if (m_interrupt_active == 0)
		{
			// check for pending nmi
			if (m_nmi_pending)
			{
				m_interrupt_active = IOGA_INTERRUPT_NMI;

				m_out_nmi_func(ASSERT_LINE);
				return;
			}

			// check for any pending irq
			for (int i = 0; i < IOGA_INTERRUPT_COUNT; i++)
			{
				if (m_int_vector[i] & IOGA_INTERRUPT_PENDING)
				{
					m_interrupt_active = IOGA_INTERRUPT_INTERNAL; // TODO: flag internal/external
					m_irq_current = i;

					m_out_int_func(ASSERT_LINE);
					return;
				}
			}

			// check for any pending soft interrupts (low type)
			for (int i = 0; i < 8; i++)
			{
				if (m_softint & (1 << i))
				{
					m_interrupt_active = IOGA_INTERRUPT_SOFT_LO;
					m_irq_current = i;

					m_out_int_func(ASSERT_LINE);
					return;
				}
			}

			// check for any pending soft interrupts (high type)
			for (int i = 0; i < 8; i++)
			{
				if (m_softint_vector[i] & IOGA_INTERRUPT_PENDING)
				{
					m_interrupt_active = IOGA_INTERRUPT_SOFT_HI;
					m_irq_current = i;

					m_out_int_func(ASSERT_LINE);
					return;
				}
			}
		}
		break;
	}
}

WRITE16_MEMBER(interpro_ioga_device::icr_w)
{
	LOG_INTERRUPT("interrupt vector %d set to 0x%04x at pc 0x%08x\n", offset, data, space.device().safe_pc());

	// FIXME: now that the interrupt handling only depends on IOGA_INTERRUPT_PENDING, we might be able
	// to avoid this hack
	if (data & IOGA_INTERRUPT_PENDING)
	{
		m_irq_forced |= 1 << offset;
		m_int_vector[offset] = data & ~IOGA_INTERRUPT_PENDING;
	}
	else if (m_irq_forced & 1 << offset)
	{
		m_int_vector[offset] = data;

		// clear forced flag
		m_irq_forced &= ~(1 << offset);

		// force an interrupt
		set_irq_line(offset, ASSERT_LINE);
	}
	else
		m_int_vector[offset] = data;
}

WRITE8_MEMBER(interpro_ioga_device::softint_w)
{
	// save the existing value
	uint8_t previous = m_softint;

	// store the written value
	m_softint = data;

	// force soft interrupt for any bit written from 1 to 0
	for (int i = 0; i < 8; i++)
	{
		uint8_t mask = 1 << i;

		// check for transition from 1 to 0 and force a soft interrupt
		if (previous & mask && !(data & mask))
			set_irq_soft(i, ASSERT_LINE);
	}
}

WRITE8_MEMBER(interpro_ioga_device::nmictrl_w)
{
	// save the existing value
	uint8_t previous = m_nmictrl;

	// store the written value
	m_nmictrl = data;

	// force an nmi when pending bit is written low
	if (previous & IOGA_NMI_PENDING && !(data & IOGA_NMI_PENDING))
		set_nmi_line(ASSERT_LINE);
}

WRITE16_MEMBER(interpro_ioga_device::softint_vector_w)
{
	// save the existing value
	uint16_t previous = m_softint_vector[offset];

	// store the written value
	m_softint_vector[offset] = data;

	// check for transition from 1 to 0 and force a soft interrupt
	if (previous & IOGA_INTERRUPT_PENDING && !(data & IOGA_INTERRUPT_PENDING))
		set_irq_soft(offset + 8, ASSERT_LINE);
}

/******************************************************************************
 DMA
******************************************************************************/
void interpro_ioga_device::drq(int state, int channel)
{
	// this member is called when the device has data ready for reading via dma
	m_dma_channel[channel].drq_state = state;

	if (state)
	{
		// TODO: check if dma is enabled
		m_dma_timer->adjust(attotime::zero, channel);
	}
}
/*
0x94: error address reg: expect 0x7f200000 after bus error (from dma virtual address)
0x98: error cycle type: expect 0x52f0 (after failed dma?)
        0x5331 - forced berr with nmi/interrupts disabled?
        0xc2f0
        0x62f0
*/
// TODO: 7.0266 - forced BERR not working

uint32_t interpro_ioga_device::dma_r(address_space &space, offs_t offset, uint32_t mem_mask, int channel)
{
	switch (offset)
	{
	case 0:
		return m_dma_channel[channel].real_address;

	case 1:
		return m_dma_channel[channel].virtual_address;

	case 2:
		return m_dma_channel[channel].transfer_count;

	case 3:
		return m_dma_channel[channel].control;
	}

	logerror("dma_r: unknown channel %d\n", channel);
	return 0;
}

void interpro_ioga_device::dma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask, int channel)
{
	switch (offset)
	{
	case 0:
		m_dma_channel[channel].real_address = data;
		break;

	case 1:
		m_dma_channel[channel].virtual_address = data & ~0x3;
		break;

	case 2:
		m_dma_channel[channel].transfer_count = data;
		break;

	case 3:
		m_dma_channel[channel].control = data & IOGA_DMA_CTRL_WMASK;

		logerror("dma: channel = %d, control = 0x%08x, ra = 0x%08x, va = 0x%08x, tc = 0x%08x\n",
			channel, data, m_dma_channel[channel].real_address, m_dma_channel[channel].virtual_address, m_dma_channel[channel].transfer_count);

		// iogadiag test 7.0265
		if (data == IOGA_DMA_CTRL_START)
		{
			uint32_t mask = 0;

			switch (channel)
			{
			case IOGA_DMA_PLOTTER:
				mask = IOGA_ARBCTL_BGR_PLOT;
				break;

			case IOGA_DMA_SCSI:
				mask = IOGA_ARBCTL_BGR_SCSI;
				break;

			case IOGA_DMA_FLOPPY:
				mask = IOGA_ARBCTL_BGR_FDC;
				break;
			}

			// if bus grant is not enabled, set the busy flag
			if (!(m_arbctl & mask))
				m_dma_channel[channel].control |= IOGA_DMA_CTRL_BUSY;
		}
		break;
	}
}
