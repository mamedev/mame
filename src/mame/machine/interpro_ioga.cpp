// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "interpro_ioga.h"

#define VERBOSE 0
#if VERBOSE
#define LOG_TIMER(...) logerror(__VA_ARGS__)
#define LOG_INTERRUPT(...) logerror(__VA_ARGS__)
#define LOG_IOGA(...) logerror(__VA_ARGS__)
#else
#define LOG_TIMER(...)
#define LOG_INTERRUPT(...)
#define LOG_IOGA(...)
#endif

DEVICE_ADDRESS_MAP_START(map, 32, interpro_ioga_device)
	AM_RANGE(0x30, 0x3f) AM_READWRITE(fdc_dma_r, fdc_dma_w)

	AM_RANGE(0x5c, 0x7f) AM_READWRITE16(icr_r, icr_w, 0xffffffff)
	AM_RANGE(0x80, 0x83) AM_READWRITE16(icr18_r, icr18_w, 0x0000ffff)
	AM_RANGE(0x80, 0x83) AM_READWRITE8(softint_r, softint_w, 0x00ff0000)
	AM_RANGE(0x80, 0x83) AM_READWRITE8(nmictrl_r, nmictrl_w, 0xff000000)

	AM_RANGE(0x88, 0x8b) AM_READWRITE(timer_prescaler_r, timer_prescaler_w)
	AM_RANGE(0x8c, 0x8f) AM_READWRITE(timer0_r, timer0_w)
	AM_RANGE(0x90, 0x93) AM_READWRITE(timer1_r, timer1_w)

	AM_RANGE(0xa8, 0xab) AM_READWRITE(timer3_r, timer3_w)
ADDRESS_MAP_END

// InterPro IOGA
const device_type INTERPRO_IOGA = &device_creator<interpro_ioga_device>;

interpro_ioga_device::interpro_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_IOGA, "InterPro IOGA", tag, owner, clock, "ioga", __FILE__),
	m_out_nmi_func(*this),
	m_out_int_func(*this),
	m_dma_r_func{ { *this }, { *this }, { *this }, { *this } },
	m_dma_w_func{ { *this }, { *this }, { *this }, { *this } }
	{
}

void interpro_ioga_device::device_start()
{
	// resolve callbacks
	m_out_nmi_func.resolve();
	m_out_int_func.resolve();

	for (auto & r : m_dma_r_func)
		r.resolve_safe(0xff);

	for (auto & w : m_dma_w_func)
		w.resolve();

	m_cpu = machine().device<cpu_device>("cpu");

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
	m_irq_active = false;

	m_state_drq = 0;

	// configure timer 0 at 60Hz
	//m_timer_reg[0] = 0;
	//m_timer[0]->adjust(attotime::from_hz(60), IOGA_TIMER_0, attotime::from_hz(60));
}

/******************************************************************************
  Timers
******************************************************************************/
void interpro_ioga_device::write_timer(int timer, uint32_t value, device_timer_id id)
{
	switch (id)
	{
	case IOGA_TIMER_1:
		// disable the timer
		m_timer[timer]->enable(false);

		// store the value
		m_timer_reg[timer] = value & IOGA_TIMER1_VMASK;

		// start the timer if necessary
		if (value & IOGA_TIMER1_START)
			m_timer[timer]->adjust(attotime::zero, id, attotime::from_usec(m_prescaler));
		break;

	case IOGA_TIMER_3:
		// write the value without the top two bits to the register
		m_timer_reg[timer] = value & IOGA_TIMER3_VMASK;

		// start the timer if necessary
		if (value & IOGA_TIMER3_START)
			m_timer[timer]->adjust(attotime::zero, id, attotime::from_hz(IOGA_TIMER3_CLOCK));
		else
			m_timer[timer]->enable(false);
		break;

	default:
		// save the value
		m_timer_reg[timer] = value;

		// timer_set(attotime::from_usec(500), id);

		logerror("timer %d set to 0x%x (%d)\n", timer, m_timer_reg[timer], m_timer_reg[timer]);
		break;
	}
}

void interpro_ioga_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_TIMER("interpro_ioga_device::device_timer(id = %d)\n", id);

	switch (id)
	{
	case IOGA_TIMER_0:
		m_timer_reg[0]++;
		set_irq_line(IOGA_TIMER0_IRQ, ASSERT_LINE);
		break;

	case IOGA_TIMER_1:
		m_timer_reg[1]--;

		// check if timer has expired
		if (m_timer_reg[1] == 0)
		{
			// disable timer
			timer.enable(false);

			// set expired flag
			m_timer_reg[1] |= IOGA_TIMER1_EXPIRED;

			// throw an interrupt
			set_irq_line(IOGA_TIMER1_IRQ, ASSERT_LINE);
		}
		break;

	case IOGA_TIMER_3:
		m_timer_reg[3]--;
		if (m_timer_reg[3] == 0)
		{
			// disable timer
			timer.enable(false);

			// set expired flag
			m_timer_reg[3] |= IOGA_TIMER3_EXPIRED;

			// throw an interrupt
			set_irq_line(IOGA_TIMER3_IRQ, ASSERT_LINE);
		}
		break;

	case IOGA_TIMER_DMA:
		// transfer data from fdc to memory
		// TODO: vice-versa
		// TODO: get the dma transfer address and count
		// TODO: implement multiple dma channels
	{
		address_space &space = m_cpu->space(AS_PROGRAM);

		space.write_byte(m_fdc_dma[0]++, m_dma_r_func[IOGA_DMA_CHANNEL_FLOPPY]());
		if (--m_fdc_dma[2])
			m_dma_timer->adjust(attotime::from_usec(10));
		else
			m_dma_timer->adjust(attotime::never);
	}
	break;
	}
}

/*
IOGA
00: ethernet remap 003e0480 // ET_82586_BASE_ADDR or IOGA_ETH_REMAP
04 : ethernet map page 00fff4b0 // ET_82586_CNTL_REG  or IOGA_ETH_MAPPG
08 : ethernet control 000004b2 // IOGA_ETH_CTL
0C : plotter real address 00000fff

10 : plotter virtual address fffffffc
14 : plotter transfer count 003fffff
18 : plotter control ec000001
1C : plotter end - of - scanline counter ffffffff

20 : SCSI real address 00000000
24 : SCSI virtual address 007e96b8
28 : SCSI transfer count
2C : SCSI control

30 : floppy real address
34 : floppy virtual address
38 : floppy transfer count
3C : floppy control

40 : serial address 0 (003ba298)
44 : serial control 0 (01000000)
48 : serial address 1 (ffffffff)
4C : serial control 1 (01200000)

50 : serial address 2 (ffffffff)
54 : serial control 2 (01200000)

-- 16 bit
5A : SIB control(00ff)
5C : internal int  3 (timer 2) 00ff        irq 0
5E : internal int  4 (timer 3) 00ff        irq 1

60 : external int  0 (SCSI)0a20            irq 2
62 : external int  1 (floppy)0621          irq 3
64 : external int  2 (plotter)1622         irq 4
66 : external int  3 (SRX / CBUS 0) 0a02   irq 5
68 : external int  4 (SRX / CBUS 1) 0e24   irq 6
6A : external int  5 (SRX / CBUS 2) 0e25   irq 7
6C : external int  6 (VB)0c26              irq 8
6E : external int  7 0cff                  irq 9
70 : external int  8 (CBUS 3) 0cff         irq 10
72 : external int  9 (clock / calendar) 0e29  irq 11
74 : external int 10 (clock/SGA) 04fe         irq 12

76 : internal int  0 (mouse)0010              irq 13
78 : internal int  1 (timer 0) 0011 - 60Hz    irq 14
7A : internal int  2 (timer 1) 0212           irq 15
7C : internal int  5 (serial DMA) 0e13        irq 16
7E : external int 11 (serial) 0a01            irq 17
80 : external int 12 (Ethernet)162c			  irq 18 // IOGA_EXTINT12

-- 8 bit
82 : soft int 00
83 : NMI control 1b

-- 32 bit
84 : mouse status 00070000
88 : timer prescaler 05aa06da
8C : timer 0 00000022
90 : timer 1 00010005
94 : error address 7f100000
98 : error cycle type 00001911 // short?

-- 16 bit
9C: arbiter control 000a // IOGA_ARBCTL
	#define ETHC_BR_ENA	(1<<0)
	#define SCSI_BR_ENA	(1<<1)
	#define PLT_BR_ENA	(1<<2)
	#define FLP_BR_ENA	(1<<3)
	#define SER0_BR_ENA	(1<<4)
	#define SER1_BR_ENA	(1<<5)
	#define SER2_BR_ENA	(1<<6)
	#define ETHB_BR_ENA	(1<<7)
	#define ETHA_BR_ENA	(1<<8)

9E : I / O base address

-- 32 bit
A0 : timer 2 count ccc748ec
A4 : timer 2 value ffffffff
A8 : timer 3 bfffffff // timer 3 count
AC : bus timeout 445a445c

-- 16 bit
B0 : soft int 8 00ff
B2 : soft int 9 00ff
B4 : soft int 10 00ff
B6 : soft int 11 00ff
B8 : soft int 12 00ff
BA : soft int 13 00ff
BC : soft int 14 00ff
BE : soft int 15 00ff

-- 32 bit
C0 : ethernet address A 403bc002 // IOGA_ETHADDR_A
C4 : ethernet address B 403a68a0 // IOGA_ETHADDR_B
C8 : ethernet address C 4039f088 // IOGA_ETHADDR_C

		 (62) = 0x0421			// set floppy interrupts?
	(3C) &= 0xfeff ffff		// turn off and on again
	(3C) |= 0x0100 0000
	(9C) |= 0x0008
	(62) = 0x0621

	during rom boot, all interrupt vectors point at 7f10249e

	int 16 = prioritized interrupt 16, level 0, number 0, mouse interface
	17 timer 0
*/

/******************************************************************************
 Interrupts
******************************************************************************/

static const uint16_t irq_enable_mask[19] =
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

void interpro_ioga_device::set_irq_line(int irq, int state)
{
	LOG_INTERRUPT("set_irq_line(%d, %d)\n", irq, state);
	switch (state)
	{
	case ASSERT_LINE:
		if (m_vectors[irq] & irq_enable_mask[irq])
		{
			// set interrupt pending bit
			m_vectors[irq] |= IOGA_INTERRUPT_PENDING;

			// update irq line state
			update_irq(state);
		}
		else
			LOG_INTERRUPT("received disabled interrupt irq %d vector 0x%04x\n", irq, m_vectors[irq]);
		break;

	case CLEAR_LINE:
		// clear interrupt pending bit
		m_vectors[irq] &= ~IOGA_INTERRUPT_PENDING;

		// update irq line state
		update_irq(state);
		break;
	}
}

IRQ_CALLBACK_MEMBER(interpro_ioga_device::inta_cb)
{
	switch (irqline)
	{
	case -1:
		// return vector for current interrupt without clearing irq line
		return m_vectors[m_irq_current] & 0xff;

	case INPUT_LINE_IRQ0:
		// FIXME: clear pending bit - can't rely on device callbacks
		m_vectors[m_irq_current] &= ~IOGA_INTERRUPT_PENDING;

		// clear irq line
		update_irq(CLEAR_LINE);

		// return interrupt vector
		return m_vectors[m_irq_current] & 0xff;

	case INPUT_LINE_NMI:
		// clear nmi line
		m_out_nmi_func(CLEAR_LINE);

		return 0;

	default:
		return 0;
	}
}

void interpro_ioga_device::update_irq(int state)
{
	switch (state)
	{
	case CLEAR_LINE:
		if (m_irq_active)
		{
			// the cpu has acknowledged the active interrupt, deassert the irq line
			m_irq_active = false;
			m_out_int_func(CLEAR_LINE);

		}
		// fall through to handle any pending interrupts

	case ASSERT_LINE:
		// if an irq is currently active, don't do anything
		if (!m_irq_active)
		{
			// check for any pending interrupts
			for (int irq = 0; irq < 19; irq++)
			{
				if (m_vectors[irq] & IOGA_INTERRUPT_PENDING)
				{
					m_irq_active = true;
					m_irq_current = irq;

					m_out_int_func(ASSERT_LINE);
					return;
				}
			}
		}
		break;
	}
}

READ16_MEMBER(interpro_ioga_device::icr_r)
{
	return m_vectors[offset] & ~IOGA_INTERRUPT_FORCED;
}

WRITE16_MEMBER(interpro_ioga_device::icr_w)
{
	LOG_INTERRUPT("interrupt vector %d set to 0x%04x at pc 0x%08x\n", offset, data, space.device().safe_pc());

	// FIXME: now that the interrupt handling only depends on IOGA_INTERRUPT_PENDING, we might be able
	// to avoid this hack
	if (data & IOGA_INTERRUPT_PENDING)
		m_vectors[offset] = (data | IOGA_INTERRUPT_FORCED) & ~IOGA_INTERRUPT_PENDING;
	else if (m_vectors[offset] & IOGA_INTERRUPT_FORCED)
	{
		m_vectors[offset] = data;

		set_irq_line(offset, ASSERT_LINE);
	}
	else
		m_vectors[offset] = data;
}

WRITE8_MEMBER(interpro_ioga_device::softint_w)
{
	logerror("soft interrupt write 0x%02x\n", data); 
	
	// FIXME: appears that forced interrupts work the same way, by
	// writing bits in this register on and then off again. Don't
	// know how the actual interrupt should be directed.
	m_softint = data;
}

/******************************************************************************
 DMA
******************************************************************************/
WRITE_LINE_MEMBER(interpro_ioga_device::drq)
{
	if (state)
	{
		// TODO: check if dma is enabled
		m_dma_timer->adjust(attotime::from_usec(10));
	}

	m_state_drq = state;
}