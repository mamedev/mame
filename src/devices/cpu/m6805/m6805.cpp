// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/*** m6805: Portable 6805 emulator ******************************************

    m6805.c (Also supports hd68705 and hd63705 variants)

    References:

        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    uint16_t must be 16 bit unsigned int
                            uint8_t must be 8 bit unsigned int
                            uint32_t must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

  Additional Notes:

  K.Wilkins 18/03/99 - Added 63705 functonality and modified all CPU functions
                       necessary to support:
                           Variable width address bus
                           Different stack pointer
                           Alternate boot vectors
                           Alternate interrups vectors


*****************************************************************************/

#include "emu.h"
#include "m6805.h"
#include "m6805defs.h"

#include "debugger.h"

#include <algorithm>

#define IRQ_LEVEL_DETECT 0

#define OP(name)        (&m6805_base_device::name)
#define OP_T(name)      (&m6805_base_device::name<true>)
#define OP_F(name)      (&m6805_base_device::name<false>)
#define OP_IM(name)     (&m6805_base_device::name<addr_mode::IM>)
#define OP_DI(name)     (&m6805_base_device::name<addr_mode::DI>)
#define OP_EX(name)     (&m6805_base_device::name<addr_mode::EX>)
#define OP_IX(name)     (&m6805_base_device::name<addr_mode::IX>)
#define OP_IX1(name)    (&m6805_base_device::name<addr_mode::IX1>)
#define OP_IX2(name)    (&m6805_base_device::name<addr_mode::IX2>)
const m6805_base_device::op_handler_func m6805_base_device::m_ophndlr[256] =
{
	/*      0/8          1/9          2/A          3/B          4/C          5/D          6/E          7/F */
	/* 0 */ OP(brset<0>),OP(brclr<0>),OP(brset<1>),OP(brclr<1>),OP(brset<2>),OP(brclr<2>),OP(brset<3>),OP(brclr<3>),
			OP(brset<4>),OP(brclr<4>),OP(brset<5>),OP(brclr<5>),OP(brset<6>),OP(brclr<6>),OP(brset<7>),OP(brclr<7>),
	/* 1 */ OP(bset<0>), OP(bclr<0>), OP(bset<1>), OP(bclr<1>), OP(bset<2>), OP(bclr<2>), OP(bset<3>), OP(bclr<3>),
			OP(bset<4>), OP(bclr<4>), OP(bset<5>), OP(bclr<5>), OP(bset<6>), OP(bclr<6>), OP(bset<7>), OP(bclr<7>),
	/* 2 */ OP_T(bra),   OP_F(bra),   OP_T(bhi),   OP_F(bhi),   OP_T(bcc),   OP_F(bcc),   OP_T(bne),   OP_F(bne),
			OP_T(bhcc),  OP_F(bhcc),  OP_T(bpl),   OP_F(bpl),   OP_T(bmc),   OP_F(bmc),   OP(bil),     OP(bih),
	/* 3 */ OP_DI(neg),  OP(illegal), OP(illegal), OP_DI(com),  OP_DI(lsr),  OP(illegal), OP_DI(ror),  OP_DI(asr),
			OP_DI(lsl),  OP_DI(rol),  OP_DI(dec),  OP(illegal), OP_DI(inc),  OP_DI(tst),  OP(illegal), OP_DI(clr),
	/* 4 */ OP(nega),    OP(illegal), OP(illegal), OP(coma),    OP(lsra),    OP(illegal), OP(rora),    OP(asra),
			OP(lsla),    OP(rola),    OP(deca),    OP(illegal), OP(inca),    OP(tsta),    OP(illegal), OP(clra),
	/* 5 */ OP(negx),    OP(illegal), OP(illegal), OP(comx),    OP(lsrx),    OP(illegal), OP(rorx),    OP(asrx),
			OP(lslx),    OP(rolx),    OP(decx),    OP(illegal), OP(incx),    OP(tstx),    OP(illegal), OP(clrx),
	/* 6 */ OP_IX1(neg), OP(illegal), OP(illegal), OP_IX1(com), OP_IX1(lsr), OP(illegal), OP_IX1(ror), OP_IX1(asr),
			OP_IX1(lsl), OP_IX1(rol), OP_IX1(dec), OP(illegal), OP_IX1(inc), OP_IX1(tst), OP(illegal), OP_IX1(clr),
	/* 7 */ OP_IX(neg),  OP(illegal), OP(illegal), OP_IX(com),  OP_IX(lsr),  OP(illegal), OP_IX(ror),  OP_IX(asr),
			OP_IX(lsl),  OP_IX(rol),  OP_IX(dec),  OP(illegal), OP_IX(inc),  OP_IX(tst),  OP(illegal), OP_IX(clr),
	/* 8 */ OP(rti),     OP(rts),     OP(illegal), OP(swi),     OP(illegal), OP(illegal), OP(illegal), OP(illegal),
			OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal),
	/* 9 */ OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(tax), 
			OP(clc),     OP(sec),     OP(cli),     OP(sei),     OP(rsp),     OP(nop),     OP(illegal), OP(txa),
	/* A */ OP_IM(suba), OP_IM(cmpa), OP_IM(sbca), OP_IM(cpx),  OP_IM(anda), OP_IM(bita), OP_IM(lda),  OP(illegal),
			OP_IM(eora), OP_IM(adca), OP_IM(ora),  OP_IM(adda), OP(illegal), OP(bsr),     OP_IM(ldx),  OP(illegal),
	/* B */ OP_DI(suba), OP_DI(cmpa), OP_DI(sbca), OP_DI(cpx),  OP_DI(anda), OP_DI(bita), OP_DI(lda),  OP_DI(sta),
			OP_DI(eora), OP_DI(adca), OP_DI(ora),  OP_DI(adda), OP_DI(jmp),  OP_DI(jsr),  OP_DI(ldx),  OP_DI(stx),
	/* C */ OP_EX(suba), OP_EX(cmpa), OP_EX(sbca), OP_EX(cpx),  OP_EX(anda), OP_EX(bita), OP_EX(lda),  OP_EX(sta),
			OP_EX(eora), OP_EX(adca), OP_EX(ora),  OP_EX(adda), OP_EX(jmp),  OP_EX(jsr),  OP_EX(ldx),  OP_EX(stx),
	/* D */ OP_IX2(suba),OP_IX2(cmpa),OP_IX2(sbca),OP_IX2(cpx), OP_IX2(anda),OP_IX2(bita),OP_IX2(lda), OP_IX2(sta),
			OP_IX2(eora),OP_IX2(adca),OP_IX2(ora), OP_IX2(adda),OP_IX2(jmp), OP_IX2(jsr), OP_IX2(ldx), OP_IX2(stx),
	/* E */ OP_IX1(suba),OP_IX1(cmpa),OP_IX1(sbca),OP_IX1(cpx), OP_IX1(anda),OP_IX1(bita),OP_IX1(lda), OP_IX1(sta),
			OP_IX1(eora),OP_IX1(adca),OP_IX1(ora), OP_IX1(adda),OP_IX1(jmp), OP_IX1(jsr), OP_IX1(ldx), OP_IX1(stx),
	/* F */ OP_IX(suba), OP_IX(cmpa), OP_IX(sbca), OP_IX(cpx),  OP_IX(anda), OP_IX(bita), OP_IX(lda),  OP_IX(sta),
			OP_IX(eora), OP_IX(adca), OP_IX(ora),  OP_IX(adda), OP_IX(jmp),  OP_IX(jsr),  OP_IX(ldx),  OP_IX(stx)
};

const uint8_t m6805_base_device::m_flags8i[256] =   /* increment */
{
	/*       0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*0*/ 0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*1*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*2*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*3*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*4*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*5*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*6*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*7*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*8*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*9*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*A*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*B*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*C*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*D*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*E*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*F*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
};

const uint8_t m6805_base_device::m_flags8d[256] = /* decrement */
{
	/*       0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/*0*/ 0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*1*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*2*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*3*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*4*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*5*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*6*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*7*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*8*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*9*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*A*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*B*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*C*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*D*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*E*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	/*F*/ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
};

/* what they say it is ... */
const uint8_t m6805_base_device::m_cycles1[] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	/*1*/  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 0, 6,
	/*4*/  4, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4,
	/*5*/  4, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4,
	/*6*/  7, 0, 0, 7, 7, 0, 7, 7, 7, 7, 7, 0, 7, 7, 0, 7,
	/*7*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 0, 6,
	/*8*/  9, 6, 0,11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*9*/  0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 2,
	/*A*/  2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 0, 8, 2, 0,
	/*B*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 4, 5,
	/*C*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 8, 5, 6,
	/*D*/  6, 6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 5, 9, 6, 7,
	/*E*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 8, 5, 6,
	/*F*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 4, 5
};


void m6805_base_device::rd_s_handler_b(uint8_t *b)
{
	SP_INC;
	*b = RM( S );
}

void m6805_base_device::rd_s_handler_w(PAIR *p)
{
	CLEAR_PAIR(p);
	SP_INC;
	p->b.h = RM( S );
	SP_INC;
	p->b.l = RM( S );
}

void m6805_base_device::wr_s_handler_b(uint8_t *b)
{
	WM( S, *b );
	SP_DEC;
}

void m6805_base_device::wr_s_handler_w(PAIR *p)
{
	WM( S, p->b.l );
	SP_DEC;
	WM( S, p->b.h );
	SP_DEC;
}

void m6805_base_device::RM16(uint32_t addr, PAIR *p)
{
	CLEAR_PAIR(p);
	p->b.h = RM(addr);
	++addr;
//  if( ++addr > AMASK ) addr = 0;
	p->b.l = RM(addr);
}

void m6805_base_device::interrupt_vector()
{
	RM16(0xffff - 5, &m_pc);
}

void m68hc05eg_device::interrupt_vector()
{
	if ((m_pending_interrupts & (1 << M68HC05EG_INT_IRQ)) != 0)
	{
		m_pending_interrupts &= ~(1 << M68HC05EG_INT_IRQ);
		RM16(0x1ffa, &m_pc);
	}
	else if((m_pending_interrupts & (1 << M68HC05EG_INT_TIMER)) != 0)
	{
		m_pending_interrupts &= ~(1 << M68HC05EG_INT_TIMER);
		RM16(0x1ff8, &m_pc);
	}
	else if((m_pending_interrupts & (1 << M68HC05EG_INT_CPI)) != 0)
	{
		m_pending_interrupts &= ~(1 << M68HC05EG_INT_CPI);
		RM16(0x1ff6, &m_pc);
	}
}

void hd63705_device::interrupt_vector()
{
	/* Need to add emulation of other interrupt sources here KW-2/4/99 */
	/* This is just a quick patch for Namco System 2 operation         */

	if ((m_pending_interrupts & (1 << HD63705_INT_IRQ1)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_IRQ1);
		RM16(0x1ff8, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_IRQ2)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_IRQ2);
		RM16(0x1fec, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_ADCONV)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_ADCONV);
		RM16(0x1fea, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_TIMER1)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_TIMER1);
		RM16(0x1ff6, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_TIMER2)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_TIMER2);
		RM16(0x1ff4, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_TIMER3)) != 0)
	{
		m_pending_interrupts &= ~(1<<HD63705_INT_TIMER3);
		RM16(0x1ff2, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_PCI)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_PCI);
		RM16(0x1ff0, &m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_SCI)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_SCI);
		RM16(0x1fee, &m_pc);
	}
}

/* Generate interrupts */
void m6805_base_device::interrupt()
{
	/* the 6805 latches interrupt requests internally, so we don't clear */
	/* pending_interrupts until the interrupt is taken, no matter what the */
	/* external IRQ pin does. */

	if ((m_pending_interrupts & (1 << HD63705_INT_NMI)) != 0)
	{
		PUSHWORD(m_pc);
		PUSHBYTE(m_x);
		PUSHBYTE(m_a);
		PUSHBYTE(m_cc);
		SEI;
		/* no vectors supported, just do the callback to clear irq_state if needed */
		standard_irq_callback(0);

		RM16(0x1ffc, &m_pc);
		m_pending_interrupts &= ~(1 << HD63705_INT_NMI);

		m_icount -= 11;
		burn_cycles(11);
	}
	else if((m_pending_interrupts & ((1 << M6805_IRQ_LINE) | HD63705_INT_MASK)) != 0)
	{
		if ((CC & IFLAG) == 0)
		{
			/* standard IRQ */
			PUSHWORD(m_pc);
			PUSHBYTE(m_x);
			PUSHBYTE(m_a);
			PUSHBYTE(m_cc);
			SEI;
			/* no vectors supported, just do the callback to clear irq_state if needed */
			standard_irq_callback(0);

			interrupt_vector();

			m_pending_interrupts &= ~(1 << M6805_IRQ_LINE);
		}
		m_icount -= 11;
		burn_cycles(11);
	}
}


//-------------------------------------------------
//  m6809_base_device - constructor
//-------------------------------------------------

m6805_base_device::m6805_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, const char *name, uint32_t addr_width, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 8, addr_width)
{
}

m6805_base_device::m6805_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, const char *name, uint32_t addr_width, address_map_delegate internal_map, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 8, addr_width, 0, internal_map)
{
}



void m6805_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// set our instruction counter
	m_icountptr = &m_icount;

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc.w.l).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc.w.l).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_cc).callimport().callexport().formatstr("%8s").noshow();
	state_add(M6805_A,         "A",         m_a).mask(0xff);
	state_add(M6805_PC,        "PC",        m_pc.w.l).mask(0xffff);
	state_add(M6805_S,         "S",         m_s.w.l).mask(0xff);
	state_add(M6805_X,         "X",         m_x).mask(0xff);
	state_add(M6805_CC,        "CC",        m_cc).mask(0xff);

	// register for savestates
	save_item(NAME(EA));
	save_item(NAME(SP_MASK));
	save_item(NAME(SP_LOW));
	save_item(NAME(A));
	save_item(NAME(PC));
	save_item(NAME(S));
	save_item(NAME(X));
	save_item(NAME(CC));
	save_item(NAME(m_pending_interrupts));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_nmi_state));

	std::fill(std::begin(m_irq_state), std::end(m_irq_state), 0);
}



void m6805_base_device::device_reset()
{
	m_ea.w.l = 0;
	m_sp_mask = 0x07f;
	m_sp_low = 0x060;
	m_pc.w.l = 0;
	m_s.w.l = SP_MASK;
	m_a = 0;
	m_x = 0;
	m_cc = 0;
	m_pending_interrupts = 0;

	m_nmi_state = 0;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	/* IRQ disabled */
	SEI;

	RM16(0xfffe, &m_pc);
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *m6805_base_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	return nullptr;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void m6805_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				(m_cc & 0x80) ? '?' : '.',
				(m_cc & 0x40) ? '?' : '.',
				(m_cc & 0x20) ? '?' : '.',
				(m_cc & 0x10) ? 'H' : '.',
				(m_cc & 0x08) ? 'I' : '.',
				(m_cc & 0x04) ? 'N' : '.',
				(m_cc & 0x02) ? 'Z' : '.',
				(m_cc & 0x01) ? 'C' : '.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

uint32_t m6805_base_device::disasm_min_opcode_bytes() const
{
	return 1;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

uint32_t m6805_base_device::disasm_max_opcode_bytes() const
{
	return 3;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t m6805_base_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( m6805 );
	return CPU_DISASSEMBLE_NAME(m6805)(this, stream, pc, oprom, opram, options);
}


void m6805_device::execute_set_input(int inputnum, int state)
{
	/* Basic 6805 only has one IRQ line */
	/* See HD63705 specific version     */
	if (m_irq_state[0] != state)
	{
		m_irq_state[0] = state;

		if (state != CLEAR_LINE)
		{
			m_pending_interrupts |= 1 << M6805_IRQ_LINE;
		}
	}
}

#include "6805ops.hxx"

//-------------------------------------------------
//  execute_clocks_to_cycles - convert the raw
//  clock into cycles per second
//-------------------------------------------------

uint64_t m6805_base_device::execute_clocks_to_cycles(uint64_t clocks) const
{
	return (clocks + 3) / 4;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert a cycle
//  count back to raw clocks
//-------------------------------------------------

uint64_t m6805_base_device::execute_cycles_to_clocks(uint64_t cycles) const
{
	return cycles * 4;
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t m6805_base_device::execute_min_cycles() const
{
	return 2;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t m6805_base_device::execute_max_cycles() const
{
	return 10;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

uint32_t m6805_base_device::execute_input_lines() const
{
	return 9;
}


/* execute instructions on this CPU until icount expires */
void m6805_base_device::execute_run()
{
	S = SP_ADJUST( S );     /* Taken from CPU_SET_CONTEXT when pointer'afying */

	do
	{
		if (m_pending_interrupts != 0)
		{
			interrupt();
		}

		debugger_instruction_hook(this, PC);

		u8 const ireg = M_RDOP(PC++);

		(this->*m_ophndlr[ireg])();
		m_icount -= m_cycles1[ireg];
		burn_cycles(m_cycles1[ireg]);
	}
	while (m_icount > 0);
}

/****************************************************************************
 * M68HC05EG section
 ****************************************************************************/
void m68hc05eg_device::device_reset()
{
	m6805_base_device::device_reset();

	m_sp_mask = 0xff;
	m_sp_low = 0xc0;

	RM16(0x1ffe, &m_pc);
}

void m68hc05eg_device::execute_set_input(int inputnum, int state)
{
	if (m_irq_state[inputnum] != state)
	{
		m_irq_state[inputnum] = state;

		if (state != CLEAR_LINE)
		{
			m_pending_interrupts |= 1 << inputnum;
		}
	}
}


/****************************************************************************
 * HD63705 section
 ****************************************************************************/
void hd63705_device::device_reset()
{
	m6805_base_device::device_reset();

	m_sp_mask = 0x17f;
	m_sp_low = 0x100;
	m_s.w.l = SP_MASK;

	RM16(0x1ffe, &m_pc);
}

void hd63705_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == INPUT_LINE_NMI)
	{
		if (m_nmi_state != state)
		{
			m_nmi_state = state;

			if (state != CLEAR_LINE)
			{
				m_pending_interrupts |= 1 << HD63705_INT_NMI;
			}
		}
	}
	else if (inputnum <= HD63705_INT_ADCONV)
	{
		if (m_irq_state[inputnum] != state)
		{
			m_irq_state[inputnum] = state;

			if (state != CLEAR_LINE)
			{
				m_pending_interrupts |= 1 << inputnum;
			}
		}
	}
}

const device_type M6805 = &device_creator<m6805_device>;
const device_type M68HC05EG = &device_creator<m68hc05eg_device>;
const device_type HD63705 = &device_creator<hd63705_device>;
