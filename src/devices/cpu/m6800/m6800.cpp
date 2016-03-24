// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** m6800: Portable 6800 class  emulator *************************************

    m68xx.c

    References:

        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    UINT16 must be 16 bit unsigned int
                            UINT8 must be 8 bit unsigned int
                            UINT32 must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

History
991031  ZV
    Added NSC-8105 support

990319  HJB
    Fixed wrong LSB/MSB order for push/pull word.
    Subtract .extra_cycles at the beginning/end of the exectuion loops.

990316  HJB
    Renamed to 6800, since that's the basic CPU.
    Added different cycle count tables for M6800/2/8, M6801/3 and m68xx.

990314  HJB
    Also added the M6800 subtype.

990311  HJB
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
TODO:
    Verify invalid opcodes for the different CPU types.
    Add proper credits to _info functions.
    Integrate m6808_Flags into the registers (multiple m6808 type CPUs?)

990301  HJB
    Modified the interrupt handling. No more pending interrupt checks.
    WAI opcode saves state, when an interrupt is taken (IRQ or OCI),
    the state is only saved if not already done by WAI.

*****************************************************************************/

/*

    Chip                RAM     NVRAM   ROM     SCI     r15-f   ports
    -----------------------------------------------------------------
    MC6800              -       -       -       no      no      4
    MC6802              128     32      -       no      no      4
    MC6802NS            128     -       -       no      no      4
    MC6808              -       -       -       no      no      4

    MC6801              128     64      2K      yes     no      4
    MC68701             128     64      -       yes     no      4
    MC6803              128     64      -       yes     no      4

    MC6801U4            192     32      4K      yes     yes     4
    MC6803U4            192     32      -       yes     yes     4

    HD6801              128     64      2K      yes     no      4
    HD6301V             128     -       4K      yes     no      4
    HD63701V            192     -       4K      yes     no      4
    HD6303R             128     -       -       yes     no      4

    HD6301X             192     -       4K      yes     yes     6
    HD6301Y             256     -       16K     yes     yes     6
    HD6303X             192     -       -       yes     yes     6
    HD6303Y             256     -       -       yes     yes     6

    NSC8105
    MS2010-A

*/

#include "emu.h"
#include "debugger.h"
#include "m6800.h"

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#if 0
/* CPU subtypes, needed for extra insn after TAP/CLI/SEI */
enum
{
	SUBTYPE_M6800,
	SUBTYPE_M6801,
	SUBTYPE_M6802,
	SUBTYPE_M6803,
	SUBTYPE_M6808,
	SUBTYPE_HD6301,
	SUBTYPE_HD63701,
	SUBTYPE_NSC8105
};
#endif


#if 0
static void hd63701_trap_pc();
#endif

#define pPPC    m_ppc
#define pPC     m_pc
#define pS      m_s
#define pX      m_x
#define pD      m_d

#define PC      m_pc.w.l
#define PCD     m_pc.d
#define S       m_s.w.l
#define SD      m_s.d
#define X       m_x.w.l
#define D       m_d.w.l
#define A       m_d.b.h
#define B       m_d.b.l
#define CC      m_cc

#define CT      m_counter.w.l
#define CTH     m_counter.w.h
#define CTD     m_counter.d
#define OC      m_output_compare.w.l
#define OCH     m_output_compare.w.h
#define OCD     m_output_compare.d
#define TOH     m_timer_over.w.l
#define TOD     m_timer_over.d

#define EAD     m_ea.d
#define EA      m_ea.w.l

/* point of next timer event */
static UINT32 timer_next;

/* memory interface */

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(Addr) ((unsigned)m_program->read_byte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (m_program->write_byte(Addr,Value))

/****************************************************************************/
/* M6800_RDOP() is identical to M6800_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M_RDOP(Addr) ((unsigned)m_decrypted_opcodes_direct->read_byte(Addr))

/****************************************************************************/
/* M6800_RDOP_ARG() is identical to M6800_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M_RDOP_ARG(Addr) ((unsigned)m_direct->read_byte(Addr))

/* macros to access memory */
#define IMMBYTE(b)  b = M_RDOP_ARG(PCD); PC++
#define IMMWORD(w)  w.d = (M_RDOP_ARG(PCD)<<8) | M_RDOP_ARG((PCD+1)&0xffff); PC+=2

#define PUSHBYTE(b) WM(SD,b); --S
#define PUSHWORD(w) WM(SD,w.b.l); --S; WM(SD,w.b.h); --S
#define PULLBYTE(b) S++; b = RM(SD)
#define PULLWORD(w) S++; w.d = RM(SD)<<8; S++; w.d |= RM(SD)

#define MODIFIED_tcsr { \
	m_irq2 = (m_tcsr&(m_tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF); \
}

#define SET_TIMER_EVENT {                   \
	timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;   \
}

/* cleanup high-word of counters */
#define CLEANUP_COUNTERS() {                        \
	OCH -= CTH;                                 \
	TOH -= CTH;                                 \
	CTH = 0;                                    \
	SET_TIMER_EVENT;                            \
}

/* when change freerunningcounter or outputcapture */
#define MODIFIED_counters {                     \
	OCH = (OC >= CT) ? CTH : CTH+1;             \
	SET_TIMER_EVENT;                            \
}

// I/O registers

enum
{
	IO_P1DDR = 0,
	IO_P2DDR,
	IO_P1DATA,
	IO_P2DATA,
	IO_P3DDR,
	IO_P4DDR,
	IO_P3DATA,
	IO_P4DATA,
	IO_TCSR,
	IO_CH,
	IO_CL,
	IO_OCRH,
	IO_OCRL,
	IO_ICRH,
	IO_ICRL,
	IO_P3CSR,
	IO_RMCR,
	IO_TRCSR,
	IO_RDR,
	IO_TDR,
	IO_RCR,
	IO_CAAH,
	IO_CAAL,
	IO_TCR1,
	IO_TCR2,
	IO_TSR,
	IO_OCR2H,
	IO_OCR2L,
	IO_OCR3H,
	IO_OCR3L,
	IO_ICR2H,
	IO_ICR2L
};

// serial I/O

#define M6800_RMCR_SS_MASK      0x03 // Speed Select
#define M6800_RMCR_SS_4096      0x03 // E / 4096
#define M6800_RMCR_SS_1024      0x02 // E / 1024
#define M6800_RMCR_SS_128       0x01 // E / 128
#define M6800_RMCR_SS_16        0x00 // E / 16
#define M6800_RMCR_CC_MASK      0x0c // Clock Control/Format Select

#define M6800_TRCSR_RDRF        0x80 // Receive Data Register Full
#define M6800_TRCSR_ORFE        0x40 // Over Run Framing Error
#define M6800_TRCSR_TDRE        0x20 // Transmit Data Register Empty
#define M6800_TRCSR_RIE         0x10 // Receive Interrupt Enable
#define M6800_TRCSR_RE          0x08 // Receive Enable
#define M6800_TRCSR_TIE         0x04 // Transmit Interrupt Enable
#define M6800_TRCSR_TE          0x02 // Transmit Enable
#define M6800_TRCSR_WU          0x01 // Wake Up

#define M6800_PORT2_IO4         0x10
#define M6800_PORT2_IO3         0x08

#define M6801_P3CSR_LE          0x08
#define M6801_P3CSR_OSS         0x10
#define M6801_P3CSR_IS3_ENABLE  0x40
#define M6801_P3CSR_IS3_FLAG    0x80

static const int M6800_RMCR_SS[] = { 16, 128, 1024, 4096 };

#define M6800_SERIAL_START      0
#define M6800_SERIAL_STOP       9

enum
{
	M6800_TX_STATE_INIT = 0,
	M6800_TX_STATE_READY
};

/* take interrupt */
#define TAKE_ICI enter_interrupt("M6800 '%s' take ICI\n",0xfff6)
#define TAKE_OCI enter_interrupt("M6800 '%s' take OCI\n",0xfff4)
#define TAKE_TOI enter_interrupt("M6800 '%s' take TOI\n",0xfff2)
#define TAKE_SCI enter_interrupt("M6800 '%s' take SCI\n",0xfff0)
#define TAKE_TRAP enter_interrupt("M6800 '%s' take TRAP\n",0xffee)

/* operate one instruction for */
#define ONE_MORE_INSN() {       \
	UINT8 ireg;                             \
	pPPC = pPC;                             \
	debugger_instruction_hook(this, PCD);                       \
	ireg=M_RDOP(PCD);                       \
	PC++;                                   \
	(this->*m_insn[ireg])();               \
	increment_counter(m_cycles[ireg]);    \
}

/* CC masks                       HI NZVC
                                7654 3210   */
#define CLR_HNZVC   CC&=0xd0
#define CLR_NZV     CC&=0xf1
#define CLR_HNZC    CC&=0xd2
#define CLR_NZVC    CC&=0xf0
#define CLR_Z       CC&=0xfb
#define CLR_NZC     CC&=0xf2
#define CLR_ZC      CC&=0xfa
#define CLR_C       CC&=0xfe

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)        if(!(a))SEZ
#define SET_Z8(a)       SET_Z((UINT8)(a))
#define SET_Z16(a)      SET_Z((UINT16)(a))
#define SET_N8(a)       CC|=(((a)&0x80)>>4)
#define SET_N16(a)      CC|=(((a)&0x8000)>>12)
#define SET_H(a,b,r)    CC|=((((a)^(b)^(r))&0x10)<<1)
#define SET_C8(a)       CC|=(((a)&0x100)>>8)
#define SET_C16(a)      CC|=(((a)&0x10000)>>16)
#define SET_V8(a,b,r)   CC|=((((a)^(b)^(r)^((r)>>1))&0x80)>>6)
#define SET_V16(a,b,r)  CC|=((((a)^(b)^(r)^((r)>>1))&0x8000)>>14)

const UINT8 m6800_cpu_device::flags8i[256]=     /* increment */
{
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};


const UINT8 m6800_cpu_device::flags8d[256]= /* decrement */
{
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

#define SET_FLAGS8I(a)      {CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)      {CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)          {SET_N8(a);SET_Z8(a);}
#define SET_NZ16(a)         {SET_N16(a);SET_Z16(a);}
#define SET_FLAGS8(a,b,r)   {SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)  {SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

/* for treating an UINT8 as a signed INT16 */
#define SIGNED(b) ((INT16)(b&0x80?b|0xff00:b))

/* Macros for addressing modes */
#define DIRECT IMMBYTE(EAD)
#define IMM8 EA=PC++
#define IMM16 {EA=PC;PC+=2;}
#define EXTENDED IMMWORD(m_ea)
#define INDEXED {EA=X+(UINT8)M_RDOP_ARG(PCD);PC++;}

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=0x01
#define CLC CC&=0xfe
#define SEZ CC|=0x04
#define CLZ CC&=0xfb
#define SEN CC|=0x08
#define CLN CC&=0xf7
#define SEV CC|=0x02
#define CLV CC&=0xfd
#define SEH CC|=0x20
#define CLH CC&=0xdf
#define SEI CC|=0x10
#define CLI CC&=~0x10

/* mnemonicos for the Timer Control and Status Register bits */
#define TCSR_OLVL 0x01
#define TCSR_IEDG 0x02
#define TCSR_ETOI 0x04
#define TCSR_EOCI 0x08
#define TCSR_EICI 0x10
#define TCSR_TOF  0x20
#define TCSR_OCF  0x40
#define TCSR_ICF  0x80

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}

#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDXWORD(w) {INDEXED;w.d=RM16(EAD);}

/* Macros for branch instructions */
#define BRANCH(f) {IMMBYTE(t);if(f){PC+=SIGNED(t);}}
#define NXORV  ((CC&0x08)^((CC&0x02)<<2))

#define M6800_WAI       8           /* set when WAI is waiting for an interrupt */
#define M6800_SLP       0x10        /* HD63701 only */

/* Note: don't use 0 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */
#define XX 5 // invalid opcode unknown cc
const UINT8 m6800_cpu_device::cycles_6800[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
	/*4*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*5*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*6*/  7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
	/*7*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*8*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
	/*9*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
	/*A*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
	/*B*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5,XX,XX, 6, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4,XX,XX, 5, 6
};

const UINT8 m6800_cpu_device::cycles_6803[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 2,XX,XX, 3, 3, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  3, 3, 4, 4, 3, 3, 3, 3, 5, 5, 3,10, 4,10, 9,12,
	/*4*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*5*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*6*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*7*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*8*/  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 6, 3, 3,
	/*9*/  3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4,
	/*A*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	/*B*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	/*C*/  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
	/*D*/  3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	/*E*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};

const UINT8 m6800_cpu_device::cycles_63701[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 1,XX,XX, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/*1*/  1, 1,XX,XX,XX,XX, 1, 1, 2, 2, 4, 1,XX,XX,XX,XX,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  1, 1, 3, 3, 1, 1, 4, 4, 4, 5, 1,10, 5, 7, 9,12,
	/*4*/  1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
	/*5*/  1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
	/*6*/  6, 7, 7, 6, 6, 7, 6, 6, 6, 6, 6, 5, 6, 4, 3, 5,
	/*7*/  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 3, 5,
	/*8*/  2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 5, 3, 3,
	/*9*/  3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4,
	/*A*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*B*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
	/*C*/  2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
	/*D*/  3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	/*E*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};

const UINT8 m6800_cpu_device::cycles_nsc8105[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/  5,XX, 2,XX,XX, 2,XX, 2, 4, 2, 4, 2, 2, 2, 2, 2,
	/*1*/  2,XX, 2,XX,XX, 2,XX, 2,XX,XX, 2, 2,XX,XX,XX,XX,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,XX,XX, 5,10,XX, 9,XX,12,
	/*4*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 3, 8, 4,
	/*5*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 4, 6, 5,
	/*6*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 6, 8, 7,
	/*7*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 5, 9, 6,
	/*8*/  2,XX,XX, 2, 2, 2,XX, 2, 2, 2, 2,XX, 2,XX, 2, 2,
	/*9*/  2,XX,XX, 2, 2, 2,XX, 2, 2, 2, 2,XX, 2,XX, 2, 2,
	/*A*/  7,XX,XX, 7, 7, 7,XX, 7, 7, 7, 7,XX, 7, 4, 7, 7,
	/*B*/  6,XX,XX, 6, 6, 6,XX, 6, 6, 6, 6, 5, 6, 3, 6, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX, 3,XX, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX, 4,XX, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 5, 6,XX, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5,XX, 6
};
#undef XX // /invalid opcode unknown cc

#define EAT_CYCLES                                                  \
{                                                                   \
	int cycles_to_eat;                                              \
																	\
	cycles_to_eat = timer_next - CTD;                               \
	if( cycles_to_eat > m_icount) cycles_to_eat = m_icount; \
	if (cycles_to_eat > 0)                                          \
	{                                                               \
		increment_counter(cycles_to_eat);                         \
	}                                                               \
}


const device_type M6800 = &device_creator<m6800_cpu_device>;
const device_type M6801 = &device_creator<m6801_cpu_device>;
const device_type M6802 = &device_creator<m6802_cpu_device>;
const device_type M6803 = &device_creator<m6803_cpu_device>;
const device_type M6808 = &device_creator<m6808_cpu_device>;
const device_type HD6301 = &device_creator<hd6301_cpu_device>;
const device_type HD63701 = &device_creator<hd63701_cpu_device>;
const device_type NSC8105 = &device_creator<nsc8105_cpu_device>;
const device_type HD6303R = &device_creator<hd6303r_cpu_device>;
const device_type HD6303Y = &device_creator<hd6303y_cpu_device>;


m6800_cpu_device::m6800_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, M6800, "M6800", tag, owner, clock, "m6800", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_decrypted_opcodes_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 9, 0)
	, m_has_io(false)
	, m_out_sc2_func(*this)
	, m_out_sertx_func(*this)
	, m_insn(m6800_insn)
	, m_cycles(cycles_6800)
{
	m_clock_divider = 1;
}

m6800_cpu_device::m6800_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, bool has_io, int clock_divider, const op_func *insn, const UINT8 *cycles, address_map_constructor internal)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, internal)
	, m_decrypted_opcodes_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 9, 0)
	, m_has_io(has_io)
	, m_out_sc2_func(*this)
	, m_out_sertx_func(*this)
	, m_insn(insn)
	, m_cycles(cycles)
{
	m_clock_divider = clock_divider;
}

m6801_cpu_device::m6801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6800_cpu_device(mconfig, M6801, "M6801", tag, owner, clock, "m6801", __FILE__, true, 4, m6803_insn, cycles_6803)
{
}

m6801_cpu_device::m6801_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, const op_func *insn, const UINT8 *cycles, address_map_constructor internal)
	: m6800_cpu_device(mconfig, type, name, tag, owner, clock, shortname, source, true, 4, insn, cycles, internal)
{
}

m6802_cpu_device::m6802_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6800_cpu_device(mconfig, M6802, "M6802", tag, owner, clock, "m6802", __FILE__, false, 4, m6800_insn, cycles_6800)
{
}

m6802_cpu_device::m6802_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, const op_func *insn, const UINT8 *cycles)
	: m6800_cpu_device(mconfig, type, name, tag, owner, clock, shortname, source, false, 4, insn, cycles)
{
}

static ADDRESS_MAP_START(m6803_mem, AS_PROGRAM, 8, m6800_cpu_device)
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0020, 0x007f) AM_NOP        /* unused */
	AM_RANGE(0x0080, 0x00ff) AM_RAM        /* 6803 internal RAM */
ADDRESS_MAP_END


m6803_cpu_device::m6803_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6801_cpu_device(mconfig, M6803, "M6803", tag, owner, clock, "m6803", __FILE__, m6803_insn, cycles_6803, ADDRESS_MAP_NAME(m6803_mem))
{
}

m6808_cpu_device::m6808_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6802_cpu_device(mconfig, M6808, "M6808", tag, owner, clock, "m6808", __FILE__, m6800_insn, cycles_6800)
{
}

hd6301_cpu_device::hd6301_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6801_cpu_device(mconfig, HD6301, "HD6301", tag, owner, clock, "hd6301", __FILE__, hd63701_insn, cycles_63701)
{
}

hd6301_cpu_device::hd6301_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: m6801_cpu_device(mconfig, type, name, tag, owner, clock, shortname, source, hd63701_insn, cycles_63701)
{
}

hd63701_cpu_device::hd63701_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6801_cpu_device(mconfig, HD63701, "HD63701", tag, owner, clock, "hd63701", __FILE__, hd63701_insn, cycles_63701)
{
}

nsc8105_cpu_device::nsc8105_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m6802_cpu_device(mconfig, NSC8105, "NSC8105", tag, owner, clock, "nsc8105", __FILE__, nsc8105_insn, cycles_nsc8105)
{
}

hd6303r_cpu_device::hd6303r_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hd6301_cpu_device(mconfig, HD6303R, "HD6303R", tag, owner, clock, "hd6303r", __FILE__)
{
}

hd6303y_cpu_device::hd6303y_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hd6301_cpu_device(mconfig, HD6303Y, "HD6303Y", tag, owner, clock, "hd6303y", __FILE__)
{
}

const address_space_config *m6800_cpu_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	case AS_IO:                return m_has_io ? &m_io_config : nullptr;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &m_decrypted_opcodes_config : nullptr;
	default:                   return nullptr;
	}
}


UINT32 m6800_cpu_device::RM16(UINT32 Addr )
{
	UINT32 result = RM(Addr) << 8;
	return result | RM((Addr+1)&0xffff);
}

void m6800_cpu_device::WM16(UINT32 Addr, PAIR *p )
{
	WM( Addr, p->b.h );
	WM( (Addr+1)&0xffff, p->b.l );
}

/* IRQ enter */
void m6800_cpu_device::enter_interrupt(const char *message,UINT16 irq_vector)
{
	LOG((message, tag()));
	if( m_wai_state & (M6800_WAI|M6800_SLP) )
	{
		if( m_wai_state & M6800_WAI )
			m_icount -= 4;
		m_wai_state &= ~(M6800_WAI|M6800_SLP);
	}
	else
	{
		PUSHWORD(pPC);
		PUSHWORD(pX);
		PUSHBYTE(A);
		PUSHBYTE(B);
		PUSHBYTE(CC);
		m_icount -= 12;
	}
	SEI;
	PCD = RM16( irq_vector );
}



void m6800_cpu_device::m6800_check_irq2()
{
	if ((m_tcsr & (TCSR_EICI|TCSR_ICF)) == (TCSR_EICI|TCSR_ICF))
	{
		TAKE_ICI;
		standard_irq_callback(M6801_TIN_LINE);
	}
	else if ((m_tcsr & (TCSR_EOCI|TCSR_OCF)) == (TCSR_EOCI|TCSR_OCF))
	{
		TAKE_OCI;
	}
	else if ((m_tcsr & (TCSR_ETOI|TCSR_TOF)) == (TCSR_ETOI|TCSR_TOF))
	{
		TAKE_TOI;
	}
	else if (((m_trcsr & (M6800_TRCSR_RIE|M6800_TRCSR_RDRF)) == (M6800_TRCSR_RIE|M6800_TRCSR_RDRF)) ||
				((m_trcsr & (M6800_TRCSR_RIE|M6800_TRCSR_ORFE)) == (M6800_TRCSR_RIE|M6800_TRCSR_ORFE)) ||
				((m_trcsr & (M6800_TRCSR_TIE|M6800_TRCSR_TDRE)) == (M6800_TRCSR_TIE|M6800_TRCSR_TDRE)))
	{
		//logerror("M6800 '%s' SCI interrupt\n", tag());
		TAKE_SCI;
	}
}


/* check the IRQ lines for pending interrupts */
void m6800_cpu_device::CHECK_IRQ_LINES()
{
	// TODO: IS3 interrupt

	if (m_nmi_pending)
	{
		if(m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;

		m_nmi_pending = FALSE;
		enter_interrupt("M6800 '%s' take NMI\n",0xfffc);
	}
	else
	{
		if( m_irq_state[M6800_IRQ_LINE] != CLEAR_LINE )
		{   /* standard IRQ */
			if(m_wai_state & M6800_SLP)
				m_wai_state &= ~M6800_SLP;

			if( !(CC & 0x10) )
			{
				enter_interrupt("M6800 '%s' take IRQ1\n",0xfff8);
				standard_irq_callback(M6800_IRQ_LINE);
			}
		}
		else
			if( !(CC & 0x10) )
				m6800_check_irq2();
	}
}

/* check OCI or TOI */
void m6800_cpu_device::check_timer_event()
{
	/* OCI */
	if( CTD >= OCD)
	{
		OCH++;  // next IRQ point
		m_tcsr |= TCSR_OCF;
		m_pending_tcsr |= TCSR_OCF;
		MODIFIED_tcsr;
		if((m_tcsr & TCSR_EOCI) && m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;
		if ( !(CC & 0x10) && (m_tcsr & TCSR_EOCI))
			TAKE_OCI;

		// if output on P21 is enabled, let's do it
		if (m_port2_ddr & 2)
		{
			m_port2_data &= ~2;
			m_port2_data |= (m_tcsr & TCSR_OLVL) << 1;
			m_port2_written = 1;
			write_port2();
		}
	}
	/* TOI */
	if( CTD >= TOD)
	{
		TOH++;  // next IRQ point
#if 0
		CLEANUP_COUNTERS();
#endif
		m_tcsr |= TCSR_TOF;
		m_pending_tcsr |= TCSR_TOF;
		MODIFIED_tcsr;
		if((m_tcsr & TCSR_ETOI) && m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;
		if ( !(CC & 0x10) && (m_tcsr & TCSR_ETOI))
			TAKE_TOI;
	}
	/* set next event */
	SET_TIMER_EVENT;
}

void m6800_cpu_device::increment_counter(int amount)
{
	m_icount -= amount;
	CTD += amount;
	if( CTD >= timer_next)
		check_timer_event();
}

void m6800_cpu_device::set_rmcr(UINT8 data)
{
	if (m_rmcr == data) return;

	m_rmcr = data;

	switch ((m_rmcr & M6800_RMCR_CC_MASK) >> 2)
	{
	case 0:
		m_sci_timer->enable(false);
		m_use_ext_serclock = false;
		break;

	case 3: // external clock
		m_use_ext_serclock = true;
		m_sci_timer->enable(false);
		break;

	case 1:
	case 2:
		{
			int divisor = M6800_RMCR_SS[m_rmcr & M6800_RMCR_SS_MASK];
			int clock = m_clock / m_clock_divider;

			m_sci_timer->adjust(attotime::from_hz(clock / divisor), 0, attotime::from_hz(clock / divisor));
			m_use_ext_serclock = false;
		}
		break;
	}
}

void m6800_cpu_device::write_port2()
{
	if (!m_port2_written) return;

	UINT8 data = m_port2_data;
	UINT8 ddr = m_port2_ddr & 0x1f;

	if ((ddr != 0x1f) && ddr)
	{
		data = (m_port2_data & ddr) | (ddr ^ 0xff);
	}

	if (m_trcsr & M6800_TRCSR_TE)
	{
		data = (data & 0xef) | (m_tx << 4);
	}

	data &= 0x1f;

	m_io->write_byte(M6801_PORT2, data);
}

/* include the opcode prototypes and function pointer tables */
#include "6800tbl.inc"

/* include the opcode functions */
#include "6800ops.inc"

int m6800_cpu_device::m6800_rx()
{
	return (m_io->read_byte(M6801_PORT2) & M6800_PORT2_IO3) >> 3;
}

void m6800_cpu_device::serial_transmit()
{
	//logerror("M6800 '%s' Tx Tick\n", tag());

	if (m_trcsr & M6800_TRCSR_TE)
	{
		// force Port 2 bit 4 as output
		m_port2_ddr |= M6800_PORT2_IO4;

		switch (m_txstate)
		{
		case M6800_TX_STATE_INIT:
			m_tx = 1;
			m_txbits++;

			if (m_txbits == 10)
			{
				m_txstate = M6800_TX_STATE_READY;
				m_txbits = M6800_SERIAL_START;
			}
			break;

		case M6800_TX_STATE_READY:
			switch (m_txbits)
			{
			case M6800_SERIAL_START:
				if (m_trcsr & M6800_TRCSR_TDRE)
				{
					// transmit buffer is empty, send nothing
					return;
				}
				else
				{
					// transmit buffer is full, send data

					// load TDR to shift register
					m_tsr = m_tdr;

					// transmit buffer is empty, set TDRE flag
					m_trcsr |= M6800_TRCSR_TDRE;

					// send start bit '0'
					m_tx = 0;

					m_txbits++;

					//logerror("M6800 '%s' Transmit START Data %02x\n", tag(), m_tsr);
				}
				break;

			case M6800_SERIAL_STOP:
				// send stop bit '1'
				m_tx = 1;

				CHECK_IRQ_LINES();

				m_txbits = M6800_SERIAL_START;

				//logerror("M6800 '%s' Transmit STOP\n", tag());
				break;

			default:
				// send data bit '0' or '1'
				m_tx = m_tsr & 0x01;

				// shift transmit register
				m_tsr >>= 1;

				//logerror("M6800 '%s' Transmit Bit %u: %u\n", tag(), m_txbits, m_tx);

				m_txbits++;
				break;
			}
			break;
		}

		m_out_sertx_func((m_tx == 1) ? ASSERT_LINE : CLEAR_LINE);
		m_port2_written = 1;
		write_port2();
	}
}

void m6800_cpu_device::serial_receive()
{
	//logerror("M6800 '%s' Rx Tick TRCSR %02x bits %u check %02x\n", tag(), m_trcsr, m_rxbits, m_trcsr & M6800_TRCSR_RE);

	if (m_trcsr & M6800_TRCSR_RE)
	{
		if (m_trcsr & M6800_TRCSR_WU)
		{
			// wait for 10 bits of '1'
			if (m6800_rx() == 1)
			{
				m_rxbits++;

				//logerror("M6800 '%s' Received WAKE UP bit %u\n", tag(), m_rxbits);

				if (m_rxbits == 10)
				{
					//logerror("M6800 '%s' Receiver Wake Up\n", tag());

					m_trcsr &= ~M6800_TRCSR_WU;
					m_rxbits = M6800_SERIAL_START;
				}
			}
			else
			{
				//logerror("M6800 '%s' Receiver Wake Up interrupted\n", tag());

				m_rxbits = M6800_SERIAL_START;
			}
		}
		else
		{
			// receive data
			switch (m_rxbits)
			{
			case M6800_SERIAL_START:
				if (m6800_rx() == 0)
				{
					// start bit found
					m_rxbits++;

					//logerror("M6800 '%s' Received START bit\n", tag());
				}
				break;

			case M6800_SERIAL_STOP:
				if (m6800_rx() == 1)
				{
					//logerror("M6800 '%s' Received STOP bit\n", tag());

					if (m_trcsr & M6800_TRCSR_RDRF)
					{
						// overrun error
						m_trcsr |= M6800_TRCSR_ORFE;

						//logerror("M6800 '%s' Receive Overrun Error\n", tag());

						CHECK_IRQ_LINES();
					}
					else
					{
						if (!(m_trcsr & M6800_TRCSR_ORFE))
						{
							// transfer data into receive register
							m_rdr = m_rsr;

							//logerror("M6800 '%s' Receive Data Register: %02x\n", tag(), m_rdr);

							// set RDRF flag
							m_trcsr |= M6800_TRCSR_RDRF;

							CHECK_IRQ_LINES();
						}
					}
				}
				else
				{
					// framing error
					if (!(m_trcsr & M6800_TRCSR_ORFE))
					{
						// transfer unframed data into receive register
						m_rdr = m_rsr;
					}

					m_trcsr |= M6800_TRCSR_ORFE;
					m_trcsr &= ~M6800_TRCSR_RDRF;

					//logerror("M6800 '%s' Receive Framing Error\n", tag());

					CHECK_IRQ_LINES();
				}

				m_rxbits = M6800_SERIAL_START;
				break;

			default:
				// shift receive register
				m_rsr >>= 1;

				// receive bit into register
				m_rsr |= (m6800_rx() << 7);

				//logerror("M6800 '%s' Received DATA bit %u: %u\n", tag(), m_rxbits, BIT(m_rsr, 7));

				m_rxbits++;
				break;
			}
		}
	}
}

TIMER_CALLBACK_MEMBER( m6800_cpu_device::sci_tick )
{
	serial_transmit();
	serial_receive();
}


void m6800_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_decrypted_opcodes = has_space(AS_DECRYPTED_OPCODES) ? &space(AS_DECRYPTED_OPCODES) : m_program;
	m_decrypted_opcodes_direct = &m_decrypted_opcodes->direct();
	if ( m_has_io )
		m_io = &space(AS_IO);

	m_out_sc2_func.resolve_safe();
	m_out_sertx_func.resolve_safe();
	m_sci_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m6800_cpu_device::sci_tick),this));

	m_port4_ddr = 0;
	m_port4_data = 0;
	m_input_capture = 0;
	m_rdr = 0;
	m_tdr = 0;
	m_rmcr = 0;
	m_ram_ctrl = 0;

	m_pc.d = 0;
	m_s.d = 0;
	m_x.d = 0;
	m_d.d = 0;
	m_cc = 0;
	m_wai_state = 0;
	m_irq_state[0] = m_irq_state[1] = m_irq_state[2] = 0;

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_s.w.l));
	save_item(NAME(m_x.w.l));
	save_item(NAME(m_d.w.l));
	save_item(NAME(m_cc));
	save_item(NAME(m_wai_state));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_ic_eddge));

	save_item(NAME(m_port1_ddr));
	save_item(NAME(m_port2_ddr));
	save_item(NAME(m_port3_ddr));
	save_item(NAME(m_port4_ddr));
	save_item(NAME(m_port1_data));
	save_item(NAME(m_port2_data));
	save_item(NAME(m_port3_data));
	save_item(NAME(m_port4_data));
	save_item(NAME(m_port2_written));
	save_item(NAME(m_port3_latched));
	save_item(NAME(m_p3csr));
	save_item(NAME(m_p3csr_is3_flag_read));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_pending_tcsr));
	save_item(NAME(m_irq2));
	save_item(NAME(m_ram_ctrl));

	save_item(NAME(m_counter.d));
	save_item(NAME(m_output_compare.d));
	save_item(NAME(m_input_capture));
	save_item(NAME(m_timer_over.d));

	save_item(NAME(m_clock_divider));
	save_item(NAME(m_trcsr));
	save_item(NAME(m_rmcr));
	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_rxbits));
	save_item(NAME(m_txbits));
	save_item(NAME(m_txstate));
	save_item(NAME(m_trcsr_read_tdre));
	save_item(NAME(m_trcsr_read_orfe));
	save_item(NAME(m_trcsr_read_rdrf));
	save_item(NAME(m_tx));

	state_add( M6800_A,         "A", m_d.b.h).formatstr("%02X");
	state_add( M6800_B,         "B", m_d.b.l).formatstr("%02X");
	state_add( M6800_PC,        "PC", m_pc.w.l).formatstr("%04X");
	state_add( M6800_S,         "S", m_s.w.l).formatstr("%04X");
	state_add( M6800_X,         "X", m_x.w.l).formatstr("%04X");
	state_add( M6800_CC,        "CC", m_cc).formatstr("%02X");
	state_add( M6800_WAI_STATE, "WAI", m_wai_state).formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_pc.w.l).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_cc).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}

void m6800_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_cc & 0x80 ? '?':'.',
				m_cc & 0x40 ? '?':'.',
				m_cc & 0x20 ? 'H':'.',
				m_cc & 0x10 ? 'I':'.',
				m_cc & 0x08 ? 'N':'.',
				m_cc & 0x04 ? 'Z':'.',
				m_cc & 0x02 ? 'V':'.',
				m_cc & 0x01 ? 'C':'.');
			break;
	}
}

void m6800_cpu_device::device_reset()
{
	m_cc = 0xc0;
	SEI;                /* IRQ disabled */
	PCD = RM16( 0xfffe );

	m_wai_state = 0;
	m_nmi_state = 0;
	m_nmi_pending = 0;
	m_sc1_state = 0;
	m_irq_state[M6800_IRQ_LINE] = 0;
	m_irq_state[M6801_TIN_LINE] = 0;
	m_ic_eddge = 0;

	m_port1_ddr = 0x00;
	m_port2_ddr = 0x00;
	m_port3_ddr = 0x00;
	m_port1_data = 0;
	m_p3csr = 0x00;
	m_p3csr_is3_flag_read = 0;
	m_port2_written = 0;
	m_port3_latched = 0;
	/* TODO: on reset port 2 should be read to determine the operating mode (bits 0-2) */
	m_tcsr = 0x00;
	m_pending_tcsr = 0x00;
	m_irq2 = 0;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	m_ram_ctrl |= 0x40;
	m_latch09 = 0;

	m_trcsr = M6800_TRCSR_TDRE;

	m_txstate = M6800_TX_STATE_INIT;
	m_txbits = m_rxbits = 0;
	m_tx = 1;
	m_trcsr_read_tdre = 0;
	m_trcsr_read_orfe = 0;
	m_trcsr_read_rdrf = 0;
	m_ext_serclock = 0;
	m_use_ext_serclock = false;

	set_rmcr(0);
}


void m6800_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_NMI:
		if (!m_nmi_state && state != CLEAR_LINE)
			m_nmi_pending = TRUE;
		m_nmi_state = state;
		break;

	case M6801_SC1_LINE:
		if (!m_port3_latched && (m_p3csr & M6801_P3CSR_LE))
		{
			if (!m_sc1_state && state)
			{
				// latch input data to port 3
				m_port3_data = (m_io->read_byte(M6801_PORT3) & (m_port3_ddr ^ 0xff)) | (m_port3_data & m_port3_ddr);
				m_port3_latched = 1;
				//logerror("M6801 '%s' Latched Port 3 Data: %02x\n", tag(), m_port3_data);

				// set IS3 flag bit
				m_p3csr |= M6801_P3CSR_IS3_FLAG;
			}
		}
		m_sc1_state = state;
		break;

	default:
		LOG(("M6800 '%s' set_irq_line %d,%d\n", tag(), irqline, state));
		m_irq_state[irqline] = state;

		if (irqline == M6801_TIN_LINE && state != m_irq_state[irqline])
		{
			//eddge = (state == CLEAR_LINE ) ? 2 : 0;
			if( ((m_tcsr&TCSR_IEDG) ^ (state==CLEAR_LINE ? TCSR_IEDG : 0))==0 )
				return;
			/* active edge in */
			m_tcsr |= TCSR_ICF;
			m_pending_tcsr |= TCSR_ICF;
			m_input_capture = CT;
			MODIFIED_tcsr;
		}
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
void m6800_cpu_device::execute_run()
{
	UINT8 ireg;

	CHECK_IRQ_LINES(); /* HJB 990417 */

	CLEANUP_COUNTERS();

	do
	{
		if( m_wai_state & (M6800_WAI|M6800_SLP) )
		{
			EAT_CYCLES;
		}
		else
		{
			pPPC = pPC;
			debugger_instruction_hook(this, PCD);
			ireg=M_RDOP(PCD);
			PC++;
			(this->*m_insn[ireg])();
			increment_counter(m_cycles[ireg]);
		}
	} while( m_icount>0 );
}


/*
    if change_pc() direccted these areas ,Call hd63701_trap_pc().
    'mode' is selected by the sense of p2.0,p2.1,and p2.3 at reset timming.
    mode 0,1,2,4,6 : $0000-$001f
    mode 5         : $0000-$001f,$0200-$efff
    mode 7         : $0000-$001f,$0100-$efff
*/
#if 0
void m6800_cpu_device::hd63701_trap_pc()
{
	TAKE_TRAP;
}
#endif

void m6800_cpu_device::set_os3(int state)
{
	//logerror("M6801 '%s' OS3: %u\n", tag(), state);

	m_out_sc2_func(state);
}

READ8_MEMBER( m6800_cpu_device::m6801_io_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case IO_P1DDR:
		data = m_port1_ddr;
		break;

	case IO_P2DDR:
		data = m_port2_ddr;
		break;

	case IO_P1DATA:
		if(m_port1_ddr == 0xff)
			data = m_port1_data;
		else
			data = (m_io->read_byte(M6801_PORT1) & (m_port1_ddr ^ 0xff))
				| (m_port1_data & m_port1_ddr);
		break;

	case IO_P2DATA:
		if(m_port2_ddr == 0xff)
			data = m_port2_data;
		else
			data = (m_io->read_byte(M6801_PORT2) & (m_port2_ddr ^ 0xff))
				| (m_port2_data & m_port2_ddr);
		break;

	case IO_P3DDR:
		logerror("M6801 '%s' Port 3 DDR is a write-only register\n", space.device().tag());
		break;

	case IO_P4DDR:
		data = m_port4_ddr;
		break;

	case IO_P3DATA:
		if (!space.debugger_access())
		{
			if (m_p3csr_is3_flag_read)
			{
				//logerror("M6801 '%s' Cleared IS3\n", space.device().tag());
				m_p3csr &= ~M6801_P3CSR_IS3_FLAG;
				m_p3csr_is3_flag_read = 0;
			}

			if (!(m_p3csr & M6801_P3CSR_OSS))
			{
				set_os3(ASSERT_LINE);
			}
		}

		if ((m_p3csr & M6801_P3CSR_LE) || (m_port3_ddr == 0xff))
			data = m_port3_data;
		else
			data = (m_io->read_byte(M6801_PORT3) & (m_port3_ddr ^ 0xff))
				| (m_port3_data & m_port3_ddr);

		if (!space.debugger_access())
		{
			m_port3_latched = 0;

			if (!(m_p3csr & M6801_P3CSR_OSS))
			{
				set_os3(CLEAR_LINE);
			}
		}
		break;

	case IO_P4DATA:
		if(m_port4_ddr == 0xff)
			data = m_port4_data;
		else
			data = (m_io->read_byte(M6801_PORT4) & (m_port4_ddr ^ 0xff))
				| (m_port4_data & m_port4_ddr);
		break;

	case IO_TCSR:
		m_pending_tcsr = 0;
		data = m_tcsr;
		break;

	case IO_CH:
		if(!(m_pending_tcsr&TCSR_TOF) && !space.debugger_access())
		{
			m_tcsr &= ~TCSR_TOF;
			MODIFIED_tcsr;
		}
		data = m_counter.b.h;
		break;

	case IO_CL:
		data = m_counter.b.l;
		// HACK there should be a break here, but Coleco Adam won't boot with it present, proper fix required to the free-running counter

	case IO_OCRH:
		if(!(m_pending_tcsr&TCSR_OCF) && !space.debugger_access())
		{
			m_tcsr &= ~TCSR_OCF;
			MODIFIED_tcsr;
		}
		data = m_output_compare.b.h;
		break;

	case IO_OCRL:
		if(!(m_pending_tcsr&TCSR_OCF) && !space.debugger_access())
		{
			m_tcsr &= ~TCSR_OCF;
			MODIFIED_tcsr;
		}
		data = m_output_compare.b.l;
		break;

	case IO_ICRH:
		if(!(m_pending_tcsr&TCSR_ICF) && !space.debugger_access())
		{
			m_tcsr &= ~TCSR_ICF;
			MODIFIED_tcsr;
		}
		data = (m_input_capture >> 0) & 0xff;
		break;

	case IO_ICRL:
		data = (m_input_capture >> 8) & 0xff;
		break;

	case IO_P3CSR:
		if ((m_p3csr & M6801_P3CSR_IS3_FLAG) && !space.debugger_access())
		{
			m_p3csr_is3_flag_read = 1;
		}

		data = m_p3csr;
		break;

	case IO_RMCR:
		data = m_rmcr;
		break;

	case IO_TRCSR:
		if (!space.debugger_access())
		{
			if (m_trcsr & M6800_TRCSR_TDRE)
			{
				m_trcsr_read_tdre = 1;
			}

			if (m_trcsr & M6800_TRCSR_ORFE)
			{
				m_trcsr_read_orfe = 1;
			}

			if (m_trcsr & M6800_TRCSR_RDRF)
			{
				m_trcsr_read_rdrf = 1;
			}
		}

		data = m_trcsr;
		break;

	case IO_RDR:
		if (!space.debugger_access())
		{
			if (m_trcsr_read_orfe)
			{
				//logerror("M6801 '%s' Cleared ORFE\n", space.device().tag());
				m_trcsr_read_orfe = 0;
				m_trcsr &= ~M6800_TRCSR_ORFE;
			}

			if (m_trcsr_read_rdrf)
			{
				//logerror("M6801 '%s' Cleared RDRF\n", space.device().tag());
				m_trcsr_read_rdrf = 0;
				m_trcsr &= ~M6800_TRCSR_RDRF;
			}
		}

		data = m_rdr;
		break;

	case IO_TDR:
		data = m_tdr;
		break;

	case IO_RCR:
		data = m_ram_ctrl;
		break;

	case IO_CAAH:
	case IO_CAAL:
	case IO_TCR1:
	case IO_TCR2:
	case IO_TSR:
	case IO_OCR2H:
	case IO_OCR2L:
	case IO_OCR3H:
	case IO_OCR3L:
	case IO_ICR2H:
	case IO_ICR2L:
	default:
		logerror("M6801 '%s' PC %04x: warning - read from reserved internal register %02x\n",space.device().tag(),space.device().safe_pc(),offset);
	}

	return data;
}

WRITE8_MEMBER( m6800_cpu_device::m6801_io_w )
{
	switch (offset)
	{
	case IO_P1DDR:
		//logerror("M6801 '%s' Port 1 Data Direction Register: %02x\n", space.device().tag(), data);

		if (m_port1_ddr != data)
		{
			m_port1_ddr = data;
			if(m_port1_ddr == 0xff)
				m_io->write_byte(M6801_PORT1,m_port1_data);
			else
				m_io->write_byte(M6801_PORT1,(m_port1_data & m_port1_ddr) | (m_port1_ddr ^ 0xff));
		}
		break;

	case IO_P2DDR:
		//logerror("M6801 '%s' Port 2 Data Direction Register: %02x\n", space.device().tag(), data);

		if (m_port2_ddr != data)
		{
			m_port2_ddr = data;
			write_port2();
		}
		break;

	case IO_P1DATA:
		//logerror("M6801 '%s' Port 1 Data Register: %02x\n", space.device().tag(), data);

		m_port1_data = data;
		if(m_port1_ddr == 0xff)
			m_io->write_byte(M6801_PORT1,m_port1_data);
		else
			m_io->write_byte(M6801_PORT1,(m_port1_data & m_port1_ddr) | (m_port1_ddr ^ 0xff));
		break;

	case IO_P2DATA:
		//logerror("M6801 '%s' Port 2 Data Register: %02x\n", space.device().tag(), data);

		m_port2_data = data;
		m_port2_written = 1;
		write_port2();
		break;

	case IO_P3DDR:
		//logerror("M6801 '%s' Port 3 Data Direction Register: %02x\n", space.device().tag(), data);

		if (m_port3_ddr != data)
		{
			m_port3_ddr = data;
			if(m_port3_ddr == 0xff)
				m_io->write_byte(M6801_PORT3,m_port3_data);
			else
				m_io->write_byte(M6801_PORT3,(m_port3_data & m_port3_ddr) | (m_port3_ddr ^ 0xff));
		}
		break;

	case IO_P4DDR:
		//logerror("M6801 '%s' Port 4 Data Direction Register: %02x\n", space.device().tag(), data);

		if (m_port4_ddr != data)
		{
			m_port4_ddr = data;
			if(m_port4_ddr == 0xff)
				m_io->write_byte(M6801_PORT4,m_port4_data);
			else
				m_io->write_byte(M6801_PORT4,(m_port4_data & m_port4_ddr) | (m_port4_ddr ^ 0xff));
		}
		break;

	case IO_P3DATA:
		//logerror("M6801 '%s' Port 3 Data Register: %02x\n", space.device().tag(), data);

		if (m_p3csr_is3_flag_read)
		{
			//logerror("M6801 '%s' Cleared IS3\n", space.device().tag());
			m_p3csr &= ~M6801_P3CSR_IS3_FLAG;
			m_p3csr_is3_flag_read = 0;
		}

		if (m_p3csr & M6801_P3CSR_OSS)
		{
			set_os3(ASSERT_LINE);
		}

		m_port3_data = data;
		if(m_port3_ddr == 0xff)
			m_io->write_byte(M6801_PORT3,m_port3_data);
		else
			m_io->write_byte(M6801_PORT3,(m_port3_data & m_port3_ddr) | (m_port3_ddr ^ 0xff));

		if (m_p3csr & M6801_P3CSR_OSS)
		{
			set_os3(CLEAR_LINE);
		}
		break;

	case IO_P4DATA:
		//logerror("M6801 '%s' Port 4 Data Register: %02x\n", space.device().tag(), data);

		m_port4_data = data;
		if(m_port4_ddr == 0xff)
			m_io->write_byte(M6801_PORT4,m_port4_data);
		else
			m_io->write_byte(M6801_PORT4,(m_port4_data & m_port4_ddr) | (m_port4_ddr ^ 0xff));
		break;

	case IO_TCSR:
		//logerror("M6801 '%s' Timer Control and Status Register: %02x\n", space.device().tag(), data);

		m_tcsr = data;
		m_pending_tcsr &= m_tcsr;
		MODIFIED_tcsr;
		if( !(CC & 0x10) )
			m6800_check_irq2();
		break;

	case IO_CH:
		//logerror("M6801 '%s' Counter High Register: %02x\n", space.device().tag(), data);

		m_latch09 = data & 0xff;    /* 6301 only */
		CT  = 0xfff8;
		TOH = CTH;
		MODIFIED_counters;
		break;

	case IO_CL: /* 6301 only */
		//logerror("M6801 '%s' Counter Low Register: %02x\n", space.device().tag(), data);

		CT = (m_latch09 << 8) | (data & 0xff);
		TOH = CTH;
		MODIFIED_counters;
		break;

	case IO_OCRH:
		//logerror("M6801 '%s' Output Compare High Register: %02x\n", space.device().tag(), data);

		if( m_output_compare.b.h != data)
		{
			m_output_compare.b.h = data;
			MODIFIED_counters;
		}
		break;

	case IO_OCRL:
		//logerror("M6801 '%s' Output Compare Low Register: %02x\n", space.device().tag(), data);

		if( m_output_compare.b.l != data)
		{
			m_output_compare.b.l = data;
			MODIFIED_counters;
		}
		break;

	case IO_ICRH:
	case IO_ICRL:
	case IO_RDR:
		//logerror("CPU '%s' PC %04x: warning - write %02x to read only internal register %02x\n",space.device().tag(),space.device().safe_pc(),data,offset);
		break;

	case IO_P3CSR:
		//logerror("M6801 '%s' Port 3 Control and Status Register: %02x\n", space.device().tag(), data);

		m_p3csr = data;
		break;

	case IO_RMCR:
		//logerror("M6801 '%s' Rate and Mode Control Register: %02x\n", space.device().tag(), data);

		set_rmcr(data);
		break;

	case IO_TRCSR:
		//logerror("M6801 '%s' Transmit/Receive Control and Status Register: %02x\n", space.device().tag(), data);

		if ((data & M6800_TRCSR_TE) && !(m_trcsr & M6800_TRCSR_TE))
		{
			m_txstate = M6800_TX_STATE_INIT;
			m_txbits = 0;
			m_tx = 1;
		}

		if ((data & M6800_TRCSR_RE) && !(m_trcsr & M6800_TRCSR_RE))
		{
			m_rxbits = 0;
		}

		m_trcsr = (m_trcsr & 0xe0) | (data & 0x1f);
		break;

	case IO_TDR:
		//logerror("M6800 '%s' Transmit Data Register: %02x\n", space.device().tag(), data);

		if (m_trcsr_read_tdre)
		{
			m_trcsr_read_tdre = 0;
			m_trcsr &= ~M6800_TRCSR_TDRE;
		}
		m_tdr = data;
		break;

	case IO_RCR:
		//logerror("M6801 '%s' RAM Control Register: %02x\n", space.device().tag(), data);

		m_ram_ctrl = data;
		break;

	case IO_CAAH:
	case IO_CAAL:
	case IO_TCR1:
	case IO_TCR2:
	case IO_TSR:
	case IO_OCR2H:
	case IO_OCR2L:
	case IO_OCR3H:
	case IO_OCR3L:
	case IO_ICR2H:
	case IO_ICR2L:
	default:
		logerror("M6801 '%s' PC %04x: warning - write %02x to reserved internal register %02x\n",space.device().tag(),space.device().safe_pc(),data,offset);
		break;
	}
}

void m6801_cpu_device::m6801_clock_serial()
{
	if (m_use_ext_serclock)
	{
		m_ext_serclock++;

		if (m_ext_serclock >= 8)
		{
			m_ext_serclock = 0;
			serial_transmit();
			serial_receive();
		}
	}
}

offs_t m6800_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( m6800 );
	return CPU_DISASSEMBLE_NAME(m6800)(this, buffer, pc, oprom, opram, options);
}


offs_t m6801_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( m6801 );
	return CPU_DISASSEMBLE_NAME(m6801)(this, buffer, pc, oprom, opram, options);
}


offs_t m6802_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( m6802 );
	return CPU_DISASSEMBLE_NAME(m6802)(this, buffer, pc, oprom, opram, options);
}


offs_t m6803_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( m6803 );
	return CPU_DISASSEMBLE_NAME(m6803)(this, buffer, pc, oprom, opram, options);
}


offs_t m6808_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( m6808 );
	return CPU_DISASSEMBLE_NAME(m6808)(this, buffer, pc, oprom, opram, options);
}


offs_t hd6301_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hd6301 );
	return CPU_DISASSEMBLE_NAME(hd6301)(this, buffer, pc, oprom, opram, options);
}


offs_t hd63701_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hd63701 );
	return CPU_DISASSEMBLE_NAME(hd63701)(this, buffer, pc, oprom, opram, options);
}


offs_t nsc8105_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( nsc8105 );
	return CPU_DISASSEMBLE_NAME(nsc8105)(this, buffer, pc, oprom, opram, options);
}

WRITE_LINE_MEMBER( m6800_cpu_device::irq_line )
{
	set_input_line( M6800_IRQ_LINE, state );
}

WRITE_LINE_MEMBER( m6800_cpu_device::nmi_line )
{
	set_input_line( INPUT_LINE_NMI, state );
}
