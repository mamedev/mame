/*** m6800: Portable 6800 class  emulator *************************************

    m68xx.c

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

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

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

/* 6800 Registers */
typedef struct _m6800_state m6800_state;
struct _m6800_state
{
//  int     subtype;        /* CPU subtype */
	PAIR	ppc;			/* Previous program counter */
	PAIR	pc; 			/* Program counter */
	PAIR	s;				/* Stack pointer */
	PAIR	x;				/* Index register */
	PAIR	d;				/* Accumulators */
	UINT8	cc; 			/* Condition codes */
	UINT8	wai_state;		/* WAI opcode state ,(or sleep opcode state) */
	UINT8	nmi_state;		/* NMI line state */
	UINT8	nmi_pending;	/* NMI pending */
	UINT8	irq_state[3];	/* IRQ line state [IRQ1,TIN,SC1] */
	UINT8	ic_eddge;		/* InputCapture eddge , b.0=fall,b.1=raise */
	int		sc1_state;

	device_irq_callback irq_callback;
	legacy_cpu_device *device;

	/* Memory spaces */
    address_space *program;
    direct_read_data *direct;
    address_space *data;
    address_space *io;

	void	(* const * insn)(m6800_state *);	/* instruction table */
	const UINT8 *cycles;			/* clock cycle of instruction table */
	/* internal registers */
	UINT8	port1_ddr;
	UINT8	port2_ddr;
	UINT8	port3_ddr;
	UINT8	port4_ddr;
	UINT8	port1_data;
	UINT8	port2_data;
	UINT8	port3_data;
	UINT8	port4_data;
	UINT8	p3csr;			// Port 3 Control/Status Register
	UINT8	tcsr;			/* Timer Control and Status Register */
	UINT8	pending_tcsr;	/* pending IRQ flag for clear IRQflag process */
	UINT8	irq2;			/* IRQ2 flags */
	UINT8	ram_ctrl;
	PAIR	counter;		/* free running counter */
	PAIR	output_compare;	/* output compare       */
	UINT16	input_capture;	/* input capture        */
	int		p3csr_is3_flag_read;
	int		port3_latched;

	int		clock;
	UINT8	trcsr, rmcr, rdr, tdr, rsr, tsr;
	int		rxbits, txbits, txstate, trcsr_read_tdre, trcsr_read_orfe, trcsr_read_rdrf, tx;
	int		port2_written;

	int		icount;
	int		latch09;

	PAIR	timer_over;
	emu_timer *sci_timer;
	PAIR ea;		/* effective address */

	devcb_resolved_write_line	out_sc2_func;
};

INLINE m6800_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == M6800 ||
		   device->type() == M6801 ||
		   device->type() == M6802 ||
		   device->type() == M6803 ||
		   device->type() == M6808 ||
		   device->type() == HD6301 ||
		   device->type() == HD63701 ||
		   device->type() == NSC8105);
	return (m6800_state *)downcast<legacy_cpu_device *>(device)->token();
}

#if 0
static void hd63701_trap_pc(m6800_state *cpustate);
#endif

#define	pPPC	cpustate->ppc
#define pPC 	cpustate->pc
#define pS		cpustate->s
#define pX		cpustate->x
#define pD		cpustate->d

#define PC		cpustate->pc.w.l
#define PCD		cpustate->pc.d
#define S		cpustate->s.w.l
#define SD		cpustate->s.d
#define X		cpustate->x.w.l
#define D		cpustate->d.w.l
#define A		cpustate->d.b.h
#define B		cpustate->d.b.l
#define CC		cpustate->cc

#define CT		cpustate->counter.w.l
#define CTH		cpustate->counter.w.h
#define CTD		cpustate->counter.d
#define OC		cpustate->output_compare.w.l
#define OCH		cpustate->output_compare.w.h
#define OCD		cpustate->output_compare.d
#define TOH		cpustate->timer_over.w.l
#define TOD		cpustate->timer_over.d

#define EAD 	cpustate->ea.d
#define EA		cpustate->ea.w.l

/* point of next timer event */
static UINT32 timer_next;

/* memory interface */

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(Addr) ((unsigned)cpustate->program->read_byte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (cpustate->program->write_byte(Addr,Value))

/****************************************************************************/
/* M6800_RDOP() is identical to M6800_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M_RDOP(Addr) ((unsigned)cpustate->direct->read_decrypted_byte(Addr))

/****************************************************************************/
/* M6800_RDOP_ARG() is identical to M6800_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M_RDOP_ARG(Addr) ((unsigned)cpustate->direct->read_raw_byte(Addr))

/* macros to access memory */
#define IMMBYTE(b)	b = M_RDOP_ARG(PCD); PC++
#define IMMWORD(w)	w.d = (M_RDOP_ARG(PCD)<<8) | M_RDOP_ARG((PCD+1)&0xffff); PC+=2

#define PUSHBYTE(b) WM(SD,b); --S
#define PUSHWORD(w) WM(SD,w.b.l); --S; WM(SD,w.b.h); --S
#define PULLBYTE(b) S++; b = RM(SD)
#define PULLWORD(w) S++; w.d = RM(SD)<<8; S++; w.d |= RM(SD)

#define MODIFIED_tcsr {	\
	cpustate->irq2 = (cpustate->tcsr&(cpustate->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF); \
}

#define SET_TIMER_EVENT {					\
	timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;	\
}

/* cleanup high-word of counters */
#define CLEANUP_COUNTERS() {						\
	OCH -= CTH;									\
	TOH -= CTH;									\
	CTH = 0;									\
	SET_TIMER_EVENT;							\
}

/* when change freerunningcounter or outputcapture */
#define MODIFIED_counters {						\
	OCH = (OC >= CT) ? CTH : CTH+1;				\
	SET_TIMER_EVENT;							\
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

#define M6800_RMCR_SS_MASK		0x03 // Speed Select
#define M6800_RMCR_SS_4096		0x03 // E / 4096
#define M6800_RMCR_SS_1024		0x02 // E / 1024
#define M6800_RMCR_SS_128		0x01 // E / 128
#define M6800_RMCR_SS_16		0x00 // E / 16
#define M6800_RMCR_CC_MASK		0x0c // Clock Control/Format Select

#define M6800_TRCSR_RDRF		0x80 // Receive Data Register Full
#define M6800_TRCSR_ORFE		0x40 // Over Run Framing Error
#define M6800_TRCSR_TDRE		0x20 // Transmit Data Register Empty
#define M6800_TRCSR_RIE			0x10 // Receive Interrupt Enable
#define M6800_TRCSR_RE			0x08 // Receive Enable
#define M6800_TRCSR_TIE			0x04 // Transmit Interrupt Enable
#define M6800_TRCSR_TE			0x02 // Transmit Enable
#define M6800_TRCSR_WU			0x01 // Wake Up

#define M6800_PORT2_IO4			0x10
#define M6800_PORT2_IO3			0x08

#define M6801_P3CSR_LE			0x08
#define M6801_P3CSR_OSS			0x10
#define M6801_P3CSR_IS3_ENABLE	0x40
#define M6801_P3CSR_IS3_FLAG	0x80

static const int M6800_RMCR_SS[] = { 16, 128, 1024, 4096 };

#define M6800_SERIAL_START		0
#define M6800_SERIAL_STOP		9

enum
{
	M6800_TX_STATE_INIT = 0,
	M6800_TX_STATE_READY
};

/* take interrupt */
#define TAKE_ICI enter_interrupt(cpustate, "M6800 '%s' take ICI\n",0xfff6)
#define TAKE_OCI enter_interrupt(cpustate, "M6800 '%s' take OCI\n",0xfff4)
#define TAKE_TOI enter_interrupt(cpustate, "M6800 '%s' take TOI\n",0xfff2)
#define TAKE_SCI enter_interrupt(cpustate, "M6800 '%s' take SCI\n",0xfff0)
#define TAKE_TRAP enter_interrupt(cpustate, "M6800 '%s' take TRAP\n",0xffee)

/* operate one instruction for */
#define ONE_MORE_INSN() {		\
	UINT8 ireg; 							\
	pPPC = pPC; 							\
	debugger_instruction_hook(cpustate->device, PCD);						\
	ireg=M_RDOP(PCD);						\
	PC++;									\
	(*cpustate->insn[ireg])(cpustate);					\
	increment_counter(cpustate, cpustate->cycles[ireg]);	\
}

/* CC masks                       HI NZVC
                                7654 3210   */
#define CLR_HNZVC	CC&=0xd0
#define CLR_NZV 	CC&=0xf1
#define CLR_HNZC	CC&=0xd2
#define CLR_NZVC	CC&=0xf0
#define CLR_Z		CC&=0xfb
#define CLR_NZC 	CC&=0xf2
#define CLR_ZC		CC&=0xfa
#define CLR_C		CC&=0xfe

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)		if(!(a))SEZ
#define SET_Z8(a)		SET_Z((UINT8)(a))
#define SET_Z16(a)		SET_Z((UINT16)(a))
#define SET_N8(a)		CC|=(((a)&0x80)>>4)
#define SET_N16(a)		CC|=(((a)&0x8000)>>12)
#define SET_H(a,b,r)	CC|=((((a)^(b)^(r))&0x10)<<1)
#define SET_C8(a)		CC|=(((a)&0x100)>>8)
#define SET_C16(a)		CC|=(((a)&0x10000)>>16)
#define SET_V8(a,b,r)	CC|=((((a)^(b)^(r)^((r)>>1))&0x80)>>6)
#define SET_V16(a,b,r)	CC|=((((a)^(b)^(r)^((r)>>1))&0x8000)>>14)

static const UINT8 flags8i[256]=	 /* increment */
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
static const UINT8 flags8d[256]= /* decrement */
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
#define SET_FLAGS8I(a)		{CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z8(a);}
#define SET_NZ16(a)			{SET_N16(a);SET_Z16(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

/* for treating an UINT8 as a signed INT16 */
#define SIGNED(b) ((INT16)(b&0x80?b|0xff00:b))

/* Macros for addressing modes */
#define DIRECT IMMBYTE(EAD)
#define IMM8 EA=PC++
#define IMM16 {EA=PC;PC+=2;}
#define EXTENDED IMMWORD(cpustate->ea)
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
#define DIRWORD(w) {DIRECT;w.d=RM16(cpustate, EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(cpustate, EAD);}

#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDXWORD(w) {INDEXED;w.d=RM16(cpustate, EAD);}

/* Macros for branch instructions */
#define BRANCH(f) {IMMBYTE(t);if(f){PC+=SIGNED(t);}}
#define NXORV  ((CC&0x08)^((CC&0x02)<<2))

#define M6800_WAI		8			/* set when WAI is waiting for an interrupt */
#define M6800_SLP		0x10		/* HD63701 only */

/* Note: don't use 0 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */
#define XX 5 // invalid opcode unknown cc
static const UINT8 cycles_6800[] =
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

static const UINT8 cycles_6803[] =
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

static const UINT8 cycles_63701[] =
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

static const UINT8 cycles_nsc8105[] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX,XX, 2,XX,XX, 2,XX, 2, 4, 2, 4, 2, 2, 2, 2, 2,
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
	/*B*/  6,XX,XX, 6, 6, 6,XX, 6, 6, 6, 6,XX, 6, 3, 6, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX, 3,XX, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX, 4,XX, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 5, 6,XX, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5,XX, 6
};
#undef XX // /invalid opcode unknown cc

#define EAT_CYCLES													\
{																	\
	int cycles_to_eat;												\
																	\
	cycles_to_eat = timer_next - CTD;								\
	if( cycles_to_eat > cpustate->icount) cycles_to_eat = cpustate->icount;	\
	if (cycles_to_eat > 0)											\
	{																\
		increment_counter(cpustate, cycles_to_eat);							\
	}																\
}

INLINE UINT32 RM16(m6800_state *cpustate, UINT32 Addr )
{
	UINT32 result = RM(Addr) << 8;
	return result | RM((Addr+1)&0xffff);
}

INLINE void WM16(m6800_state *cpustate, UINT32 Addr, PAIR *p )
{
	WM( Addr, p->b.h );
	WM( (Addr+1)&0xffff, p->b.l );
}

/* IRQ enter */
static void enter_interrupt(m6800_state *cpustate, const char *message,UINT16 irq_vector)
{
	LOG((message, cpustate->device->tag()));
	if( cpustate->wai_state & (M6800_WAI|M6800_SLP) )
	{
		if( cpustate->wai_state & M6800_WAI )
			cpustate->icount -= 4;
		cpustate->wai_state &= ~(M6800_WAI|M6800_SLP);
	}
	else
	{
		PUSHWORD(pPC);
		PUSHWORD(pX);
		PUSHBYTE(A);
		PUSHBYTE(B);
		PUSHBYTE(CC);
		cpustate->icount -= 12;
	}
	SEI;
	PCD = RM16(cpustate,  irq_vector );
}



static void m6800_check_irq2(m6800_state *cpustate)
{
	if ((cpustate->tcsr & (TCSR_EICI|TCSR_ICF)) == (TCSR_EICI|TCSR_ICF))
	{
		TAKE_ICI;
		if( cpustate->irq_callback )
			(void)(*cpustate->irq_callback)(cpustate->device, M6801_TIN_LINE);
	}
	else if ((cpustate->tcsr & (TCSR_EOCI|TCSR_OCF)) == (TCSR_EOCI|TCSR_OCF))
	{
		TAKE_OCI;
	}
	else if ((cpustate->tcsr & (TCSR_ETOI|TCSR_TOF)) == (TCSR_ETOI|TCSR_TOF))
	{
		TAKE_TOI;
	}
	else if (((cpustate->trcsr & (M6800_TRCSR_RIE|M6800_TRCSR_RDRF)) == (M6800_TRCSR_RIE|M6800_TRCSR_RDRF)) ||
			 ((cpustate->trcsr & (M6800_TRCSR_RIE|M6800_TRCSR_ORFE)) == (M6800_TRCSR_RIE|M6800_TRCSR_ORFE)) ||
			 ((cpustate->trcsr & (M6800_TRCSR_TIE|M6800_TRCSR_TDRE)) == (M6800_TRCSR_TIE|M6800_TRCSR_TDRE)))
	{
		//logerror("M6800 '%s' SCI interrupt\n", cpustate->device->tag());
		TAKE_SCI;
	}
}


/* check the IRQ lines for pending interrupts */
INLINE void CHECK_IRQ_LINES(m6800_state *cpustate)
{
	// TODO: IS3 interrupt

	if (cpustate->nmi_pending)
	{
		if(cpustate->wai_state & M6800_SLP)
			cpustate->wai_state &= ~M6800_SLP;

		cpustate->nmi_pending = FALSE;
		enter_interrupt(cpustate, "M6800 '%s' take NMI\n",0xfffc);
	}
	else
	{
		if( cpustate->irq_state[M6800_IRQ_LINE] != CLEAR_LINE )
		{	/* standard IRQ */
			if(cpustate->wai_state & M6800_SLP)
				cpustate->wai_state &= ~M6800_SLP;

			if( !(CC & 0x10) )
			{
				enter_interrupt(cpustate, "M6800 '%s' take IRQ1\n",0xfff8);
				if( cpustate->irq_callback )
					(void)(*cpustate->irq_callback)(cpustate->device, M6800_IRQ_LINE);
			}
		}
		else
			if( !(CC & 0x10) )
				m6800_check_irq2(cpustate);
	}
}

/* check OCI or TOI */
static void check_timer_event(m6800_state *cpustate)
{
	/* OCI */
	if( CTD >= OCD)
	{
		OCH++;	// next IRQ point
		cpustate->tcsr |= TCSR_OCF;
		cpustate->pending_tcsr |= TCSR_OCF;
		MODIFIED_tcsr;
		if((cpustate->tcsr & TCSR_EOCI) && cpustate->wai_state & M6800_SLP)
			cpustate->wai_state &= ~M6800_SLP;
		if ( !(CC & 0x10) && (cpustate->tcsr & TCSR_EOCI))
			TAKE_OCI;
	}
	/* TOI */
	if( CTD >= TOD)
	{
		TOH++;	// next IRQ point
#if 0
		CLEANUP_COUNTERS();
#endif
		cpustate->tcsr |= TCSR_TOF;
		cpustate->pending_tcsr |= TCSR_TOF;
		MODIFIED_tcsr;
		if((cpustate->tcsr & TCSR_ETOI) && cpustate->wai_state & M6800_SLP)
			cpustate->wai_state &= ~M6800_SLP;
		if ( !(CC & 0x10) && (cpustate->tcsr & TCSR_ETOI))
			TAKE_TOI;
	}
	/* set next event */
	SET_TIMER_EVENT;
}

INLINE void increment_counter(m6800_state *cpustate, int amount)
{
	cpustate->icount -= amount;
	CTD += amount;
	if( CTD >= timer_next)
		check_timer_event(cpustate);
}

INLINE void set_rmcr(m6800_state *cpustate, UINT8 data)
{
	if (cpustate->rmcr == data) return;

	cpustate->rmcr = data;

	switch ((cpustate->rmcr & M6800_RMCR_CC_MASK) >> 2)
	{
	case 0:
	case 3: // not implemented
		cpustate->sci_timer->enable(false);
		break;

	case 1:
	case 2:
		{
			int divisor = M6800_RMCR_SS[cpustate->rmcr & M6800_RMCR_SS_MASK];

			cpustate->sci_timer->adjust(attotime::from_hz(cpustate->clock / divisor), 0, attotime::from_hz(cpustate->clock / divisor));
		}
		break;
	}
}

INLINE void write_port2(m6800_state *cpustate)
{
	if (!cpustate->port2_written) return;

	UINT8 data = cpustate->port2_data;
	UINT8 ddr = cpustate->port2_ddr & 0x1f;

	if ((ddr != 0x1f) && ddr)
	{
		data = (cpustate->port2_data & ddr)	| (ddr ^ 0xff);
	}

	if (cpustate->trcsr & M6800_TRCSR_TE)
	{
		data = (data & 0xef) | (cpustate->tx << 4);
	}

	data &= 0x1f;

	cpustate->io->write_byte(M6801_PORT2, data);
}

/* include the opcode prototypes and function pointer tables */
#include "6800tbl.c"

/* include the opcode functions */
#include "6800ops.c"

static int m6800_rx(m6800_state *cpustate)
{
	return (cpustate->io->read_byte(M6801_PORT2) & M6800_PORT2_IO3) >> 3;
}

static void serial_transmit(m6800_state *cpustate)
{
	//logerror("M6800 '%s' Tx Tick\n", cpustate->device->tag());

	if (cpustate->trcsr & M6800_TRCSR_TE)
	{
		// force Port 2 bit 4 as output
		cpustate->port2_ddr |= M6800_PORT2_IO4;

		switch (cpustate->txstate)
		{
		case M6800_TX_STATE_INIT:
			cpustate->tx = 1;
			cpustate->txbits++;

			if (cpustate->txbits == 10)
			{
				cpustate->txstate = M6800_TX_STATE_READY;
				cpustate->txbits = M6800_SERIAL_START;
			}
			break;

		case M6800_TX_STATE_READY:
			switch (cpustate->txbits)
			{
			case M6800_SERIAL_START:
				if (cpustate->trcsr & M6800_TRCSR_TDRE)
				{
					// transmit buffer is empty, send consecutive '1's
					cpustate->tx = 1;
				}
				else
				{
					// transmit buffer is full, send data

					// load TDR to shift register
					cpustate->tsr = cpustate->tdr;

					// transmit buffer is empty, set TDRE flag
					cpustate->trcsr |= M6800_TRCSR_TDRE;

					// send start bit '0'
					cpustate->tx = 0;

					cpustate->txbits++;

					//logerror("M6800 '%s' Transmit START Data %02x\n", cpustate->device->tag(), cpustate->tsr);
				}
				break;

			case M6800_SERIAL_STOP:
				// send stop bit '1'
				cpustate->tx = 1;

				CHECK_IRQ_LINES(cpustate);

				cpustate->txbits = M6800_SERIAL_START;

				//logerror("M6800 '%s' Transmit STOP\n", cpustate->device->tag());
				break;

			default:
				// send data bit '0' or '1'
				cpustate->tx = cpustate->tsr & 0x01;

				// shift transmit register
				cpustate->tsr >>= 1;

				//logerror("M6800 '%s' Transmit Bit %u: %u\n", cpustate->device->tag(), cpustate->txbits, cpustate->tx);

				cpustate->txbits++;
				break;
			}
			break;
		}

		cpustate->port2_written = 1;
		write_port2(cpustate);
	}
}

static void serial_receive(m6800_state *cpustate)
{
	//logerror("M6800 '%s' Rx Tick TRCSR %02x bits %u check %02x\n", cpustate->device->tag(), cpustate->trcsr, cpustate->rxbits, cpustate->trcsr & M6800_TRCSR_RE);

	if (cpustate->trcsr & M6800_TRCSR_RE)
	{
		if (cpustate->trcsr & M6800_TRCSR_WU)
		{
			// wait for 10 bits of '1'
			if (m6800_rx(cpustate) == 1)
			{
				cpustate->rxbits++;

				//logerror("M6800 '%s' Received WAKE UP bit %u\n", cpustate->device->tag(), cpustate->rxbits);

				if (cpustate->rxbits == 10)
				{
					//logerror("M6800 '%s' Receiver Wake Up\n", cpustate->device->tag());

					cpustate->trcsr &= ~M6800_TRCSR_WU;
					cpustate->rxbits = M6800_SERIAL_START;
				}
			}
			else
			{
				//logerror("M6800 '%s' Receiver Wake Up interrupted\n", cpustate->device->tag());

				cpustate->rxbits = M6800_SERIAL_START;
			}
		}
		else
		{
			// receive data
			switch (cpustate->rxbits)
			{
			case M6800_SERIAL_START:
				if (m6800_rx(cpustate) == 0)
				{
					// start bit found
					cpustate->rxbits++;

					//logerror("M6800 '%s' Received START bit\n", cpustate->device->tag());
				}
				break;

			case M6800_SERIAL_STOP:
				if (m6800_rx(cpustate) == 1)
				{
					//logerror("M6800 '%s' Received STOP bit\n", cpustate->device->tag());

					if (cpustate->trcsr & M6800_TRCSR_RDRF)
					{
						// overrun error
						cpustate->trcsr |= M6800_TRCSR_ORFE;

						//logerror("M6800 '%s' Receive Overrun Error\n", cpustate->device->tag());

						CHECK_IRQ_LINES(cpustate);
					}
					else
					{
						if (!(cpustate->trcsr & M6800_TRCSR_ORFE))
						{
							// transfer data into receive register
							cpustate->rdr = cpustate->rsr;

							//logerror("M6800 '%s' Receive Data Register: %02x\n", cpustate->device->tag(), cpustate->rdr);

							// set RDRF flag
							cpustate->trcsr |= M6800_TRCSR_RDRF;

							CHECK_IRQ_LINES(cpustate);
						}
					}
				}
				else
				{
					// framing error
					if (!(cpustate->trcsr & M6800_TRCSR_ORFE))
					{
						// transfer unframed data into receive register
						cpustate->rdr = cpustate->rsr;
					}

					cpustate->trcsr |= M6800_TRCSR_ORFE;
					cpustate->trcsr &= ~M6800_TRCSR_RDRF;

					//logerror("M6800 '%s' Receive Framing Error\n", cpustate->device->tag());

					CHECK_IRQ_LINES(cpustate);
				}

				cpustate->rxbits = M6800_SERIAL_START;
				break;

			default:
				// shift receive register
				cpustate->rsr >>= 1;

				// receive bit into register
				cpustate->rsr |= (m6800_rx(cpustate) << 7);

				//logerror("M6800 '%s' Received DATA bit %u: %u\n", cpustate->device->tag(), cpustate->rxbits, BIT(cpustate->rsr, 7));

				cpustate->rxbits++;
				break;
			}
		}
	}
}

static TIMER_CALLBACK( sci_tick )
{
    m6800_state *cpustate = (m6800_state *)ptr;

	serial_transmit(cpustate);
	serial_receive(cpustate);
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static void state_register(m6800_state *cpustate, const char *type)
{
	cpustate->device->save_item(NAME(cpustate->ppc.w.l));
	cpustate->device->save_item(NAME(cpustate->pc.w.l));
	cpustate->device->save_item(NAME(cpustate->s.w.l));
	cpustate->device->save_item(NAME(cpustate->x.w.l));
	cpustate->device->save_item(NAME(cpustate->d.w.l));
	cpustate->device->save_item(NAME(cpustate->cc));
	cpustate->device->save_item(NAME(cpustate->wai_state));
	cpustate->device->save_item(NAME(cpustate->nmi_state));
	cpustate->device->save_item(NAME(cpustate->nmi_pending));
	cpustate->device->save_item(NAME(cpustate->irq_state));
	cpustate->device->save_item(NAME(cpustate->ic_eddge));

	cpustate->device->save_item(NAME(cpustate->port1_ddr));
	cpustate->device->save_item(NAME(cpustate->port2_ddr));
	cpustate->device->save_item(NAME(cpustate->port3_ddr));
	cpustate->device->save_item(NAME(cpustate->port4_ddr));
	cpustate->device->save_item(NAME(cpustate->port1_data));
	cpustate->device->save_item(NAME(cpustate->port2_data));
	cpustate->device->save_item(NAME(cpustate->port3_data));
	cpustate->device->save_item(NAME(cpustate->port4_data));
	cpustate->device->save_item(NAME(cpustate->port2_written));
	cpustate->device->save_item(NAME(cpustate->port3_latched));
	cpustate->device->save_item(NAME(cpustate->p3csr));
	cpustate->device->save_item(NAME(cpustate->p3csr_is3_flag_read));
	cpustate->device->save_item(NAME(cpustate->tcsr));
	cpustate->device->save_item(NAME(cpustate->pending_tcsr));
	cpustate->device->save_item(NAME(cpustate->irq2));
	cpustate->device->save_item(NAME(cpustate->ram_ctrl));

	cpustate->device->save_item(NAME(cpustate->counter.d));
	cpustate->device->save_item(NAME(cpustate->output_compare.d));
	cpustate->device->save_item(NAME(cpustate->input_capture));
	cpustate->device->save_item(NAME(cpustate->timer_over.d));

	cpustate->device->save_item(NAME(cpustate->clock));
	cpustate->device->save_item(NAME(cpustate->trcsr));
	cpustate->device->save_item(NAME(cpustate->rmcr));
	cpustate->device->save_item(NAME(cpustate->rdr));
	cpustate->device->save_item(NAME(cpustate->tdr));
	cpustate->device->save_item(NAME(cpustate->rsr));
	cpustate->device->save_item(NAME(cpustate->tsr));
	cpustate->device->save_item(NAME(cpustate->rxbits));
	cpustate->device->save_item(NAME(cpustate->txbits));
	cpustate->device->save_item(NAME(cpustate->txstate));
	cpustate->device->save_item(NAME(cpustate->trcsr_read_tdre));
	cpustate->device->save_item(NAME(cpustate->trcsr_read_orfe));
	cpustate->device->save_item(NAME(cpustate->trcsr_read_rdrf));
	cpustate->device->save_item(NAME(cpustate->tx));
}

static CPU_INIT( m6800 )
{
	m6800_state *cpustate = get_safe_token(device);

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	//  cpustate->subtype   = SUBTYPE_M6800;
	cpustate->insn = m6800_insn;
	cpustate->cycles = cycles_6800;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	state_register(cpustate, "m6800");
}

static CPU_RESET( m6800 )
{
	m6800_state *cpustate = get_safe_token(device);

	cpustate->cc = 0xc0;
	SEI;				/* IRQ disabled */
	PCD = RM16(cpustate,  0xfffe );

	cpustate->wai_state = 0;
	cpustate->nmi_state = 0;
	cpustate->nmi_pending = 0;
	cpustate->sc1_state = 0;
	cpustate->irq_state[M6800_IRQ_LINE] = 0;
	cpustate->irq_state[M6801_TIN_LINE] = 0;
	cpustate->ic_eddge = 0;

	cpustate->port1_ddr = 0x00;
	cpustate->port2_ddr = 0x00;
	cpustate->port3_ddr = 0x00;
	cpustate->p3csr = 0x00;
	cpustate->p3csr_is3_flag_read = 0;
	cpustate->port2_written = 0;
	cpustate->port3_latched = 0;
	/* TODO: on reset port 2 should be read to determine the operating mode (bits 0-2) */
	cpustate->tcsr = 0x00;
	cpustate->pending_tcsr = 0x00;
	cpustate->irq2 = 0;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	cpustate->ram_ctrl |= 0x40;

	cpustate->trcsr = M6800_TRCSR_TDRE;

	cpustate->txstate = M6800_TX_STATE_INIT;
	cpustate->txbits = cpustate->rxbits = 0;
	cpustate->tx = 1;
	cpustate->trcsr_read_tdre = 0;
	cpustate->trcsr_read_orfe = 0;
	cpustate->trcsr_read_rdrf = 0;

	set_rmcr(cpustate, 0);
}

/****************************************************************************
 * Shut down CPU emulation
 ****************************************************************************/
static CPU_EXIT( m6800 )
{
	/* nothing to do */
}


static void set_irq_line(m6800_state *cpustate, int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_NMI:
		if (!cpustate->nmi_state && state != CLEAR_LINE)
			cpustate->nmi_pending = TRUE;
		cpustate->nmi_state = state;
		break;

	case M6801_SC1_LINE:
		if (!cpustate->port3_latched && (cpustate->p3csr & M6801_P3CSR_LE))
		{
			if (!cpustate->sc1_state && state)
			{
				// latch input data to port 3
				cpustate->port3_data = (cpustate->io->read_byte(M6801_PORT3) & (cpustate->port3_ddr ^ 0xff)) | (cpustate->port3_data & cpustate->port3_ddr);
				cpustate->port3_latched = 1;
				//logerror("M6801 '%s' Latched Port 3 Data: %02x\n", cpustate->device->tag(), cpustate->port3_data);

				// set IS3 flag bit
				cpustate->p3csr |= M6801_P3CSR_IS3_FLAG;
			}
		}
		cpustate->sc1_state = state;
		break;

	default:
		LOG(("M6800 '%s' set_irq_line %d,%d\n", cpustate->device->tag(), irqline, state));
		cpustate->irq_state[irqline] = state;

		if (irqline == M6801_TIN_LINE && state != cpustate->irq_state[irqline])
		{
			//eddge = (state == CLEAR_LINE ) ? 2 : 0;
			if( ((cpustate->tcsr&TCSR_IEDG) ^ (state==CLEAR_LINE ? TCSR_IEDG : 0))==0 )
				return;
			/* active edge in */
			cpustate->tcsr |= TCSR_ICF;
			cpustate->pending_tcsr |= TCSR_ICF;
			cpustate->input_capture = CT;
			MODIFIED_tcsr;
		}
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static CPU_EXECUTE( m6800 )
{
	m6800_state *cpustate = get_safe_token(device);
	UINT8 ireg;

	CHECK_IRQ_LINES(cpustate); /* HJB 990417 */

	CLEANUP_COUNTERS();

	do
	{
		if( cpustate->wai_state & (M6800_WAI|M6800_SLP) )
		{
			EAT_CYCLES;
		}
		else
		{
			pPPC = pPC;
			debugger_instruction_hook(device, PCD);
			ireg=M_RDOP(PCD);
			PC++;
			(*cpustate->insn[ireg])(cpustate);
			increment_counter(cpustate, cpustate->cycles[ireg]);
		}
	} while( cpustate->icount>0 );
}

/****************************************************************************
 * M6801 almost (fully?) equal to the M6803
 ****************************************************************************/
static CPU_INIT( m6801 )
{
	m6800_state *cpustate = get_safe_token(device);
//  cpustate->subtype = SUBTYPE_M6801;
	cpustate->insn = m6803_insn;
	cpustate->cycles = cycles_6803;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	cpustate->clock = device->clock() / 4;
	cpustate->sci_timer = device->machine().scheduler().timer_alloc(FUNC(sci_tick), cpustate);

	state_register(cpustate, "m6801");

	if (device->static_config() != NULL)
	{
		m6801_interface *intf = (m6801_interface *) device->static_config();

		cpustate->out_sc2_func.resolve(intf->out_sc2_func, *device);
	}
	else
	{
		devcb_write_line nullcb = DEVCB_NULL;
		cpustate->out_sc2_func.resolve(nullcb, *device);
	}
}

/****************************************************************************
 * M6802 almost (fully?) equal to the M6800
 ****************************************************************************/
static CPU_INIT( m6802 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype   = SUBTYPE_M6802;
	cpustate->insn = m6800_insn;
	cpustate->cycles = cycles_6800;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	state_register(cpustate, "m6802");
}

/****************************************************************************
 * M6803 almost (fully?) equal to the M6801
 ****************************************************************************/
static CPU_INIT( m6803 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_M6803;
	cpustate->insn = m6803_insn;
	cpustate->cycles = cycles_6803;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	cpustate->clock = device->clock() / 4;
	cpustate->sci_timer = device->machine().scheduler().timer_alloc(FUNC(sci_tick), cpustate);

	state_register(cpustate, "m6803");

	if (device->static_config() != NULL)
	{
		m6801_interface *intf = (m6801_interface *) device->static_config();

		cpustate->out_sc2_func.resolve(intf->out_sc2_func, *device);
	}
	else
	{
		devcb_write_line nullcb = DEVCB_NULL;
		cpustate->out_sc2_func.resolve(nullcb, *device);
	}
}

static ADDRESS_MAP_START(m6803_mem, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0020, 0x007f) AM_NOP        /* unused */
	AM_RANGE(0x0080, 0x00ff) AM_RAM        /* 6803 internal RAM */
ADDRESS_MAP_END


/****************************************************************************
 * M6808 almost (fully?) equal to the M6800
 ****************************************************************************/
static CPU_INIT( m6808 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_M6808;
	cpustate->insn = m6800_insn;
	cpustate->cycles = cycles_6800;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	state_register(cpustate, "m6808");
}

/****************************************************************************
 * HD6301 similiar to the M6800
 ****************************************************************************/

static CPU_INIT( hd6301 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_HD6301;
	cpustate->insn = hd63701_insn;
	cpustate->cycles = cycles_63701;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	cpustate->clock = device->clock() / 4;
	cpustate->sci_timer = device->machine().scheduler().timer_alloc(FUNC(sci_tick), cpustate);

	state_register(cpustate, "hd6301");
}

/****************************************************************************
 * HD63701 similiar to the HD6301
 ****************************************************************************/

static CPU_INIT( hd63701 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_HD63701;
	cpustate->insn = hd63701_insn;
	cpustate->cycles = cycles_63701;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	cpustate->clock = device->clock() / 4;
	cpustate->sci_timer = device->machine().scheduler().timer_alloc(FUNC(sci_tick), cpustate);

	state_register(cpustate, "hd63701");
}

/*
    if change_pc(cpustate) direccted these areas ,Call hd63701_trap_pc(cpustate).
    'mode' is selected by the sense of p2.0,p2.1,and p2.3 at reset timming.
    mode 0,1,2,4,6 : $0000-$001f
    mode 5         : $0000-$001f,$0200-$efff
    mode 7         : $0000-$001f,$0100-$efff
*/
#if 0
static void hd63701_trap_pc(m6800_state *cpustate)
{
	TAKE_TRAP;
}
#endif

/****************************************************************************
 * NSC-8105 similiar to the M6800, but the opcodes are scrambled and there
 * is at least one new opcode ($fc)
 ****************************************************************************/
static CPU_INIT( nsc8105 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_NSC8105;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	cpustate->insn = nsc8105_insn;
	cpustate->cycles = cycles_nsc8105;
	state_register(cpustate, "nsc8105");
}

INLINE void set_os3(m6800_state *cpustate, int state)
{
	//logerror("M6801 '%s' OS3: %u\n", cpustate->device->tag(), state);

	cpustate->out_sc2_func(state);
}

READ8_HANDLER( m6801_io_r )
{
	m6800_state *cpustate = get_safe_token(&space->device());

	UINT8 data = 0;

	switch (offset)
	{
	case IO_P1DDR:
		data = cpustate->port1_ddr;
		break;

	case IO_P2DDR:
		data = cpustate->port2_ddr;
		break;

	case IO_P1DATA:
		if(cpustate->port1_ddr == 0xff)
			data = cpustate->port1_data;
		else
			data = (cpustate->io->read_byte(M6801_PORT1) & (cpustate->port1_ddr ^ 0xff))
				| (cpustate->port1_data & cpustate->port1_ddr);
		break;

	case IO_P2DATA:
		if(cpustate->port2_ddr == 0xff)
			data = cpustate->port2_data;
		else
			data = (cpustate->io->read_byte(M6801_PORT2) & (cpustate->port2_ddr ^ 0xff))
				| (cpustate->port2_data & cpustate->port2_ddr);
		break;

	case IO_P3DDR:
		logerror("M6801 '%s' Port 3 DDR is a write-only register\n", space->device().tag());
		break;

	case IO_P4DDR:
		data = cpustate->port4_ddr;
		break;

	case IO_P3DATA:
		if (!space->debugger_access())
		{
			if (cpustate->p3csr_is3_flag_read)
			{
				//logerror("M6801 '%s' Cleared IS3\n", space->device().tag());
				cpustate->p3csr &= ~M6801_P3CSR_IS3_FLAG;
				cpustate->p3csr_is3_flag_read = 0;
			}

			if (!(cpustate->p3csr & M6801_P3CSR_OSS))
			{
				set_os3(cpustate, ASSERT_LINE);
			}
		}

		if ((cpustate->p3csr & M6801_P3CSR_LE) || (cpustate->port3_ddr == 0xff))
			data = cpustate->port3_data;
		else
			data = (cpustate->io->read_byte(M6801_PORT3) & (cpustate->port3_ddr ^ 0xff))
				| (cpustate->port3_data & cpustate->port3_ddr);

		if (!space->debugger_access())
		{
			cpustate->port3_latched = 0;

			if (!(cpustate->p3csr & M6801_P3CSR_OSS))
			{
				set_os3(cpustate, CLEAR_LINE);
			}
		}
		break;

	case IO_P4DATA:
		if(cpustate->port4_ddr == 0xff)
			data = cpustate->port4_data;
		else
			data = (cpustate->io->read_byte(M6801_PORT4) & (cpustate->port4_ddr ^ 0xff))
				| (cpustate->port4_data & cpustate->port4_ddr);
		break;

	case IO_TCSR:
		cpustate->pending_tcsr = 0;
		data = cpustate->tcsr;
		break;

	case IO_CH:
		if(!(cpustate->pending_tcsr&TCSR_TOF) && !space->debugger_access())
		{
			cpustate->tcsr &= ~TCSR_TOF;
			MODIFIED_tcsr;
		}
		data = cpustate->counter.b.h;
		break;

	case IO_CL:
		data = cpustate->counter.b.l;

	case IO_OCRH:
		if(!(cpustate->pending_tcsr&TCSR_OCF) && !space->debugger_access())
		{
			cpustate->tcsr &= ~TCSR_OCF;
			MODIFIED_tcsr;
		}
		data = cpustate->output_compare.b.h;
		break;

	case IO_OCRL:
		if(!(cpustate->pending_tcsr&TCSR_OCF) && !space->debugger_access())
		{
			cpustate->tcsr &= ~TCSR_OCF;
			MODIFIED_tcsr;
		}
		data = cpustate->output_compare.b.l;
		break;

	case IO_ICRH:
		if(!(cpustate->pending_tcsr&TCSR_ICF) && !space->debugger_access())
		{
			cpustate->tcsr &= ~TCSR_ICF;
			MODIFIED_tcsr;
		}
		data = (cpustate->input_capture >> 0) & 0xff;
		break;

	case IO_ICRL:
		data = (cpustate->input_capture >> 8) & 0xff;
		break;

	case IO_P3CSR:
		if ((cpustate->p3csr & M6801_P3CSR_IS3_FLAG) && !space->debugger_access())
		{
			cpustate->p3csr_is3_flag_read = 1;
		}

		data = cpustate->p3csr;
		break;

	case IO_RMCR:
		data = cpustate->rmcr;
		break;

	case IO_TRCSR:
		if (!space->debugger_access())
		{
			if (cpustate->trcsr & M6800_TRCSR_TDRE)
			{
				cpustate->trcsr_read_tdre = 1;
			}

			if (cpustate->trcsr & M6800_TRCSR_ORFE)
			{
				cpustate->trcsr_read_orfe = 1;
			}

			if (cpustate->trcsr & M6800_TRCSR_RDRF)
			{
				cpustate->trcsr_read_rdrf = 1;
			}
		}

		data = cpustate->trcsr;
		break;

	case IO_RDR:
		if (!space->debugger_access())
		{
			if (cpustate->trcsr_read_orfe)
			{
				//logerror("M6801 '%s' Cleared ORFE\n", space->device().tag());
				cpustate->trcsr_read_orfe = 0;
				cpustate->trcsr &= ~M6800_TRCSR_ORFE;
			}

			if (cpustate->trcsr_read_rdrf)
			{
				//logerror("M6801 '%s' Cleared RDRF\n", space->device().tag());
				cpustate->trcsr_read_rdrf = 0;
				cpustate->trcsr &= ~M6800_TRCSR_RDRF;
			}
		}

		data = cpustate->rdr;
		break;

	case IO_TDR:
		data = cpustate->tdr;
		break;

	case IO_RCR:
		data = cpustate->ram_ctrl;
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
		logerror("M6801 '%s' PC %04x: warning - read from reserved internal register %02x\n",space->device().tag(),cpu_get_pc(&space->device()),offset);
	}

	return data;
}

WRITE8_HANDLER( m6801_io_w )
{
	m6800_state *cpustate = get_safe_token(&space->device());

	switch (offset)
	{
	case IO_P1DDR:
		//logerror("M6801 '%s' Port 1 Data Direction Register: %02x\n", space->device().tag(), data);

		if (cpustate->port1_ddr != data)
		{
			cpustate->port1_ddr = data;
			if(cpustate->port1_ddr == 0xff)
				cpustate->io->write_byte(M6801_PORT1,cpustate->port1_data);
			else
				cpustate->io->write_byte(M6801_PORT1,(cpustate->port1_data & cpustate->port1_ddr) | (cpustate->port1_ddr ^ 0xff));
		}
		break;

	case IO_P2DDR:
		//logerror("M6801 '%s' Port 2 Data Direction Register: %02x\n", space->device().tag(), data);

		if (cpustate->port2_ddr != data)
		{
			cpustate->port2_ddr = data;
			write_port2(cpustate);

			if (cpustate->port2_ddr & 2)
				logerror("CPU '%s' PC %04x: warning - port 2 bit 1 set as output (OLVL) - not supported\n",space->device().tag(),cpu_get_pc(&space->device()));
		}
		break;

	case IO_P1DATA:
		//logerror("M6801 '%s' Port 1 Data Register: %02x\n", space->device().tag(), data);

		cpustate->port1_data = data;
		if(cpustate->port1_ddr == 0xff)
			cpustate->io->write_byte(M6801_PORT1,cpustate->port1_data);
		else
			cpustate->io->write_byte(M6801_PORT1,(cpustate->port1_data & cpustate->port1_ddr) | (cpustate->port1_ddr ^ 0xff));
		break;

	case IO_P2DATA:
		//logerror("M6801 '%s' Port 2 Data Register: %02x\n", space->device().tag(), data);

		cpustate->port2_data = data;
		cpustate->port2_written = 1;
		write_port2(cpustate);
		break;

	case IO_P3DDR:
		//logerror("M6801 '%s' Port 3 Data Direction Register: %02x\n", space->device().tag(), data);

		if (cpustate->port3_ddr != data)
		{
			cpustate->port3_ddr = data;
			if(cpustate->port3_ddr == 0xff)
				cpustate->io->write_byte(M6801_PORT3,cpustate->port3_data);
			else
				cpustate->io->write_byte(M6801_PORT3,(cpustate->port3_data & cpustate->port3_ddr) | (cpustate->port3_ddr ^ 0xff));
		}
		break;

	case IO_P4DDR:
		//logerror("M6801 '%s' Port 4 Data Direction Register: %02x\n", space->device().tag(), data);

		if (cpustate->port4_ddr != data)
		{
			cpustate->port4_ddr = data;
			if(cpustate->port4_ddr == 0xff)
				cpustate->io->write_byte(M6801_PORT4,cpustate->port4_data);
			else
				cpustate->io->write_byte(M6801_PORT4,(cpustate->port4_data & cpustate->port4_ddr) | (cpustate->port4_ddr ^ 0xff));
		}
		break;

	case IO_P3DATA:
		//logerror("M6801 '%s' Port 3 Data Register: %02x\n", space->device().tag(), data);

		if (cpustate->p3csr_is3_flag_read)
		{
			//logerror("M6801 '%s' Cleared IS3\n", space->device().tag());
			cpustate->p3csr &= ~M6801_P3CSR_IS3_FLAG;
			cpustate->p3csr_is3_flag_read = 0;
		}

		if (cpustate->p3csr & M6801_P3CSR_OSS)
		{
			set_os3(cpustate, ASSERT_LINE);
		}

		cpustate->port3_data = data;
		if(cpustate->port3_ddr == 0xff)
			cpustate->io->write_byte(M6801_PORT3,cpustate->port3_data);
		else
			cpustate->io->write_byte(M6801_PORT3,(cpustate->port3_data & cpustate->port3_ddr) | (cpustate->port3_ddr ^ 0xff));

		if (cpustate->p3csr & M6801_P3CSR_OSS)
		{
			set_os3(cpustate, CLEAR_LINE);
		}
		break;

	case IO_P4DATA:
		//logerror("M6801 '%s' Port 4 Data Register: %02x\n", space->device().tag(), data);

		cpustate->port4_data = data;
		if(cpustate->port4_ddr == 0xff)
			cpustate->io->write_byte(M6801_PORT4,cpustate->port4_data);
		else
			cpustate->io->write_byte(M6801_PORT4,(cpustate->port4_data & cpustate->port4_ddr) | (cpustate->port4_ddr ^ 0xff));
		break;

	case IO_TCSR:
		//logerror("M6801 '%s' Timer Control and Status Register: %02x\n", space->device().tag(), data);

		cpustate->tcsr = data;
		cpustate->pending_tcsr &= cpustate->tcsr;
		MODIFIED_tcsr;
		if( !(CC & 0x10) )
			m6800_check_irq2(cpustate);
		break;

	case IO_CH:
		//logerror("M6801 '%s' Counter High Register: %02x\n", space->device().tag(), data);

		cpustate->latch09 = data & 0xff;	/* 6301 only */
		CT  = 0xfff8;
		TOH = CTH;
		MODIFIED_counters;
		break;

	case IO_CL:	/* 6301 only */
		//logerror("M6801 '%s' Counter Low Register: %02x\n", space->device().tag(), data);

		CT = (cpustate->latch09 << 8) | (data & 0xff);
		TOH = CTH;
		MODIFIED_counters;
		break;

	case IO_OCRH:
		//logerror("M6801 '%s' Output Compare High Register: %02x\n", space->device().tag(), data);

		if( cpustate->output_compare.b.h != data)
		{
			cpustate->output_compare.b.h = data;
			MODIFIED_counters;
		}
		break;

	case IO_OCRL:
		//logerror("M6801 '%s' Output Compare Low Register: %02x\n", space->device().tag(), data);

		if( cpustate->output_compare.b.l != data)
		{
			cpustate->output_compare.b.l = data;
			MODIFIED_counters;
		}
		break;

	case IO_ICRH:
	case IO_ICRL:
	case IO_RDR:
		//logerror("CPU '%s' PC %04x: warning - write %02x to read only internal register %02x\n",space->device().tag(),cpu_get_pc(&space->device()),data,offset);
		break;

	case IO_P3CSR:
		//logerror("M6801 '%s' Port 3 Control and Status Register: %02x\n", space->device().tag(), data);

		cpustate->p3csr = data;
		break;

	case IO_RMCR:
		//logerror("M6801 '%s' Rate and Mode Control Register: %02x\n", space->device().tag(), data);

		set_rmcr(cpustate, data);
		break;

	case IO_TRCSR:
		//logerror("M6801 '%s' Transmit/Receive Control and Status Register: %02x\n", space->device().tag(), data);

		if ((data & M6800_TRCSR_TE) && !(cpustate->trcsr & M6800_TRCSR_TE))
		{
			cpustate->txstate = M6800_TX_STATE_INIT;
			cpustate->txbits = 0;
			cpustate->tx = 1;
		}

		if ((data & M6800_TRCSR_RE) && !(cpustate->trcsr & M6800_TRCSR_RE))
		{
			cpustate->rxbits = 0;
		}

		cpustate->trcsr = (cpustate->trcsr & 0xe0) | (data & 0x1f);
		break;

	case IO_TDR:
		//logerror("M6800 '%s' Transmit Data Register: %02x\n", space->device().tag(), data);

		if (cpustate->trcsr_read_tdre)
		{
			cpustate->trcsr_read_tdre = 0;
			cpustate->trcsr &= ~M6800_TRCSR_TDRE;
		}
		cpustate->tdr = data;
		break;

	case IO_RCR:
		//logerror("M6801 '%s' RAM Control Register: %02x\n", space->device().tag(), data);

		cpustate->ram_ctrl = data;
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
		logerror("M6801 '%s' PC %04x: warning - write %02x to reserved internal register %02x\n",space->device().tag(),cpu_get_pc(&space->device()),data,offset);
		break;
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m6800 )
{
	m6800_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6800_IRQ_LINE:	set_irq_line(cpustate, M6800_IRQ_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + M6801_TIN_LINE:	set_irq_line(cpustate, M6801_TIN_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + M6801_SC1_LINE:	set_irq_line(cpustate, M6801_SC1_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:							PC = info->i;								break;
		case CPUINFO_INT_REGISTER + M6800_PC:			cpustate->pc.w.l = info->i;					break;
		case CPUINFO_INT_SP:							S = info->i;							break;
		case CPUINFO_INT_REGISTER + M6800_S:			cpustate->s.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6800_CC:			cpustate->cc = info->i;						break;
		case CPUINFO_INT_REGISTER + M6800_A:			cpustate->d.b.h = info->i;					break;
		case CPUINFO_INT_REGISTER + M6800_B:			cpustate->d.b.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6800_X:			cpustate->x.w.l = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m6800 )
{
	m6800_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6800_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 12;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 9;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6800_IRQ_LINE:	info->i = cpustate->irq_state[M6800_IRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + M6801_TIN_LINE:	info->i = cpustate->irq_state[M6801_TIN_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + M6801_SC1_LINE:	info->i = cpustate->irq_state[M6801_SC1_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc.w.l;				break;

		case CPUINFO_INT_PC:							info->i = PC;							break;
		case CPUINFO_INT_REGISTER + M6800_PC:			info->i = cpustate->pc.w.l;					break;
		case CPUINFO_INT_SP:							info->i = S;							break;
		case CPUINFO_INT_REGISTER + M6800_S:			info->i = cpustate->s.w.l;					break;
		case CPUINFO_INT_REGISTER + M6800_CC:			info->i = cpustate->cc;						break;
		case CPUINFO_INT_REGISTER + M6800_A:			info->i = cpustate->d.b.h;					break;
		case CPUINFO_INT_REGISTER + M6800_B:			info->i = cpustate->d.b.l;					break;
		case CPUINFO_INT_REGISTER + M6800_X:			info->i = cpustate->x.w.l;					break;
		case CPUINFO_INT_REGISTER + M6800_WAI_STATE:	info->i = cpustate->wai_state;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m6800);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6800);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m6800);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m6800);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6800);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6800);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6800");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Motorola 6800");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.1");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "The MAME team.");		break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->cc & 0x80 ? '?':'.',
				cpustate->cc & 0x40 ? '?':'.',
				cpustate->cc & 0x20 ? 'H':'.',
				cpustate->cc & 0x10 ? 'I':'.',
				cpustate->cc & 0x08 ? 'N':'.',
				cpustate->cc & 0x04 ? 'Z':'.',
				cpustate->cc & 0x02 ? 'V':'.',
				cpustate->cc & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M6800_A:			sprintf(info->s, "A:%02X", cpustate->d.b.h); break;
		case CPUINFO_STR_REGISTER + M6800_B:			sprintf(info->s, "B:%02X", cpustate->d.b.l); break;
		case CPUINFO_STR_REGISTER + M6800_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + M6800_S:			sprintf(info->s, "S:%04X", cpustate->s.w.l); break;
		case CPUINFO_STR_REGISTER + M6800_X:			sprintf(info->s, "X:%04X", cpustate->x.w.l); break;
		case CPUINFO_STR_REGISTER + M6800_CC:			sprintf(info->s, "CC:%02X", cpustate->cc); break;
		case CPUINFO_STR_REGISTER + M6800_WAI_STATE:	sprintf(info->s, "WAI:%X", cpustate->wai_state); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m6801 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:							info->i = 4;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6801);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6801);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6801");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m6802 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6802);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6802);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6802");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m6803 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:							info->i = 4;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6803);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6803);			break;

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(m6803_mem); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6803");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m6808 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6808);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6808);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6808");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( hd6301 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:							info->i = 4;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(hd6301);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(hd6301);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "HD6301");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( hd63701 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:							info->i = 4;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(hd63701);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(hd63701);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "HD63701");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( nsc8105 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(nsc8105);				break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(nsc8105);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "NSC8105");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(M6800, m6800);
DEFINE_LEGACY_CPU_DEVICE(M6801, m6801);
DEFINE_LEGACY_CPU_DEVICE(M6802, m6802);
DEFINE_LEGACY_CPU_DEVICE(M6803, m6803);
DEFINE_LEGACY_CPU_DEVICE(M6808, m6808);
DEFINE_LEGACY_CPU_DEVICE(HD6301, hd6301);
DEFINE_LEGACY_CPU_DEVICE(HD63701, hd63701);
DEFINE_LEGACY_CPU_DEVICE(NSC8105, nsc8105);
