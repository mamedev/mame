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

// the following enables some hacks which will allow all iogadiag tests to complete successfully, but also breaks scsi dma
#define IOGA_DMA_DIAG_HACK 0

#define LOG_GENERAL      (1 << 31)
#define LOG_HWINT_ENABLE 0
#define LOG_DMA_ENABLE   0
#define LOG_TIMER_ENABLE 0

#define VERBOSE 0

#if VERBOSE
#define LOG_INTERRUPT(...)        logerror(__VA_ARGS__)
#define LOG_TIMER(timer, ...)     if (LOG_TIMER_ENABLE & (1 << timer)) logerror(__VA_ARGS__)
#define LOG_HWINT(interrupt, ...) if (LOG_HWINT_ENABLE & (1 << interrupt)) logerror(__VA_ARGS__)
#define LOG_DMA(channel, ...)     if (LOG_DMA_ENABLE & (1 << channel)) logerror(__VA_ARGS__)
#define LOG_ETH(...)              logerror(__VA_ARGS__)
#else
#define LOG_INTERRUPT(...)
#define LOG_TIMER(timer, ...)
#define LOG_HWINT(interrupt, ...)
#define LOG_DMA(channel, ...)
#define LOG_ETH(...)
#endif

DEVICE_ADDRESS_MAP_START(map, 32, interpro_ioga_device)
	AM_RANGE(0x00, 0x03) AM_READWRITE(eth_remap_r, eth_remap_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(eth_map_page_r, eth_map_page_w)
	AM_RANGE(0x08, 0x0b) AM_READWRITE(eth_control_r, eth_control_w)

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
		{ 0,0,0,0,IDLE, {*this}, {*this}, ARBCTL_BGR_PLOT, IOGA_DMA_PLOTTER, "plotter" },
		{ 0,0,0,0,IDLE, {*this}, {*this}, ARBCTL_BGR_SCSI, IOGA_DMA_SCSI, "scsi" },
		{ 0,0,0,0,IDLE, {*this}, {*this}, ARBCTL_BGR_FDC, IOGA_DMA_FLOPPY, "floppy" },
		{ 0,0,0,0,IDLE, {*this}, {*this}, ARBCTL_BGR_SER0 | ARBCTL_BGR_SER1 | ARBCTL_BGR_SER2, IOGA_DMA_SERIAL, "serial" } },
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
	m_memory_space = &mmu->space(0);

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

	// allocate ioga interrupt and dma timer
	m_ioga_clock = timer_alloc(IOGA_CLOCK);
	m_ioga_clock->adjust(attotime::never);
}

void interpro_ioga_device::device_reset()
{
	// initialise interrupt state
	m_active_interrupt_type = IOGA_INTERRUPT_NONE;
	m_hwint_forced = 0;
	m_nmi_pending = false;
	m_nmi_state = CLEAR_LINE;
	m_irq_state = CLEAR_LINE;
	m_int_line = 0;

	// configure timer 0 at 60Hz
	m_timer_reg[0] = 0;
	//m_timer[0]->adjust(attotime::zero, IOGA_TIMER_0, attotime::from_hz(60));

	// configure ioga interrupt and dma timer
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

	case IOGA_CLOCK:
		interrupt_clock();
		dma_clock();
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
	LOG_INTERRUPT("nmi: set_nmi_line(%d)\n", state);
	switch (state)
	{
	case ASSERT_LINE:
		m_nmi_pending = true;
		break;

	case CLEAR_LINE:
		m_nmi_pending = false;
		break;
	}
}

void interpro_ioga_device::set_irq_line(int irq, int state)
{
	LOG_HWINT(irq, "irq: set_irq_line(%d, %d)\n", irq, state);
	switch (state)
	{
	case ASSERT_LINE:
		// set pending bit
		m_int_line |= (1 << irq);
		m_hwicr[irq] |= IOGA_INTERRUPT_PENDING;
		break;

	case CLEAR_LINE:
		// clear pending bit
		m_int_line &= ~(1 << irq);
		m_hwicr[irq] &= ~IOGA_INTERRUPT_PENDING;
		break;
	}
}

void interpro_ioga_device::set_irq_soft(int irq, int state)
{
	LOG_INTERRUPT("irq: set_irq_soft(%d, %d)\n", irq, state);
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
		// return vector for current interrupt without clearing pending flag
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
		m_active_interrupt_type = IOGA_INTERRUPT_NONE;
		m_nmi_pending = false;
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
	if (m_nmi_pending)
	{
		// check if nmi is enabled
		if (((m_nmictrl & NMI_IE) == NMI_IE)
		|| ((m_nmictrl & (NMI_ALL | NMI_ENABLE1)) == (NMI_ALL | NMI_ENABLE1)))
		{
			LOG_INTERRUPT("nmi: accepting nmi\n");

			// if level triggered, disable input from pin
			if ((m_nmictrl & NMI_EDGE) == 0)
				m_nmictrl &= ~NMI_ENABLE2;

			m_active_interrupt_type = IOGA_INTERRUPT_NMI;
			nmi(ASSERT_LINE);
			return;
		}
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
				LOG_HWINT(i, "irq: accepting interrupt %d - %s (%s)\n", i, interrupt_source[i], external ? "external" : "internal");

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

	LOG_HWINT(offset, "irq: interrupt vector %d = 0x%04x (%s)\n", offset, data, machine().describe_context());

	if (data & IOGA_INTERRUPT_PENDING)
	{
		// record interrupt pending forced
		m_hwint_forced |= 1 << offset;

		// store all bits except pending
		m_hwicr[offset] = (m_hwicr[offset] & IOGA_INTERRUPT_PENDING) | (data & ~IOGA_INTERRUPT_PENDING);
	}
	else if (m_hwint_forced & 1 << offset)
	{
		// interrupt is being forced
		m_hwicr[offset] = data;

		// clear forced flag
		m_hwint_forced &= ~(1 << offset);

		// force an interrupt
		m_hwicr[offset] |= IOGA_INTERRUPT_PENDING;
	}
	else
		// otherwise just store the value
		m_hwicr[offset] = data;
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
	LOG_INTERRUPT("nmi: nmictrl = 0x%02x, nmi_pending = %d (%s)\n", data, m_nmi_pending, machine().describe_context());

	// check for a forced nmi (NMI_NEGPOL written from 1 to 0 with NMI_IE set)
	if ((m_nmictrl & NMI_NEGPOL) && (data & (NMI_NEGPOL | NMI_IE)) == NMI_IE)
	{
		LOG_INTERRUPT("nmi: forced nmi\n");
		m_nmi_pending = true;
	}

	m_nmictrl = data;
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
void interpro_ioga_device::dma_clock()
{
	for (auto &dma_channel : m_dma_channel)
	{
		switch (dma_channel.state)
		{
		case IDLE:
			break;

		case COMMAND:
			// start a command
			// HACK: busy flag indicates bus access required?
			if (dma_channel.control & DMA_CTRL_BUSY)
				dma_channel.state = WAIT;
			else
				dma_channel.state = COMPLETE;
			break;

		case WAIT:
			// if bus grant is enabled, clear wait flag and execute transfer
			if (m_arbctl & dma_channel.arb_mask)
			{
				// clear bus wait flag
				dma_channel.control &= ~DMA_CTRL_WAIT;

				LOG_DMA(dma_channel.channel, "dma: transfer %s device begun, channel = %d, control 0x%08x, real address 0x%08x, virtual address 0x%08x, count 0x%08x\n",
					(dma_channel.control & DMA_CTRL_WRITE) ? "to" : "from",
					dma_channel.channel, dma_channel.control, dma_channel.real_address, dma_channel.virtual_address, dma_channel.transfer_count);

				dma_channel.state = TRANSFER;
			}
			else
				// (7.0265) set the bus grant wait flag
				dma_channel.control |= DMA_CTRL_WAIT;
			break;

		case TRANSFER:
			// if there are no more bytes remaining, terminate the transfer
			if (dma_channel.transfer_count == 0)
			{
				LOG_DMA(dma_channel.channel, "dma: transfer %s device ended, channel = %d, control 0x%08x, real address 0x%08x, virtual address 0x%08x, count 0x%08x\n",
					(dma_channel.control & DMA_CTRL_WRITE) ? "to" : "from",
					dma_channel.channel, dma_channel.control, dma_channel.real_address, dma_channel.virtual_address, dma_channel.transfer_count);

				if (dma_channel.channel == IOGA_DMA_FLOPPY)
				{
					LOG_DMA(dma_channel.channel, "dma: asserting fdc terminal count line\n");

					m_fdc_tc_func(ASSERT_LINE);
					m_fdc_tc_func(CLEAR_LINE);
				}

				// set transfer count zero flag
				dma_channel.control |= DMA_CTRL_TCZERO;
				dma_channel.state = COMPLETE;
			}
#if IOGA_DMA_DIAG_HACK
			else
#define TAG ((dma_channel.control & DMA_CTRL_TAG) >> 3)

				// hacks for forced dma bus error diagnostic tests
				if ((dma_channel.control & 0xfe000000 && dma_channel.control & 0xe00) || ((dma_channel.control & DMA_CTRL_WMASK) == 0x41000000))
				if (dma_channel.real_address & 0xff000000 || dma_channel.real_address == 0)
				{
					LOG_DMA(dma_channel.channel, "dma: forced bus error hack, control 0x%08x\n", dma_channel.control);

					// (7.0267) trigger an interrupt
					m_hwicr[dma_channel.channel + 1] |= IOGA_INTERRUPT_PENDING;

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
			break;

		case COMPLETE:
			// clear busy flag
			dma_channel.control &= ~DMA_CTRL_BUSY;
			dma_channel.state = IDLE;
			break;
		}
	}
}

void interpro_ioga_device::drq(int state, int channel)
{
	// TODO: implement virtual addressing when DMA_CTRL_VIRTUAL is set

	struct dma &dma_channel = m_dma_channel[channel];

	if (state)
	{
		LOG_DMA(channel, "dma: drq for channel %d asserted\n", channel);

		if (dma_channel.state == TRANSFER && dma_channel.transfer_count)
		{
			// transfer from the memory to device or device to memory
			if (dma_channel.control & DMA_CTRL_WRITE)
				dma_channel.device_w(m_memory_space->read_byte(dma_channel.real_address));
			else
				m_memory_space->write_byte(dma_channel.real_address, dma_channel.device_r());

			// increment addresses and decrement count
			dma_channel.real_address++;
			dma_channel.virtual_address++;
			dma_channel.transfer_count--;
		}
	}
	else
		LOG_DMA(channel, "dma: drq for channel %d deasserted\n", channel);
}

u32 interpro_ioga_device::dma_r(address_space &space, offs_t offset, u32 mem_mask, int channel)
{
	struct dma &dma_channel = m_dma_channel[channel];

	switch (offset)
	{
	case 0:
		return dma_channel.real_address;

	case 1:
		return dma_channel.virtual_address;

	case 2:
		return dma_channel.transfer_count;

	case 3:
		return dma_channel.control;

	default:
		logerror("dma_r: unknown dma register %d\n", offset);
		return 0;
	}
}

void interpro_ioga_device::dma_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, int channel)
{
	struct dma &dma_channel = m_dma_channel[channel];

	switch (offset)
	{
	case 0:
		LOG_DMA(channel, "dma: channel %d real address = 0x%08x (%s)\n", channel, data, machine().describe_context());
		dma_channel.real_address = data;
		break;

	case 1:
		LOG_DMA(channel, "dma: channel %d virtual address = 0x%08x (%s)\n", channel, data, machine().describe_context());
		dma_channel.virtual_address = data & ~0x3;
		break;

	case 2:
		LOG_DMA(channel, "dma: channel %d transfer count = 0x%08x (%s)\n", channel, data, machine().describe_context());
		dma_channel.transfer_count = data;
		break;

	case 3:
		LOG_DMA(channel, "dma: channel %d control = 0x%08x (%s)\n", channel, data, machine().describe_context());

		// (7.0272) if bus error flag is set, clear existing bus error (otherwise retain existing state)
		if (data & DMA_CTRL_BERR)
			dma_channel.control = data & DMA_CTRL_WMASK;
		else
			dma_channel.control = (data & DMA_CTRL_WMASK) | (dma_channel.control & DMA_CTRL_BERR);

		dma_channel.state = COMMAND;
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
	{
		LOG_DMA(IOGA_DMA_SERIAL, "dma: channel %d(%d) control = 0x%08x, mask = 0x%08x (%s)\n", IOGA_DMA_SERIAL, channel, data, mem_mask, machine().describe_context());

		m_dma_serial[channel].control = (m_dma_serial[channel].control & ~mem_mask) | data;
	}
	else
	{
		LOG_DMA(IOGA_DMA_SERIAL, "dma: channel %d(%d) address = 0x%08x, mask = 0x%08x (%s)\n", IOGA_DMA_SERIAL, channel, data, mem_mask, machine().describe_context());

		m_dma_serial[channel].address = (m_dma_serial[channel].address & ~mem_mask) | data;
	}
}

READ32_MEMBER(interpro_ioga_device::error_businfo_r)
{
	u32 result = m_error_businfo;

	// clear register after reading
	m_error_businfo = 0;

	return result;
}

WRITE16_MEMBER(interpro_ioga_device::arbctl_w)
{
	LOG_DMA(31, "dma: arbctl = 0x%04x (%s)\n", data, machine().describe_context());

	m_arbctl = data;
}

WRITE32_MEMBER(interpro_ioga_device::eth_remap_w)
{
	LOG_ETH("eth: remap = 0x%08x (%s)\n", data, machine().describe_context());

	m_eth_remap = data & ~0xf;
}

WRITE32_MEMBER(interpro_ioga_device::eth_map_page_w)
{
	LOG_ETH("eth: remap page = 0x%08x (%s)\n", data, machine().describe_context());

	m_eth_map_page = data & ~0xf;
}

WRITE32_MEMBER(interpro_ioga_device::eth_control_w)
{
	LOG_ETH("eth: control = 0x%08x (%s)\n", data, machine().describe_context());

	/* (7.0202) eth ctrl register input test patterns and expected outputs are:

	   7809 -> 4000

	   3809 -> 0800
	   3409 -> 0400
	   3209 -> 0200
	   3109 -> 0100

	   3089 -> 80
	   3049 -> 40
	   3019 -> 10
	   300b -> 02
	*/
	if (data & ETH_RESET)
		m_eth_control = ETH_RESET;
	else
		m_eth_control = data & 0x0ff2;
}
