/*** m6809: Portable 6809 emulator ******************************************

    Copyright John Butler

    References:

        6809 Simulator V09, By L.C. Benschop, Eidnhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    UINT16 must be 16 bit unsigned int
                            UINT8 must be 8 bit unsigned int
                            UINT32 must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

    History:
991026 HJB:
    Fixed missing calls to cpu_changepc() for the TFR and EXG ocpodes.
    Replaced m6809_slapstic checks by a macro (CHANGE_PC). ESB still
    needs the tweaks.

991024 HJB:
    Tried to improve speed: Using bit7 of cycles1/2 as flag for multi
    byte opcodes is gone, those opcodes now call fetch_effective_address().
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990312 HJB:
    Added bugfixes according to Aaron's findings.
    Reset only sets CC_II and CC_IF, DP to zero and PC from reset vector.
990311 HJB:
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
    Modified the read/write stack handlers to push LSB first then MSB
    and pull MSB first then LSB.

990228 HJB:
    Changed the interrupt handling again. Now interrupts are taken
    either right at the moment the lines are asserted or whenever
    an interrupt is enabled and the corresponding line is still
    asserted. That way the pending_interrupts checks are not
    needed anymore. However, the CWAI and SYNC flags still need
    some flags, so I changed the name to 'int_state'.
    This core also has the code for the old interrupt system removed.

990225 HJB:
    Cleaned up the code here and there, added some comments.
    Slightly changed the SAR opcodes (similiar to other CPU cores).
    Added symbolic names for the flag bits.
    Changed the way CWAI/Interrupt() handle CPU state saving.
    A new flag M6809_STATE in pending_interrupts is used to determine
    if a state save is needed on interrupt entry or already done by CWAI.
    Added M6809_IRQ_LINE and M6809_FIRQ_LINE defines to m6809.h
    Moved the internal interrupt_pending flags from m6809.h to m6809.c
    Changed CWAI cycles2[0x3c] to be 2 (plus all or at least 19 if
    CWAI actually pushes the entire state).
    Implemented undocumented TFR/EXG for undefined source and mixed 8/16
    bit transfers (they should transfer/exchange the constant $ff).
    Removed unused jmp/jsr _slap functions from 6809ops.c,
    m6809_slapstick check moved into the opcode functions.

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "m6809.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* flag bits in the cc register */
#define CC_C	0x01        /* Carry */
#define CC_V	0x02        /* Overflow */
#define CC_Z    0x04        /* Zero */
#define CC_N    0x08        /* Negative */
#define CC_II   0x10        /* Inhibit IRQ */
#define CC_H    0x20        /* Half (auxiliary) carry */
#define CC_IF   0x40        /* Inhibit FIRQ */
#define CC_E    0x80        /* entire state pushed */

#define pPPC    m_ppc
#define pPC 	m_pc
#define pU		m_u
#define pS		m_s
#define pX		m_x
#define pY		m_y
#define pD		m_d

#define	PPC		m_ppc.w.l
#define PC  	m_pc.w.l
#define PCD 	m_pc.d
#define U		m_u.w.l
#define UD		m_u.d
#define S		m_s.w.l
#define SD		m_s.d
#define X		m_x.w.l
#define XD		m_x.d
#define Y		m_y.w.l
#define YD		m_y.d
#define D   	m_d.w.l
#define A   	m_d.b.h
#define B		m_d.b.l
#define DP		m_dp.b.h
#define DPD 	m_dp.d
#define CC  	m_cc

#define EA		m_ea.w.l
#define EAD 	m_ea.d
#define EAP 	m_ea

#define M6809_CWAI		8	/* set when CWAI is waiting for an interrupt */
#define M6809_SYNC		16	/* set when SYNC is waiting for an interrupt */
#define M6809_LDS		32	/* set when LDS occurred at least once */


/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(addr) ((unsigned)m_program->read_byte(addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (m_program->write_byte(Addr,Value))

/**************************************************************************/
/* ROP() is identical to RM16() except it is used for reading opcodes. In */
/* the case of a system with memory mapped I/O, this function can be used */
/* to greatly speed up emulation.                                         */
/**************************************************************************/
#define ROP(Addr) ((unsigned)m_direct->read_decrypted_byte(Addr))

/************************************************************************/
/* ROP_ARG() is identical to ROP() except it is used for reading opcode */
/* arguments. This difference can be used to support systems that use   */
/* different encoding mechanisms for opcodes and opcode arguments.      */
/************************************************************************/
#define ROP_ARG(addr) ((unsigned)m_direct->read_raw_byte(addr))

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); PC++
#define IMMWORD(w)	w.d = (ROP_ARG(PCD)<<8) | ROP_ARG((PCD+1)&0xffff); PC+=2

#define PUSHBYTE(b) --S; WM(SD,b)
#define PUSHWORD(w) --S; WM(SD,w.b.l); --S; WM(SD,w.b.h)
#define PULLBYTE(b) b = RM(SD); S++
#define PULLWORD(w) w = RM(SD)<<8; S++; w |= RM(SD); S++

#define PSHUBYTE(b) --U; WM(UD,b);
#define PSHUWORD(w) --U; WM(UD,w.b.l); --U; WM(UD,w.b.h)
#define PULUBYTE(b) b = RM(UD); U++
#define PULUWORD(w) w = RM(UD)<<8; U++; w |= RM(UD); U++

#define CLR_HNZVC   CC&=~(CC_H|CC_N|CC_Z|CC_V|CC_C)
#define CLR_NZV 	CC&=~(CC_N|CC_Z|CC_V)
#define CLR_NZ		CC&=~(CC_N|CC_Z)
#define CLR_HNZC	CC&=~(CC_H|CC_N|CC_Z|CC_C)
#define CLR_NZVC	CC&=~(CC_N|CC_Z|CC_V|CC_C)
#define CLR_Z		CC&=~(CC_Z)
#define CLR_NZC 	CC&=~(CC_N|CC_Z|CC_C)
#define CLR_ZC		CC&=~(CC_Z|CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)		if(!a)SEZ
#define SET_Z8(a)		SET_Z((UINT8)a)
#define SET_Z16(a)		SET_Z((UINT16)a)
#define SET_N8(a)		CC|=((a&0x80)>>4)
#define SET_N16(a)		CC|=((a&0x8000)>>12)
#define SET_H(a,b,r)	CC|=(((a^b^r)&0x10)<<1)
#define SET_C8(a)		CC|=((a&0x100)>>8)
#define SET_C16(a)		CC|=((a&0x10000)>>16)
#define SET_V8(a,b,r)	CC|=(((a^b^r^(r>>1))&0x80)>>6)
#define SET_V16(a,b,r)	CC|=(((a^b^r^(r>>1))&0x8000)>>14)

#define SET_FLAGS8I(a)		{CC|=m_flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=m_flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z(a);}
#define SET_NZ16(a)			{SET_N16(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#define NXORV			((CC&CC_N)^((CC&CC_V)<<2))

/* for treating an unsigned byte as a signed word */
#define SIGNED(b) ((UINT16)(b&0x80?b|0xff00:b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT	EAD = DPD; IMMBYTE(m_ea.b.l)
#define IMM8	EAD = PCD; PC++
#define IMM16	EAD = PCD; PC+=2
#define EXTENDED IMMWORD(EAP)

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=CC_C
#define CLC CC&=~CC_C
#define SEZ CC|=CC_Z
#define CLZ CC&=~CC_Z
#define SEN CC|=CC_N
#define CLN CC&=~CC_N
#define SEV CC|=CC_V
#define CLV CC&=~CC_V
#define SEH CC|=CC_H
#define CLH CC&=~CC_H

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}

/* macros for branch instructions */
#define BRANCH(f) { 					\
	UINT8 t;							\
	IMMBYTE(t); 						\
	if( f ) 							\
	{									\
		PC += SIGNED(t);				\
	}									\
}

#define LBRANCH(f) {                    \
	PAIR t; 							\
	IMMWORD(t); 						\
	if( f ) 							\
	{									\
		m_icount -= 1;					\
		PC += t.w.l;					\
	}									\
}

/* macros for setting/getting registers in TFR/EXG instructions */

UINT32 m6809_base_device::RM16(UINT32 addr)
{
	UINT32 result = RM(addr) << 8;
	return result | RM((addr+1)&0xffff);
}

void m6809_base_device::WM16(UINT32 addr, PAIR *p)
{
	WM(addr, p->b.h);
	WM((addr+1)&0xffff, p->b.l);
}

void m6809_base_device::update_state()
{
	/* compatibility with 6309 */
}

void m6809_base_device::check_irq_lines()
{
	if( m_irq_state[M6809_IRQ_LINE] != CLEAR_LINE ||
		m_irq_state[M6809_FIRQ_LINE] != CLEAR_LINE )
		m_int_state &= ~M6809_SYNC; /* clear SYNC flag */
	if( m_irq_state[M6809_FIRQ_LINE]!=CLEAR_LINE && !(CC & CC_IF) )
	{
		/* fast IRQ */
		/* HJB 990225: state already saved by CWAI? */
		if( m_int_state & M6809_CWAI )
		{
			m_int_state &= ~M6809_CWAI;  /* clear CWAI */
			m_extra_cycles += 7;		 /* subtract +7 cycles */
        }
		else
		{
			CC &= ~CC_E;				/* save 'short' state */
			PUSHWORD(pPC);
			PUSHBYTE(CC);
			m_extra_cycles += 10;	/* subtract +10 cycles */
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD=RM16(0xfff6);
		standard_irq_callback(M6809_FIRQ_LINE);
	}
	else
	if( m_irq_state[M6809_IRQ_LINE]!=CLEAR_LINE && !(CC & CC_II) )
	{
		/* standard IRQ */
		/* HJB 990225: state already saved by CWAI? */
		if( m_int_state & M6809_CWAI )
		{
			m_int_state &= ~M6809_CWAI;  /* clear CWAI flag */
			m_extra_cycles += 7;		 /* subtract +7 cycles */
		}
		else
		{
			CC |= CC_E; 				/* save entire state */
			PUSHWORD(pPC);
			PUSHWORD(pU);
			PUSHWORD(pY);
			PUSHWORD(pX);
			PUSHBYTE(DP);
			PUSHBYTE(B);
			PUSHBYTE(A);
			PUSHBYTE(CC);
			m_extra_cycles += 19;	 /* subtract +19 cycles */
		}
		CC |= CC_II;					/* inhibit IRQ */
		PCD=RM16(0xfff8);
		standard_irq_callback(M6809_IRQ_LINE);
	}
}


//-------------------------------------------------
//  static_set_config - set the configuration
//  structure
//-------------------------------------------------

void m6809_base_device::static_set_config(device_t &device, const m6809_config &config)
{
	m6809_base_device &m6809 = downcast<m6809_base_device &>(device);
	static_cast<m6809_config &>(m6809) = config;
	static_set_static_config(device, &config);
}

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type M6809 = &device_creator<m6809_device>;
const device_type M6809E = &device_creator<m6809e_device>;

//-------------------------------------------------
//  atmega8_device - constructor
//-------------------------------------------------

m6809_base_device::m6809_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const device_type type, int divider)
	: cpu_device(mconfig, type, "M6809", tag, owner, clock),
	m_program_config("program", ENDIANNESS_BIG, 8, 16),
	m_clock_divider(divider)
{
	// build the opcode table
	for (int op = 0; op < 256; op++)
		m_opcode[op] = s_opcodetable[op];
}


//**************************************************************************
//  STATIC OPCODE TABLES
//**************************************************************************

const m6809_base_device::ophandler m6809_base_device::s_opcodetable[256] =
{
	//			0xX0/4/8/C,   				0xX1/5/9/D,   				0xX2/6/A/E,   				0xX3/7/B/F

	/* 0x0X */	&m6809_base_device::neg_di,	&m6809_base_device::neg_di,	&m6809_base_device::com_di,	&m6809_base_device::com_di,
				&m6809_base_device::lsr_di,	&m6809_base_device::lsr_di,	&m6809_base_device::ror_di,	&m6809_base_device::asr_di,
				&m6809_base_device::asl_di,	&m6809_base_device::rol_di,	&m6809_base_device::dec_di,	&m6809_base_device::dec_di,
				&m6809_base_device::inc_di,	&m6809_base_device::tst_di,	&m6809_base_device::jmp_di,	&m6809_base_device::clr_di,

	/* 0x1X */	&m6809_base_device::pref10,	&m6809_base_device::pref11,	&m6809_base_device::nop,	&m6809_base_device::sync,
				&m6809_base_device::illegal,&m6809_base_device::illegal,&m6809_base_device::lbra,	&m6809_base_device::lbsr,
				&m6809_base_device::illegal,&m6809_base_device::daa,	&m6809_base_device::orcc,	&m6809_base_device::illegal,
				&m6809_base_device::andcc,	&m6809_base_device::sex,	&m6809_base_device::exg,	&m6809_base_device::tfr,

	/* 0x2X */	&m6809_base_device::bra, 	&m6809_base_device::brn,	&m6809_base_device::bhi,	&m6809_base_device::bls,
				&m6809_base_device::bcc,	&m6809_base_device::bcs,	&m6809_base_device::bne,	&m6809_base_device::beq,
				&m6809_base_device::bvc,	&m6809_base_device::bvs,	&m6809_base_device::bpl,	&m6809_base_device::bmi,
				&m6809_base_device::bge,	&m6809_base_device::blt,	&m6809_base_device::bgt,	&m6809_base_device::ble,

	/* 0x3X */	&m6809_base_device::leax,	&m6809_base_device::leay,	&m6809_base_device::leas,	&m6809_base_device::leau,
				&m6809_base_device::pshs,	&m6809_base_device::puls,	&m6809_base_device::pshu,	&m6809_base_device::pulu,
				&m6809_base_device::illegal,&m6809_base_device::rts,	&m6809_base_device::abx,	&m6809_base_device::rti,
				&m6809_base_device::cwai,	&m6809_base_device::mul,	&m6809_base_device::illegal,&m6809_base_device::swi,

	/* 0x4X */	&m6809_base_device::nega,	&m6809_base_device::nega,	&m6809_base_device::coma,	&m6809_base_device::coma,
				&m6809_base_device::lsra,	&m6809_base_device::lsra,	&m6809_base_device::rora,	&m6809_base_device::asra,
				&m6809_base_device::asla,	&m6809_base_device::rola,	&m6809_base_device::deca,	&m6809_base_device::deca,
				&m6809_base_device::inca,	&m6809_base_device::tsta,	&m6809_base_device::clra,	&m6809_base_device::clra,

	/* 0x5X */	&m6809_base_device::negb,	&m6809_base_device::negb,	&m6809_base_device::comb,	&m6809_base_device::comb,
				&m6809_base_device::lsrb,	&m6809_base_device::lsrb,	&m6809_base_device::rorb,	&m6809_base_device::asrb,
				&m6809_base_device::aslb,	&m6809_base_device::rolb,	&m6809_base_device::decb,	&m6809_base_device::decb,
				&m6809_base_device::incb,	&m6809_base_device::tstb,	&m6809_base_device::clrb,	&m6809_base_device::clrb,

	/* 0x6X */	&m6809_base_device::neg_ix,	&m6809_base_device::neg_ix,	&m6809_base_device::com_ix,	&m6809_base_device::com_ix,
				&m6809_base_device::lsr_ix,	&m6809_base_device::lsr_ix,	&m6809_base_device::ror_ix,	&m6809_base_device::asr_ix,
				&m6809_base_device::asl_ix,	&m6809_base_device::rol_ix,	&m6809_base_device::dec_ix,	&m6809_base_device::dec_ix,
				&m6809_base_device::inc_ix,	&m6809_base_device::tst_ix,	&m6809_base_device::jmp_ix,	&m6809_base_device::clr_ix,

	/* 0x7X */	&m6809_base_device::neg_ex, &m6809_base_device::neg_ex, &m6809_base_device::com_ex, &m6809_base_device::com_ex,
				&m6809_base_device::lsr_ex, &m6809_base_device::lsr_ex, &m6809_base_device::ror_ex, &m6809_base_device::asr_ex,
				&m6809_base_device::asl_ex, &m6809_base_device::rol_ex, &m6809_base_device::dec_ex, &m6809_base_device::dec_ex,
				&m6809_base_device::inc_ex, &m6809_base_device::tst_ex, &m6809_base_device::jmp_ex, &m6809_base_device::clr_ex,

	/* 0x8X */	&m6809_base_device::suba_im,&m6809_base_device::cmpa_im,&m6809_base_device::sbca_im,&m6809_base_device::subd_im,
				&m6809_base_device::anda_im,&m6809_base_device::bita_im,&m6809_base_device::lda_im, &m6809_base_device::sta_im,
				&m6809_base_device::eora_im,&m6809_base_device::adca_im,&m6809_base_device::ora_im, &m6809_base_device::adda_im,
				&m6809_base_device::cmpx_im,&m6809_base_device::bsr,	&m6809_base_device::ldx_im, &m6809_base_device::stx_im,

	/* 0x9X */	&m6809_base_device::suba_di,&m6809_base_device::cmpa_di,&m6809_base_device::sbca_di,&m6809_base_device::subd_di,
				&m6809_base_device::anda_di,&m6809_base_device::bita_di,&m6809_base_device::lda_di, &m6809_base_device::sta_di,
				&m6809_base_device::eora_di,&m6809_base_device::adca_di,&m6809_base_device::ora_di, &m6809_base_device::adda_di,
				&m6809_base_device::cmpx_di,&m6809_base_device::jsr_di, &m6809_base_device::ldx_di, &m6809_base_device::stx_di,

	/* 0xAX */	&m6809_base_device::suba_ix,&m6809_base_device::cmpa_ix,&m6809_base_device::sbca_ix,&m6809_base_device::subd_ix,
				&m6809_base_device::anda_ix,&m6809_base_device::bita_ix,&m6809_base_device::lda_ix, &m6809_base_device::sta_ix,
				&m6809_base_device::eora_ix,&m6809_base_device::adca_ix,&m6809_base_device::ora_ix, &m6809_base_device::adda_ix,
				&m6809_base_device::cmpx_ix,&m6809_base_device::jsr_ix, &m6809_base_device::ldx_ix, &m6809_base_device::stx_ix,

	/* 0xBX */	&m6809_base_device::suba_ex,&m6809_base_device::cmpa_ex,&m6809_base_device::sbca_ex,&m6809_base_device::subd_ex,
				&m6809_base_device::anda_ex,&m6809_base_device::bita_ex,&m6809_base_device::lda_ex, &m6809_base_device::sta_ex,
				&m6809_base_device::eora_ex,&m6809_base_device::adca_ex,&m6809_base_device::ora_ex, &m6809_base_device::adda_ex,
				&m6809_base_device::cmpx_ex,&m6809_base_device::jsr_ex, &m6809_base_device::ldx_ex, &m6809_base_device::stx_ex,

	/* 0xCX */	&m6809_base_device::subb_im,&m6809_base_device::cmpb_im,&m6809_base_device::sbcb_im,&m6809_base_device::addd_im,
				&m6809_base_device::andb_im,&m6809_base_device::bitb_im,&m6809_base_device::ldb_im, &m6809_base_device::stb_im,
				&m6809_base_device::eorb_im,&m6809_base_device::adcb_im,&m6809_base_device::orb_im, &m6809_base_device::addb_im,
				&m6809_base_device::ldd_im, &m6809_base_device::std_im, &m6809_base_device::ldu_im, &m6809_base_device::stu_im,

	/* 0xDX */	&m6809_base_device::subb_di,&m6809_base_device::cmpb_di,&m6809_base_device::sbcb_di,&m6809_base_device::addd_di,
				&m6809_base_device::andb_di,&m6809_base_device::bitb_di,&m6809_base_device::ldb_di, &m6809_base_device::stb_di,
				&m6809_base_device::eorb_di,&m6809_base_device::adcb_di,&m6809_base_device::orb_di, &m6809_base_device::addb_di,
				&m6809_base_device::ldd_di, &m6809_base_device::std_di, &m6809_base_device::ldu_di, &m6809_base_device::stu_di,

	/* 0xEX */	&m6809_base_device::subb_ix,&m6809_base_device::cmpb_ix,&m6809_base_device::sbcb_ix,&m6809_base_device::addd_ix,
				&m6809_base_device::andb_ix,&m6809_base_device::bitb_ix,&m6809_base_device::ldb_ix, &m6809_base_device::stb_ix,
				&m6809_base_device::eorb_ix,&m6809_base_device::adcb_ix,&m6809_base_device::orb_ix, &m6809_base_device::addb_ix,
				&m6809_base_device::ldd_ix, &m6809_base_device::std_ix, &m6809_base_device::ldu_ix, &m6809_base_device::stu_ix,

	/* 0xFX */	&m6809_base_device::subb_ex,&m6809_base_device::cmpb_ex,&m6809_base_device::sbcb_ex,&m6809_base_device::addd_ex,
				&m6809_base_device::andb_ex,&m6809_base_device::bitb_ex,&m6809_base_device::ldb_ex, &m6809_base_device::stb_ex,
				&m6809_base_device::eorb_ex,&m6809_base_device::adcb_ex,&m6809_base_device::orb_ex, &m6809_base_device::addb_ex,
				&m6809_base_device::ldd_ex, &m6809_base_device::std_ex, &m6809_base_device::ldu_ex, &m6809_base_device::stu_ex
	};


/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
void m6809_base_device::device_start()
{
	/* default configuration */
	static const m6809_config default_config =
	{
		false
	};

	if (!static_config())
	{
		static_set_config(*this, default_config);
	}

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// register our state for the debugger
	astring tempstr;
	state_add(STATE_GENPC,     "GENPC",     m_pc.w.l).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_cc).callimport().callexport().formatstr("%8s").noshow();
	state_add(M6809_PC,        "PC",    	m_pc.w.l).mask(0xffff);
	state_add(M6809_S,         "S",        	m_s.w.l).mask(0xffff);
	state_add(M6809_CC,        "CC",        m_cc).mask(0xff);
	state_add(M6809_U,         "U",        	m_u.w.l).mask(0xffff);
	state_add(M6809_A,         "A",        	m_d.b.h).mask(0xff);
	state_add(M6809_B,         "B",        	m_d.b.l).mask(0xff);
	state_add(M6809_X,         "X",        	m_x.w.l).mask(0xffff);
	state_add(M6809_Y,         "Y",        	m_y.w.l).mask(0xffff);
	state_add(M6809_DP,        "DP",        m_dp.w.l).mask(0xffff);

	/* setup regtable */
	save_item(NAME(PC));
	save_item(NAME(PPC));
	save_item(NAME(D));
	save_item(NAME(DP));
	save_item(NAME(U));
	save_item(NAME(S));
	save_item(NAME(X));
	save_item(NAME(Y));
	save_item(NAME(CC));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_int_state));
	save_item(NAME(m_nmi_state));

	// set our instruction counter
	m_icountptr = &m_icount;
}

/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
void m6809_base_device::device_reset()
{
	m_int_state = 0;
	m_nmi_state = CLEAR_LINE;
	m_irq_state[0] = CLEAR_LINE;
	m_irq_state[1] = CLEAR_LINE;

	DPD = 0;			/* Reset direct page register */

	CC |= CC_II;        /* IRQ disabled */
	CC |= CC_IF;        /* FIRQ disabled */

	PCD = RM16(0xfffe);
	update_state();
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *m6809_base_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	return NULL;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void m6809_base_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
			    (m_cc & 0x80) ? 'E' : '.',
			    (m_cc & 0x40) ? 'F' : '.',
			    (m_cc & 0x20) ? 'H' : '.',
			    (m_cc & 0x10) ? 'I' : '.',
			    (m_cc & 0x08) ? 'N' : '.',
			    (m_cc & 0x04) ? 'Z' : '.',
			    (m_cc & 0x02) ? 'V' : '.',
			    (m_cc & 0x01) ? 'C' : '.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 m6809_base_device::disasm_min_opcode_bytes() const
{
	return 1;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 m6809_base_device::disasm_max_opcode_bytes() const
{
	return 5;
}


//-------------------------------------------------
//  execute_clocks_to_cycles - convert the raw
//  clock into cycles per second
//-------------------------------------------------

UINT64 m6809_base_device::execute_clocks_to_cycles(UINT64 clocks) const
{
	return (clocks + m_clock_divider - 1) / m_clock_divider;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert a cycle
//  count back to raw clocks
//-------------------------------------------------

UINT64 m6809_base_device::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles * m_clock_divider;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t m6809_base_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( m6809 );
	return disassemble(buffer, pc, oprom, opram, 0);
}

//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void m6809_base_device::set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m_nmi_state == state) return;
		m_nmi_state = state;
		LOG(("M6809 '%s' set_irq_line (NMI) %d\n", m_tag, state));
		if( state == CLEAR_LINE ) return;

		/* if the stack was not yet initialized */
	    if( !(m_int_state & M6809_LDS) ) return;

	    m_int_state &= ~M6809_SYNC;
		/* HJB 990225: state already saved by CWAI? */
		if( m_int_state & M6809_CWAI )
		{
			m_int_state &= ~M6809_CWAI;
			m_extra_cycles += 7;	/* subtract +7 cycles next time */
	    }
		else
		{
			CC |= CC_E; 				/* save entire state */
			PUSHWORD(pPC);
			PUSHWORD(pU);
			PUSHWORD(pY);
			PUSHWORD(pX);
			PUSHBYTE(DP);
			PUSHBYTE(B);
			PUSHBYTE(A);
			PUSHBYTE(CC);
			m_extra_cycles += 19;	/* subtract +19 cycles next time */
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD = RM16(0xfffc);
	}
	else if (irqline < 2)
	{
	    LOG(("M6809 '%s' set_irq_line %d, %d\n", m_tag, irqline, state));
		m_irq_state[irqline] = state;
		if (state == CLEAR_LINE) return;
		check_irq_lines();
	}
}

/****************************************************************************
 * includes the actual opcode implementations
 ****************************************************************************/
#include "6809tbl.c"

#include "6809ops.c"

//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 m6809_base_device::execute_min_cycles() const
{
	return 2;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 m6809_base_device::execute_max_cycles() const
{
	return 19;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 m6809_base_device::execute_input_lines() const
{
	return 3;
}


void m6809_base_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum)
	{
		case M6809_FIRQ_LINE:
			set_irq_line(M6809_FIRQ_LINE, state);
			break;
		case M6809_IRQ_LINE:
			set_irq_line(M6809_IRQ_LINE, state);
			break;
		case INPUT_LINE_NMI:
			set_irq_line(INPUT_LINE_NMI, state);
			break;
	}
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void m6809_base_device::execute_run()
{
    m_icount -= m_extra_cycles;
	m_extra_cycles = 0;

	check_irq_lines();

	if (m_int_state & (M6809_CWAI | M6809_SYNC))
	{
		debugger_instruction_hook(this, PCD);
		m_icount = 0;
	}
	else
	{
		do
		{
			pPPC = pPC;

			debugger_instruction_hook(this, PCD);

			m_ireg = ROP(PCD);
			PC++;
        	(this->*m_opcode[m_ireg])();
			m_icount -= m_cycles1[m_ireg];

		} while( m_icount > 0 );

        m_icount -= m_extra_cycles;
		m_extra_cycles = 0;
    }
}

void m6809_base_device::fetch_effective_address()
{
	UINT8 postbyte = ROP_ARG(PCD);
	PC++;

	switch(postbyte)
	{
	case 0x00: EA=X;												   break;
	case 0x01: EA=X+1;												   break;
	case 0x02: EA=X+2;												   break;
	case 0x03: EA=X+3;												   break;
	case 0x04: EA=X+4;												   break;
	case 0x05: EA=X+5;												   break;
	case 0x06: EA=X+6;												   break;
	case 0x07: EA=X+7;												   break;
	case 0x08: EA=X+8;												   break;
	case 0x09: EA=X+9;												   break;
	case 0x0a: EA=X+10; 											   break;
	case 0x0b: EA=X+11; 											   break;
	case 0x0c: EA=X+12; 											   break;
	case 0x0d: EA=X+13; 											   break;
	case 0x0e: EA=X+14; 											   break;
	case 0x0f: EA=X+15; 											   break;

	case 0x10: EA=X-16; 											   break;
	case 0x11: EA=X-15; 											   break;
	case 0x12: EA=X-14; 											   break;
	case 0x13: EA=X-13; 											   break;
	case 0x14: EA=X-12; 											   break;
	case 0x15: EA=X-11; 											   break;
	case 0x16: EA=X-10; 											   break;
	case 0x17: EA=X-9;												   break;
	case 0x18: EA=X-8;												   break;
	case 0x19: EA=X-7;												   break;
	case 0x1a: EA=X-6;												   break;
	case 0x1b: EA=X-5;												   break;
	case 0x1c: EA=X-4;												   break;
	case 0x1d: EA=X-3;												   break;
	case 0x1e: EA=X-2;												   break;
	case 0x1f: EA=X-1;												   break;

	case 0x20: EA=Y;												   break;
	case 0x21: EA=Y+1;												   break;
	case 0x22: EA=Y+2;												   break;
	case 0x23: EA=Y+3;												   break;
	case 0x24: EA=Y+4;												   break;
	case 0x25: EA=Y+5;												   break;
	case 0x26: EA=Y+6;												   break;
	case 0x27: EA=Y+7;												   break;
	case 0x28: EA=Y+8;												   break;
	case 0x29: EA=Y+9;												   break;
	case 0x2a: EA=Y+10; 											   break;
	case 0x2b: EA=Y+11; 											   break;
	case 0x2c: EA=Y+12; 											   break;
	case 0x2d: EA=Y+13; 											   break;
	case 0x2e: EA=Y+14; 											   break;
	case 0x2f: EA=Y+15; 											   break;

	case 0x30: EA=Y-16; 											   break;
	case 0x31: EA=Y-15; 											   break;
	case 0x32: EA=Y-14; 											   break;
	case 0x33: EA=Y-13; 											   break;
	case 0x34: EA=Y-12; 											   break;
	case 0x35: EA=Y-11; 											   break;
	case 0x36: EA=Y-10; 											   break;
	case 0x37: EA=Y-9;												   break;
	case 0x38: EA=Y-8;												   break;
	case 0x39: EA=Y-7;												   break;
	case 0x3a: EA=Y-6;												   break;
	case 0x3b: EA=Y-5;												   break;
	case 0x3c: EA=Y-4;												   break;
	case 0x3d: EA=Y-3;												   break;
	case 0x3e: EA=Y-2;												   break;
	case 0x3f: EA=Y-1;												   break;

	case 0x40: EA=U;												   break;
	case 0x41: EA=U+1;												   break;
	case 0x42: EA=U+2;												   break;
	case 0x43: EA=U+3;												   break;
	case 0x44: EA=U+4;												   break;
	case 0x45: EA=U+5;												   break;
	case 0x46: EA=U+6;												   break;
	case 0x47: EA=U+7;												   break;
	case 0x48: EA=U+8;												   break;
	case 0x49: EA=U+9;												   break;
	case 0x4a: EA=U+10; 											   break;
	case 0x4b: EA=U+11; 											   break;
	case 0x4c: EA=U+12; 											   break;
	case 0x4d: EA=U+13; 											   break;
	case 0x4e: EA=U+14; 											   break;
	case 0x4f: EA=U+15; 											   break;

	case 0x50: EA=U-16; 											   break;
	case 0x51: EA=U-15; 											   break;
	case 0x52: EA=U-14; 											   break;
	case 0x53: EA=U-13; 											   break;
	case 0x54: EA=U-12; 											   break;
	case 0x55: EA=U-11; 											   break;
	case 0x56: EA=U-10; 											   break;
	case 0x57: EA=U-9;												   break;
	case 0x58: EA=U-8;												   break;
	case 0x59: EA=U-7;												   break;
	case 0x5a: EA=U-6;												   break;
	case 0x5b: EA=U-5;												   break;
	case 0x5c: EA=U-4;												   break;
	case 0x5d: EA=U-3;												   break;
	case 0x5e: EA=U-2;												   break;
	case 0x5f: EA=U-1;												   break;

	case 0x60: EA=S;												   break;
	case 0x61: EA=S+1;												   break;
	case 0x62: EA=S+2;												   break;
	case 0x63: EA=S+3;												   break;
	case 0x64: EA=S+4;												   break;
	case 0x65: EA=S+5;												   break;
	case 0x66: EA=S+6;												   break;
	case 0x67: EA=S+7;												   break;
	case 0x68: EA=S+8;												   break;
	case 0x69: EA=S+9;												   break;
	case 0x6a: EA=S+10; 											   break;
	case 0x6b: EA=S+11; 											   break;
	case 0x6c: EA=S+12; 											   break;
	case 0x6d: EA=S+13; 											   break;
	case 0x6e: EA=S+14; 											   break;
	case 0x6f: EA=S+15; 											   break;

	case 0x70: EA=S-16; 											   break;
	case 0x71: EA=S-15; 											   break;
	case 0x72: EA=S-14; 											   break;
	case 0x73: EA=S-13; 											   break;
	case 0x74: EA=S-12; 											   break;
	case 0x75: EA=S-11; 											   break;
	case 0x76: EA=S-10; 											   break;
	case 0x77: EA=S-9;												   break;
	case 0x78: EA=S-8;												   break;
	case 0x79: EA=S-7;												   break;
	case 0x7a: EA=S-6;												   break;
	case 0x7b: EA=S-5;												   break;
	case 0x7c: EA=S-4;												   break;
	case 0x7d: EA=S-3;												   break;
	case 0x7e: EA=S-2;												   break;
	case 0x7f: EA=S-1;												   break;

	case 0x80: EA=X;	X++;										   break;
	case 0x81: EA=X;	X+=2;										   break;
	case 0x82: X--; 	EA=X;										   break;
	case 0x83: X-=2;	EA=X;										   break;
	case 0x84: EA=X;												   break;
	case 0x85: EA=X+SIGNED(B);										   break;
	case 0x86: EA=X+SIGNED(A);										   break;
	case 0x87: EA=0;												   break; /*   ILLEGAL*/
	case 0x88: IMMBYTE(EA); 	EA=X+SIGNED(EA);					   break;
	case 0x89: IMMWORD(EAP);	EA+=X;								   break;
	case 0x8a: EA=0;												   break; /*   IIError*/
	case 0x8b: EA=X+D;												   break;
	case 0x8c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0x8d: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0x8e: EA=0;												   break; /*   ILLEGAL*/
	case 0x8f: IMMWORD(EAP);										   break;

	case 0x90: EA=X;	X++;						EAD=RM16(EAD);	   break; /* Indirect ,R+ not in my specs */
	case 0x91: EA=X;	X+=2;						EAD=RM16(EAD);	   break;
	case 0x92: X--; 	EA=X;						EAD=RM16(EAD);	   break;
	case 0x93: X-=2;	EA=X;						EAD=RM16(EAD);	   break;
	case 0x94: EA=X;								EAD=RM16(EAD);	   break;
	case 0x95: EA=X+SIGNED(B);						EAD=RM16(EAD);	   break;
	case 0x96: EA=X+SIGNED(A);						EAD=RM16(EAD);	   break;
	case 0x97: EA=0;												   break; /*   ILLEGAL*/
	case 0x98: IMMBYTE(EA); 	EA=X+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0x99: IMMWORD(EAP);	EA+=X;				EAD=RM16(EAD);	   break;
	case 0x9a: EA=0;												   break; /*   ILLEGAL*/
	case 0x9b: EA=X+D;								EAD=RM16(EAD);	   break;
	case 0x9c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0x9d: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(EAD);	   break;
	case 0x9e: EA=0;												   break; /*   ILLEGAL*/
	case 0x9f: IMMWORD(EAP);						EAD=RM16(EAD);	   break;

	case 0xa0: EA=Y;	Y++;										   break;
	case 0xa1: EA=Y;	Y+=2;										   break;
	case 0xa2: Y--; 	EA=Y;										   break;
	case 0xa3: Y-=2;	EA=Y;										   break;
	case 0xa4: EA=Y;												   break;
	case 0xa5: EA=Y+SIGNED(B);										   break;
	case 0xa6: EA=Y+SIGNED(A);										   break;
	case 0xa7: EA=0;												   break; /*   ILLEGAL*/
	case 0xa8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);					   break;
	case 0xa9: IMMWORD(EAP);	EA+=Y;								   break;
	case 0xaa: EA=0;												   break; /*   ILLEGAL*/
	case 0xab: EA=Y+D;												   break;
	case 0xac: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0xad: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0xae: EA=0;												   break; /*   ILLEGAL*/
	case 0xaf: IMMWORD(EAP);										   break;

	case 0xb0: EA=Y;	Y++;						EAD=RM16(EAD);	   break;
	case 0xb1: EA=Y;	Y+=2;						EAD=RM16(EAD);	   break;
	case 0xb2: Y--; 	EA=Y;						EAD=RM16(EAD);	   break;
	case 0xb3: Y-=2;	EA=Y;						EAD=RM16(EAD);	   break;
	case 0xb4: EA=Y;								EAD=RM16(EAD);	   break;
	case 0xb5: EA=Y+SIGNED(B);						EAD=RM16(EAD);	   break;
	case 0xb6: EA=Y+SIGNED(A);						EAD=RM16(EAD);	   break;
	case 0xb7: EA=0;												   break; /*   ILLEGAL*/
	case 0xb8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0xb9: IMMWORD(EAP);	EA+=Y;				EAD=RM16(EAD);	   break;
	case 0xba: EA=0;												   break; /*   ILLEGAL*/
	case 0xbb: EA=Y+D;								EAD=RM16(EAD);	   break;
	case 0xbc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0xbd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(EAD);	   break;
	case 0xbe: EA=0;												   break; /*   ILLEGAL*/
	case 0xbf: IMMWORD(EAP);						EAD=RM16(EAD);	   break;

	case 0xc0: EA=U;			U++;								   break;
	case 0xc1: EA=U;			U+=2;								   break;
	case 0xc2: U--; 			EA=U;								   break;
	case 0xc3: U-=2;			EA=U;								   break;
	case 0xc4: EA=U;												   break;
	case 0xc5: EA=U+SIGNED(B);										   break;
	case 0xc6: EA=U+SIGNED(A);										   break;
	case 0xc7: EA=0;												   break; /*ILLEGAL*/
	case 0xc8: IMMBYTE(EA); 	EA=U+SIGNED(EA);					   break;
	case 0xc9: IMMWORD(EAP);	EA+=U;								   break;
	case 0xca: EA=0;												   break; /*ILLEGAL*/
	case 0xcb: EA=U+D;												   break;
	case 0xcc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0xcd: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0xce: EA=0;												   break; /*ILLEGAL*/
	case 0xcf: IMMWORD(EAP);										   break;

	case 0xd0: EA=U;	U++;						EAD=RM16(EAD);	   break;
	case 0xd1: EA=U;	U+=2;						EAD=RM16(EAD);	   break;
	case 0xd2: U--; 	EA=U;						EAD=RM16(EAD);	   break;
	case 0xd3: U-=2;	EA=U;						EAD=RM16(EAD);	   break;
	case 0xd4: EA=U;								EAD=RM16(EAD);	   break;
	case 0xd5: EA=U+SIGNED(B);						EAD=RM16(EAD);	   break;
	case 0xd6: EA=U+SIGNED(A);						EAD=RM16(EAD);	   break;
	case 0xd7: EA=0;												   break; /*ILLEGAL*/
	case 0xd8: IMMBYTE(EA); 	EA=U+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0xd9: IMMWORD(EAP);	EA+=U;				EAD=RM16(EAD);	   break;
	case 0xda: EA=0;												   break; /*ILLEGAL*/
	case 0xdb: EA=U+D;								EAD=RM16(EAD);	   break;
	case 0xdc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0xdd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(EAD);	   break;
	case 0xde: EA=0;												   break; /*ILLEGAL*/
	case 0xdf: IMMWORD(EAP);						EAD=RM16(EAD);	   break;

	case 0xe0: EA=S;	S++;										   break;
	case 0xe1: EA=S;	S+=2;										   break;
	case 0xe2: S--; 	EA=S;										   break;
	case 0xe3: S-=2;	EA=S;										   break;
	case 0xe4: EA=S;												   break;
	case 0xe5: EA=S+SIGNED(B);										   break;
	case 0xe6: EA=S+SIGNED(A);										   break;
	case 0xe7: EA=0;												   break; /*ILLEGAL*/
	case 0xe8: IMMBYTE(EA); 	EA=S+SIGNED(EA);					   break;
	case 0xe9: IMMWORD(EAP);	EA+=S;								   break;
	case 0xea: EA=0;												   break; /*ILLEGAL*/
	case 0xeb: EA=S+D;												   break;
	case 0xec: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0xed: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0xee: EA=0;												   break;  /*ILLEGAL*/
	case 0xef: IMMWORD(EAP);										   break;

	case 0xf0: EA=S;	S++;						EAD=RM16(EAD);	   break;
	case 0xf1: EA=S;	S+=2;						EAD=RM16(EAD);	   break;
	case 0xf2: S--; 	EA=S;						EAD=RM16(EAD);	   break;
	case 0xf3: S-=2;	EA=S;						EAD=RM16(EAD);	   break;
	case 0xf4: EA=S;								EAD=RM16(EAD);	   break;
	case 0xf5: EA=S+SIGNED(B);						EAD=RM16(EAD);	   break;
	case 0xf6: EA=S+SIGNED(A);						EAD=RM16(EAD);	   break;
	case 0xf7: EA=0;												   break; /*ILLEGAL*/
	case 0xf8: IMMBYTE(EA); 	EA=S+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0xf9: IMMWORD(EAP);	EA+=S;				EAD=RM16(EAD);	   break;
	case 0xfa: EA=0;												   break; /*ILLEGAL*/
	case 0xfb: EA=S+D;								EAD=RM16(EAD);	   break;
	case 0xfc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	   break;
	case 0xfd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(EAD);	   break;
	case 0xfe: EA=0;												   break; /*ILLEGAL*/
	case 0xff: IMMWORD(EAP);						EAD=RM16(EAD);	   break;
	}
	m_icount -= m_index_cycle_em[postbyte];
}
