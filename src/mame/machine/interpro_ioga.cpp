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

// InterPro IOGA
const device_type INTERPRO_IOGA = &device_creator<interpro_ioga_device>;

interpro_ioga_device::interpro_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_IOGA, "InterPro IOGA", tag, owner, clock, "ioga", __FILE__),
	m_out_int_func(*this),
	m_irq_lines(0)
{
}

void interpro_ioga_device::device_start()
{
	// resolve callbacks
	m_out_int_func.resolve();

	m_cpu = machine().device<cpu_device>("cpu");
	m_fdc = machine().device<upd765_family_device>("fdc");

	// allocate timer for DMA controller
	m_dma_timer = timer_alloc(IOGA_TIMER_DMA);
	m_dma_timer->adjust(attotime::never);
}

void interpro_ioga_device::device_reset()
{
	m_irq_lines = 0;
	m_interrupt = 0;
	m_state_drq = 0;
}

void interpro_ioga_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_TIMER("interpro_ioga_device::device_timer(id = %d)\n", id);

	switch (id)
	{
	case IOGA_TIMER_0:
		m_timer[0] = 0x80000000;
		set_irq_line(14, 1);
		break;
	case IOGA_TIMER_1:
		m_timer[1] = 0x80000000;
		set_irq_line(15, 1);
		break;
	case IOGA_TIMER_3:
		m_timer[3] = 0x80000000;
		set_irq_line(1, 1);
		break;

	case IOGA_TIMER_DMA:
		// transfer data from fdc to memory
		// TODO: vice-versa
		// TODO: get the dma transfer address and count
		// TODO: implement multiple dma channels
		{
			address_space &space = m_cpu->space(AS_PROGRAM);

			space.write_byte(m_fdc_dma[0]++, m_fdc->dma_r());
			if (--m_fdc_dma[2])
				m_dma_timer->adjust(attotime::from_usec(10));
			else
				m_dma_timer->adjust(attotime::never);
	}
		break;
	}
}

DEVICE_ADDRESS_MAP_START(map, 32, interpro_ioga_device)

	AM_RANGE(0x30, 0x3f) AM_READWRITE(fdc_dma_r, fdc_dma_w)

	AM_RANGE(0x5C, 0x81) AM_READWRITE16(icr_r, icr_w, 0xffffffff)

	AM_RANGE(0x8C, 0x8F) AM_READWRITE(timer0_r, timer0_w)
	AM_RANGE(0x90, 0x93) AM_READWRITE(timer1_r, timer1_w)

	AM_RANGE(0xA8, 0xAB) AM_READWRITE(timer3_r, timer3_w)
ADDRESS_MAP_END


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
5C : internal int  3 (timer 2) 00ff
5E : internal int  4 (timer 3) 00ff

60 : external int  0 (SCSI)0a20
62 : external int  1 (floppy)0621
64 : external int  2 (plotter)1622
66 : external int  3 (SRX / CBUS 0) 0a02
68 : external int  4 (SRX / CBUS 1) 0e24
6A : external int  5 (SRX / CBUS 2) 0e25
6C : external int  6 (VB)0c26
6E : external int  7 0cff
70 : external int  8 (CBUS 3) 0cff
72 : external int  9 (clock / calendar) 0e29
74 : external int 10 (clock/SGA) 04fe

76 : internal int  0 (mouse)0010
78 : internal int  1 (timer 0) 0011 - 60Hz
7A : internal int  2 (timer 1) 0212
7C : internal int  5 (serial DMA) 0e13
7E : external int 11 (serial) 0a01
80 : external int 12 (Ethernet)162c // IOGA_EXTINT12

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

void interpro_ioga_device::set_irq_line(int irq, int state)
{
	uint32_t mask = (1 << irq);
#define E_INTRC_INTPEND		0x0100
#define	E_INTRC_EXT_IE		0x0200
#define E_INTRC_EDGE		0x0400
#define E_INTRC_NEGPOL		0x0800
#define	E_INTRC_INT_IE		0x1000

	if (m_vectors[irq] & (E_INTRC_EXT_IE | E_INTRC_INT_IE))
	{
		if (state)
		{
			LOG_INTERRUPT("interpro_ioga_device::set_irq_line(%d, %d)\n", irq, state);

			m_irq_lines |= mask;
			m_interrupt = irq;
			m_out_int_func(1);
		}
		else
		{
			m_irq_lines &= ~mask;
			m_out_int_func(0);
		}
	}
	else
		LOG_INTERRUPT("ignoring irq %d vector 0x%04x\n", irq, m_vectors[irq]);
}

IRQ_CALLBACK_MEMBER(interpro_ioga_device::inta_cb)
{
	return m_vectors[m_interrupt];
}

WRITE_LINE_MEMBER(interpro_ioga_device::drq)
{
	if (state)
	{
		// TODO: check if dma is enabled
		m_dma_timer->adjust(attotime::from_usec(10));
	}

	m_state_drq = state;
}


READ32_MEMBER(interpro_ioga_device::read)
{
	switch (offset)
	{
#if 0
	case 0x3C:
		// 3C: floppy control
		break;

	case 0x9C:
		// 9C: arbiter control 000a
		break;
#endif

	default:
		LOG_IOGA("ioga read from offset = %08x, mask = %08x, pc = %08x\n", offset, mem_mask, space.device().safe_pc());
		return 0xffffffff;
	}
}

void interpro_ioga_device::set_timer(int timer, uint32_t value, device_timer_id id)
{
	m_timer[timer] = value;
	if (value & 0x40000000)
		//timer_set(attotime::from_usec(value & 0x3fffff), id);
		timer_set(attotime::from_usec(500), id);

}

WRITE32_MEMBER(interpro_ioga_device::write)
{
	switch (offset)
	{
	default:
		LOG_IOGA("ioga write to offset = 0x%08x, mask = 0x%08x) = 0x%08x, pc = %08x\n", offset, mem_mask, data, space.device().safe_pc());
		break;
	}
}

READ16_MEMBER(interpro_ioga_device::icr_r)
{
	return m_vectors[offset];
}

WRITE16_MEMBER(interpro_ioga_device::icr_w)
{
	LOG_INTERRUPT("interrupt vector %d set to 0x%04x at pc 0x%08x\n", offset, data, space.device().safe_pc());

	m_vectors[offset] = data;
}