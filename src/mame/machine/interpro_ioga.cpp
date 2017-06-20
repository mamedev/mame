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

#define LOG_HWINT_ENABLE ((1<<2) | LOG_GENERAL)
#define LOG_DMA_ENABLE ((1<<2) | LOG_GENERAL)

#define LOG_GENERAL (1 << 31)

#define LOG_HWINT(interrupt, ...) if (LOG_HWINT_ENABLE & (1 << interrupt)) logerror(__VA_ARGS__)
#define LOG_DMA(channel, ...) if (LOG_DMA_ENABLE & (1 << channel)) logerror(__VA_ARGS__)

#define VERBOSE 0

#if VERBOSE
#define LOG_TIMER_MASK 0xff
#define LOG_TIMER(timer, ...) if (LOG_TIMER_MASK & (1 << timer)) logerror(__VA_ARGS__)
#define LOG_INTERRUPT(...) logerror(__VA_ARGS__)
#define LOG_IOGA(...) logerror(__VA_ARGS__)
#else
#define LOG_TIMER_MASK 0x00
#define LOG_TIMER(timer, ...)
#define LOG_INTERRUPT(...)
#define LOG_IOGA(...)
#endif

DEVICE_ADDRESS_MAP_START(map, 32, interpro_ioga_device)
	AM_RANGE(0x0c, 0x1b) AM_READWRITE(dma_plotter_r, dma_plotter_w)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE(dma_plotter_eosl_r, dma_plotter_eosl_w)
	AM_RANGE(0x20, 0x2f) AM_READWRITE(dma_scsi_r, dma_scsi_w)
	AM_RANGE(0x30, 0x3f) AM_READWRITE(dma_floppy_r, dma_floppy_w)
	AM_RANGE(0x40, 0x57) AM_READWRITE(dma_serial_r, dma_serial_w)

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
	AM_RANGE(0xac, 0xaf) AM_READWRITE(bus_timeout_r, bus_timeout_w) // boot code writes 0x64

	AM_RANGE(0xb0, 0xbf) AM_READWRITE16(softint_vector_r, softint_vector_w, 0xffffffff)
ADDRESS_MAP_END

DEFINE_DEVICE_TYPE(INTERPRO_IOGA, interpro_ioga_device, "ioga", "InterPro IOGA")

interpro_ioga_device::interpro_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_IOGA, tag, owner, clock),
	m_out_nmi_func(*this),
	m_out_irq_func(*this),
	m_memory_space(nullptr),
	m_dma_channel{
		{ 0,0,0,0,false, 0, {*this}, {*this} },
		{ 0,0,0,0,false, 0, {*this}, {*this} },
		{ 0,0,0,0,false, 0, {*this}, {*this} },
		{ 0,0,0,0,false, 0, {*this}, {*this} } },
	m_fdc_tc_func(*this)
{
}

#if LOG_HWINT_ENABLE
static const char *interrupt_source[IOGA_INTERRUPT_COUNT] = {
	// internal
	"timer 2",
	"timer 3",
	// external
	"SCSI",
	"floppy",
	"plotter",
	"SRX / CBUS 0",
	"SRX / CBUS 1",
	"SRX / CBUS 2",
	"VB",
	"",
	"CBUS 3",
	"clock / calendar",
	"clock / SGA",
	// internal
	"mouse",
	"timer 0",
	"timer 1",
	"serial DMA",
	// external
	"serial",
	"Ethernet",
};
#endif

void interpro_ioga_device::device_start()
{
	// resolve callbacks
	m_out_nmi_func.resolve();
	m_out_irq_func.resolve();

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

	for (auto & timer : m_timer)
		timer->enable(false);

	// allocate timer for DMA controller
	m_dma_timer = timer_alloc(IOGA_TIMER_DMA);
	m_dma_timer->adjust(attotime::never);

	m_ioga_clock = timer_alloc(IOGA_CLOCK);
	m_ioga_clock->adjust(attotime::never);
}

void interpro_ioga_device::device_reset()
{
	// initialise interrupt state
	m_active_interrupt_type = IOGA_INTERRUPT_NONE;
	m_hwint_forced = 0;
	m_nmi_state = CLEAR_LINE;
	m_irq_state = CLEAR_LINE;
	m_int_line = 0;

	// configure timer 0 at 60Hz
	m_timer_reg[0] = 0;
	//m_timer[0]->adjust(attotime::zero, IOGA_TIMER_0, attotime::from_hz(60));

	// configure ioga clock timer
	m_ioga_clock->adjust(attotime::zero, IOGA_CLOCK, attotime::from_hz(clock()));
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

void interpro_ioga_device::write_timer(int timer, u32 value, device_timer_id id)
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
		m_hwicr[IOGA_TIMER0_IRQ] |= IOGA_INTERRUPT_PENDING;
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
			m_hwicr[IOGA_TIMER1_IRQ] |= IOGA_INTERRUPT_PENDING;
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
			m_hwicr[IOGA_TIMER3_IRQ] |= IOGA_INTERRUPT_PENDING;
		}
		break;

	case IOGA_TIMER_DMA:
		dma_clock(param);
		break;

	case IOGA_CLOCK:
		interrupt_clock();
		break;
	}
}

/******************************************************************************
 Interrupts
******************************************************************************/
static const u16 irq_enable_mask[IOGA_INTERRUPT_COUNT] =
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

bool interpro_ioga_device::nmi(int state)
{
	if (m_nmi_state != state)
	{
		m_nmi_state = state;
		m_out_nmi_func(m_nmi_state);

		return true;
	}
	else
		return false;
}

bool interpro_ioga_device::irq(int state)
{
	if (m_irq_state != state)
	{
		m_irq_state = state;
		m_out_irq_func(m_irq_state);

		return true;
	}
	else
		return false;
}

void interpro_ioga_device::set_nmi_line(int state)
{
	LOG_INTERRUPT("set_nmi_line(%d)\n", state);
	switch (state)
	{
	case ASSERT_LINE:
#if 0
		if ((m_nmictrl & IOGA_NMI_ENABLE) == IOGA_NMI_ENABLE)
		{
			// if edge triggered mode, clear enable in
			if (m_nmictrl & IOGA_NMI_EDGE)
				m_nmictrl &= ~IOGA_NMI_ENABLE_IN;
		}
#endif
		m_nmictrl |= NMI_PENDING;
		break;

	case CLEAR_LINE:
		m_nmictrl &= ~NMI_PENDING;
		break;
	}
}

void interpro_ioga_device::set_irq_line(int irq, int state)
{
	LOG_HWINT(irq, "set_irq_line(%d, %d)\n", irq, state);
	switch (state)
	{
	case ASSERT_LINE:
		// set pending bit
		m_int_line |= (1 << irq);
		m_hwicr[irq] |= IOGA_INTERRUPT_PENDING;
		break;

	case CLEAR_LINE:
		// clear pending bit
		m_int_line &= (1 << irq);
		m_hwicr[irq] &= ~IOGA_INTERRUPT_PENDING;
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
			m_swicr[irq - 8] |= IOGA_INTERRUPT_PENDING;
		break;

	case CLEAR_LINE:
		// clear pending bit
		if (irq < 8)
			m_softint &= ~(1 << irq);
		else
			m_swicr[irq - 8] &= ~IOGA_INTERRUPT_PENDING;
		break;
	}
}

IRQ_CALLBACK_MEMBER(interpro_ioga_device::inta_cb)
{
	int vector = 0;

	switch (irqline)
	{
	case INPUT_LINE_IRQ0:
		// FIXME: clear pending bit - can't rely on device callbacks
		switch (m_active_interrupt_type)
		{
		case IOGA_INTERRUPT_INTERNAL:
		case IOGA_INTERRUPT_EXTERNAL:
			m_hwicr[m_active_interrupt_number] &= ~IOGA_INTERRUPT_PENDING;
			break;

		case IOGA_INTERRUPT_SOFT_LO:
			m_softint &= ~(1 << m_active_interrupt_number);
			break;

		case IOGA_INTERRUPT_SOFT_HI:
			m_swicr[m_active_interrupt_number] &= ~IOGA_INTERRUPT_PENDING;
			break;
		}

		// fall through to return interrupt vector
	case -1:
		// return vector for current interrupt without clearing irq line
		switch (m_active_interrupt_type)
		{
		case IOGA_INTERRUPT_INTERNAL:
		case IOGA_INTERRUPT_EXTERNAL:
			vector = m_hwicr[m_active_interrupt_number] & 0xff;
			break;

		case IOGA_INTERRUPT_SOFT_LO:
			vector = 0x8f + m_active_interrupt_number * 0x10;
			break;

		case IOGA_INTERRUPT_SOFT_HI:
			vector = m_swicr[m_active_interrupt_number] & 0xff;
			break;
		}

		// interrupt is acknowledged
		if (irqline == INPUT_LINE_IRQ0)
			m_active_interrupt_type = IOGA_INTERRUPT_NONE;
		break;

	case INPUT_LINE_NMI:
		// clear pending flag
		m_nmictrl &= ~NMI_PENDING;
		m_active_interrupt_type = IOGA_INTERRUPT_NONE;
		break;
	}

	return vector;
}

void interpro_ioga_device::interrupt_clock()
{
	// called on every ioga clock cycle
	// if there are no active interrupts, raise the next pending one

	// don't do anything if any interrupts are currently being serviced
	if (m_active_interrupt_type != IOGA_INTERRUPT_NONE)
		return;

	// if nmi line is asserted, clear it
	if (nmi(CLEAR_LINE))
		return;

	// if irq line is asserted, clear it
	if (irq(CLEAR_LINE))
		return;

	// check for pending nmi
	if (m_nmictrl & NMI_PENDING)
	{
		m_active_interrupt_type = IOGA_INTERRUPT_NMI;
		nmi(ASSERT_LINE);
		return;
	}

	// scan all hardware interrupts
	for (int i = 0; i < IOGA_INTERRUPT_COUNT; i++)
	{
		// check if there is a pending interrupt
		if (m_hwicr[i] & IOGA_INTERRUPT_PENDING)
		{
			// check if from an external device or internal to ioga
			bool external = m_int_line & (1 << i);

			// check if masked
			if (m_hwicr[i] & irq_enable_mask[i]) //(external ? IRQ_ENABLE_EXTERNAL : IRQ_ENABLE_INTERNAL))
			{
				LOG_HWINT(i, "accepting interrupt %d - %s (%s)\n", i, interrupt_source[i], external ? "external" : "internal");

				m_active_interrupt_type = external ? IOGA_INTERRUPT_EXTERNAL : IOGA_INTERRUPT_INTERNAL;
				m_active_interrupt_number = i;

				irq(ASSERT_LINE);
				return;
			}
		}
	}

	// check for any pending soft interrupts (low type)
	for (int i = 0; i < 8; i++)
	{
		if (m_softint & (1 << i))
		{
			m_active_interrupt_type = IOGA_INTERRUPT_SOFT_LO;
			m_active_interrupt_number = i;

			irq(ASSERT_LINE);
			return;
		}
	}

	// check for any pending soft interrupts (high type)
	for (int i = 0; i < 8; i++)
	{
		if (m_swicr[i] & IOGA_INTERRUPT_PENDING)
		{
			m_active_interrupt_type = IOGA_INTERRUPT_SOFT_HI;
			m_active_interrupt_number = i;

			irq(ASSERT_LINE);
			return;
		}
	}
}

WRITE16_MEMBER(interpro_ioga_device::icr_w)
{
	/*
	* It appears that writing the pending flag high and then low again is intended to
	* "force" an interrupt to be generated. We record the initial write in m_hwint_forced,
	* and when a subsequent write occurrs, turn the pending bit on to trigger the interrupt.
	*
	* FIXME: should we only flag a forced interrupt if pending is written high from low?
	*/

	LOG_HWINT(offset, "interrupt vector %d set to 0x%04x at %s\n", offset, data, machine().describe_context());
#if 1
	if (data & IOGA_INTERRUPT_PENDING)
	{
		m_hwint_forced |= 1 << offset;
		m_hwicr[offset] = (m_hwicr[offset] & IOGA_INTERRUPT_PENDING) | (data & ~IOGA_INTERRUPT_PENDING);
	}
	else if (m_hwint_forced & 1 << offset)
	{
		m_hwicr[offset] = data;

		// clear forced flag
		m_hwint_forced &= ~(1 << offset);

		// force an interrupt
		m_hwicr[offset] |= IOGA_INTERRUPT_PENDING;
	}
	else
		m_hwicr[offset] = data;
#else
	if (data & IOGA_INTERRUPT_PENDING)
		m_hwint_forced |= 1 << offset;

	if (data & IOGA_INTERRUPT_ENABLE_EXTERNAL)
		m_hwicr[offset] = data;
	else
		m_hwicr[offset] = data & ~IOGA_INTERRUPT_PENDING;
#endif
}

WRITE8_MEMBER(interpro_ioga_device::softint_w)
{
	// save the existing value
	u8 previous = m_softint;

	// store the written value
	m_softint = data;

	// force soft interrupt for any bit written from 1 to 0
	for (int i = 0; i < 8; i++)
	{
		u8 mask = 1 << i;

		// check for transition from 1 to 0 and force a soft interrupt
		if (previous & mask && !(data & mask))
			set_irq_soft(i, ASSERT_LINE);
	}
}

WRITE8_MEMBER(interpro_ioga_device::nmictrl_w)
{
	LOG_INTERRUPT("nmictrl = 0x%02x (%s)\n", data, machine().describe_context());

#if 0
	// save the existing value
	uint8_t previous = m_nmictrl;

	// store the written value
	m_nmictrl = data;

	// force an nmi when pending bit is written low
	if (previous & NMI_PENDING && !(data & NMI_PENDING))
		set_nmi_line(ASSERT_LINE);
#else
#if 0
	if (data & NMI_PENDING)
	{
		m_nmi_forced = true;
		m_nmictrl = (m_nmictrl & NMI_PENDING) | (data & ~NMI_PENDING);
	}
	else if (m_nmi_forced)
	{
		m_nmi_forced = false;

		m_nmictrl = data | NMI_PENDING;
	}
	else
		m_nmictrl = data;
#endif
	m_nmictrl = data & ~NMI_PENDING;
#endif
}

WRITE16_MEMBER(interpro_ioga_device::softint_vector_w)
{
	// save the existing value
	u16 previous = m_swicr[offset];

	// store the written value
	m_swicr[offset] = data;

	// check for transition from 1 to 0 and force a soft interrupt
	if (previous & IOGA_INTERRUPT_PENDING && !(data & IOGA_INTERRUPT_PENDING))
		set_irq_soft(offset + 8, ASSERT_LINE);
}

/******************************************************************************
 DMA
******************************************************************************/
void interpro_ioga_device::dma_clock(int channel)
{
	// transfer data between device and main memory

	// TODO: virtual memory?

	// handle device to memory dma
	if (m_dma_channel[channel].drq_state)
	{
		if (!m_dma_channel[channel].dma_active)
		{
			LOG_DMA(channel, "dma: transfer from device started, channel = %d, control 0x%08x, real address 0x%08x count 0x%08x\n",
				channel, m_dma_channel[channel].control, m_dma_channel[channel].real_address, m_dma_channel[channel].transfer_count);
			m_dma_channel[channel].dma_active = true;
		}

		// while the device is requesting a data transfer and the transfer count is not zero
		while (m_dma_channel[channel].drq_state && m_dma_channel[channel].transfer_count)
		{
			// transfer from the device to memory
			m_memory_space->write_byte(m_dma_channel[channel].real_address, m_dma_channel[channel].device_r());

			// increment addresses and decrement count
			m_dma_channel[channel].real_address++;
			m_dma_channel[channel].virtual_address++;
			m_dma_channel[channel].transfer_count--;
		}

		// if there are no more bytes remaining, terminate the transfer
		if (m_dma_channel[channel].transfer_count == 0)
		{
			LOG_DMA(channel, "dma: transfer from device completed, control 0x%08x, real address 0x%08x count 0x%08x\n",
				m_dma_channel[channel].control, m_dma_channel[channel].real_address, m_dma_channel[channel].transfer_count);

			if (channel == IOGA_DMA_FLOPPY)
			{
				LOG_DMA(channel, "dma: asserting fdc terminal count line\n");

				m_fdc_tc_func(ASSERT_LINE);
				m_fdc_tc_func(CLEAR_LINE);
			}

			m_dma_channel[channel].dma_active = false;
		}
	}
	else // memory to device dma
	{
		// get access to the bus
		{
			// iogadiag test 7.0265
			u32 mask = 0;

			switch (channel)
			{
			case IOGA_DMA_PLOTTER:
				mask = ARBCTL_BGR_PLOT;
				break;

			case IOGA_DMA_SCSI:
				mask = ARBCTL_BGR_SCSI;
				break;

			case IOGA_DMA_FLOPPY:
				mask = ARBCTL_BGR_FDC;
				break;
			}

			// if bus grant is not enabled, set the busy flag
			if (!(m_arbctl & mask))
			{
				LOG_DMA(channel, "dma: delay for bus grant channel %d\n", channel);

				m_dma_channel[channel].control |= IOGA_DMA_CTRL_BUSY;
				m_dma_timer->adjust(attotime::from_hz(clock()), channel);

				return;
			}
			else
				m_dma_channel[channel].control &= ~IOGA_DMA_CTRL_BUSY;
		}

		if (!m_dma_channel[channel].dma_active)
		{
			LOG_DMA(channel, "dma: transfer to device begun, channel %d, control 0x%08x, real address 0x%08x, count 0x%08x\n",
				channel, m_dma_channel[channel].control, m_dma_channel[channel].real_address, m_dma_channel[channel].transfer_count);
			m_dma_channel[channel].dma_active = true;
		}

		while (m_dma_channel[channel].transfer_count)
		{
			// transfer from memory to the device
			m_dma_channel[channel].device_w(m_memory_space->read_byte(m_dma_channel[channel].real_address));

			// increment addresses and decrement count
			m_dma_channel[channel].real_address++;
			m_dma_channel[channel].virtual_address++;
			m_dma_channel[channel].transfer_count--;
		}

		// TODO: do we need the floppy terminal count line here?

		m_dma_channel[channel].control |= IOGA_DMA_CTRL_TCZERO;
		m_dma_channel[channel].dma_active = false;

		// TODO: do we need to throw an interrupt?
		LOG_DMA(channel, "dma: transfer to device ended, channel %d, control 0x%08x, real address 0x%08x, count 0x%08x\n",
			channel, m_dma_channel[channel].control, m_dma_channel[channel].real_address, m_dma_channel[channel].transfer_count);
		
		// dma ctrl = 0xbf000600
		//          = 0xff000600
		//          = 0x63xxxxxx
		// 600 = scsi channel?
		// b = 1011
		// f = 1111
		// 6 = 0101
		// -> bit 0x4 = read/write?
	}
}

void interpro_ioga_device::drq(int state, int channel)
{
	// this member is called when the device has data ready for reading via dma
	m_dma_channel[channel].drq_state = state;

	if (state)
	{
		LOG_DMA(channel, "dma: recieved drq for channel %d\n", channel);

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

u32 interpro_ioga_device::dma_r(address_space &space, offs_t offset, u32 mem_mask, int channel)
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

void interpro_ioga_device::dma_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, int channel)
{
	switch (offset)
	{
	case 0:
		LOG_DMA(channel, "channel %d real address = 0x%08x (%s)\n", channel, data, machine().describe_context());
		m_dma_channel[channel].real_address = data;
		break;

	case 1:
		LOG_DMA(channel, "channel %d virtual address = 0x%08x (%s)\n", channel, data, machine().describe_context());
		m_dma_channel[channel].virtual_address = data & ~0x3;
		break;

	case 2:
		LOG_DMA(channel, "channel %d transfer count = 0x%08x (%s)\n", channel, data, machine().describe_context());
		m_dma_channel[channel].transfer_count = data;
		break;

	case 3:
		LOG_DMA(channel, "channel %d control = 0x%08x (%s)\n", channel, data, machine().describe_context());
		m_dma_channel[channel].control = data & IOGA_DMA_CTRL_WMASK;

		// start dma transfer if necessary
		if (data & IOGA_DMA_CTRL_START)
			m_dma_timer->adjust(attotime::from_hz(clock()), channel);
		break;
	}
}

u32 interpro_ioga_device::dma_serial_r(address_space &space, offs_t offset, u32 mem_mask)
{
	int channel = offset >> 1;

	if (offset & 1)
		return m_dma_serial[channel].control;
	else
		return m_dma_serial[channel].address;
}

void interpro_ioga_device::dma_serial_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	int channel = offset >> 1;

	if (offset & 1)
		m_dma_serial[channel].control = (m_dma_serial[channel].control & ~mem_mask) | data;
	else
		m_dma_serial[channel].address = (m_dma_serial[channel].address & ~mem_mask) | data;
}

READ32_MEMBER(interpro_ioga_device::error_businfo_r)
{
	u32 result = m_error_businfo;

	// clear register after reading
	m_error_businfo = 0;

	return result;
}