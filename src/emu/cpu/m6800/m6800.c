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

    Chip                RAM     NVRAM   ROM     SCI
    -----------------------------------------------
    MC6800              -       -       -       no
    MC6801              128     64      2K      yes
    MC68701             128     64      -       yes
    MC6803              128     64      -       yes
    MC6802              128     32      -       no
    MC6802NS            128     -       -       no
    MC6808              -       -       -       no

    HD6301              128     -       4K      yes
    HD63701             192     -       4K      yes
    HD6303              128     -       -       yes
    HD6801              128     64      2K      yes

    NSC8105
    MS2010-A

*/

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
	UINT8	irq_state[2];	/* IRQ line state [IRQ1,TIN] */
	UINT8	ic_eddge;		/* InputCapture eddge , b.0=fall,b.1=raise */

	cpu_irq_callback irq_callback;
	const device_config *device;

	/* Memory spaces */
    const address_space *program;
    const address_space *data;
    const address_space *io;

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
	UINT8	tcsr;			/* Timer Control and Status Register */
	UINT8	pending_tcsr;	/* pending IRQ flag for clear IRQflag process */
	UINT8	irq2;			/* IRQ2 flags */
	UINT8	ram_ctrl;
	PAIR	counter;		/* free running counter */
	PAIR	output_compare;	/* output compare       */
	UINT16	input_capture;	/* input capture        */

	int		clock;
	UINT8	trcsr, rmcr, rdr, tdr, rsr, tsr;
	int		rxbits, txbits, txstate, trcsr_read, tx;

	int		icount;
	int		latch09;

	PAIR	timer_over;
	emu_timer *m6800_rx_timer;
	emu_timer *m6800_tx_timer;
	PAIR ea; 		/* effective address */

};

INLINE m6800_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_M6800 ||
		   cpu_get_type(device) == CPU_M6801 ||
		   cpu_get_type(device) == CPU_M6802 ||
		   cpu_get_type(device) == CPU_M6803 ||
		   cpu_get_type(device) == CPU_M6808 ||
		   cpu_get_type(device) == CPU_HD63701 ||
		   cpu_get_type(device) == CPU_NSC8105);
	return (m6800_state *)device->token;
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
#define RM(Addr) ((unsigned)memory_read_byte_8be(cpustate->program, Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (memory_write_byte_8be(cpustate->program, Addr,Value))

/****************************************************************************/
/* M6800_RDOP() is identical to M6800_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M_RDOP(Addr) ((unsigned)memory_decrypted_read_byte(cpustate->program, Addr))

/****************************************************************************/
/* M6800_RDOP_ARG() is identical to M6800_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M_RDOP_ARG(Addr) ((unsigned)memory_raw_read_byte(cpustate->program, Addr))

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
#define M6800_TRCSR_RIE		0x10 // Receive Interrupt Enable
#define M6800_TRCSR_RE			0x08 // Receive Enable
#define M6800_TRCSR_TIE		0x04 // Transmit Interrupt Enable
#define M6800_TRCSR_TE			0x02 // Transmit Enable
#define M6800_TRCSR_WU			0x01 // Wake Up

#define M6800_PORT2_IO4		0x10
#define M6800_PORT2_IO3		0x08

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

/* Note: we use 99 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */
static const UINT8 cycles_6800[] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ 99, 2,99,99,99,99, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,99,99,99,99, 2, 2,99, 2,99, 2,99,99,99,99,
	/*2*/  4,99, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,99, 5,99,10,99,99, 9,12,
	/*4*/  2,99,99, 2, 2,99, 2, 2, 2, 2, 2,99, 2, 2,99, 2,
	/*5*/  2,99,99, 2, 2,99, 2, 2, 2, 2, 2,99, 2, 2,99, 2,
	/*6*/  7,99,99, 7, 7,99, 7, 7, 7, 7, 7,99, 7, 7, 4, 7,
	/*7*/  6,99,99, 6, 6,99, 6, 6, 6, 6, 6,99, 6, 6, 3, 6,
	/*8*/  2, 2, 2,99, 2, 2, 2,99, 2, 2, 2, 2, 3, 8, 3,99,
	/*9*/  3, 3, 3,99, 3, 3, 3, 4, 3, 3, 3, 3, 4,99, 4, 5,
	/*A*/  5, 5, 5,99, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
	/*B*/  4, 4, 4,99, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
	/*C*/  2, 2, 2,99, 2, 2, 2,99, 2, 2, 2, 2,99,99, 3,99,
	/*D*/  3, 3, 3,99, 3, 3, 3, 4, 3, 3, 3, 3,99,99, 4, 5,
	/*E*/  5, 5, 5,99, 5, 5, 5, 6, 5, 5, 5, 5,99,99, 6, 7,
	/*F*/  4, 4, 4,99, 4, 4, 4, 5, 4, 4, 4, 4,99,99, 5, 6
};

static const UINT8 cycles_6803[] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ 99, 2,99,99, 3, 3, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,99,99,99,99, 2, 2,99, 2,99, 2,99,99,99,99,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  3, 3, 4, 4, 3, 3, 3, 3, 5, 5, 3,10, 4,10, 9,12,
	/*4*/  2,99,99, 2, 2,99, 2, 2, 2, 2, 2,99, 2, 2,99, 2,
	/*5*/  2,99,99, 2, 2,99, 2, 2, 2, 2, 2,99, 2, 2,99, 2,
	/*6*/  6,99,99, 6, 6,99, 6, 6, 6, 6, 6,99, 6, 6, 3, 6,
	/*7*/  6,99,99, 6, 6,99, 6, 6, 6, 6, 6,99, 6, 6, 3, 6,
	/*8*/  2, 2, 2, 4, 2, 2, 2,99, 2, 2, 2, 2, 4, 6, 3,99,
	/*9*/  3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4,
	/*A*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	/*B*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	/*C*/  2, 2, 2, 4, 2, 2, 2,99, 2, 2, 2, 2, 3,99, 3,99,
	/*D*/  3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	/*E*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};

static const UINT8 cycles_63701[] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ 99, 1,99,99, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/*1*/  1, 1,99,99,99,99, 1, 1, 2, 2, 4, 1,99,99,99,99,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  1, 1, 3, 3, 1, 1, 4, 4, 4, 5, 1,10, 5, 7, 9,12,
	/*4*/  1,99,99, 1, 1,99, 1, 1, 1, 1, 1,99, 1, 1,99, 1,
	/*5*/  1,99,99, 1, 1,99, 1, 1, 1, 1, 1,99, 1, 1,99, 1,
	/*6*/  6, 7, 7, 6, 6, 7, 6, 6, 6, 6, 6, 5, 6, 4, 3, 5,
	/*7*/  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 3, 5,
	/*8*/  2, 2, 2, 3, 2, 2, 2,99, 2, 2, 2, 2, 3, 5, 3,99,
	/*9*/  3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4,
	/*A*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*B*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
	/*C*/  2, 2, 2, 3, 2, 2, 2,99, 2, 2, 2, 2, 3,99, 3,99,
	/*D*/  3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	/*E*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};

static const UINT8 cycles_nsc8105[] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ 99,99, 2,99,99, 2,99, 2, 4, 2, 4, 2, 2, 2, 2, 2,
	/*1*/  2,99, 2,99,99, 2,99, 2,99,99, 2, 2,99,99,99,99,
	/*2*/  4, 4,99, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,99,99, 5,10,99, 9,99,12,
	/*4*/  2,99,99, 2, 2, 2,99, 2, 2, 2, 2,99, 2,99, 2, 2,
	/*5*/  2,99,99, 2, 2, 2,99, 2, 2, 2, 2,99, 2,99, 2, 2,
	/*6*/  7,99,99, 7, 7, 7,99, 7, 7, 7, 7,99, 7, 4, 7, 7,
	/*7*/  6,99,99, 6, 6, 6,99, 6, 6, 6, 6,99, 6, 3, 6, 6,
	/*8*/  2, 2, 2,99, 2, 2, 2,99, 2, 2, 2, 2, 3, 3, 8,99,
	/*9*/  3, 3, 3,99, 3, 3, 3, 4, 3, 3, 3, 3, 4, 4,99, 5,
	/*A*/  5, 5, 5,99, 5, 5, 5, 6, 5, 5, 5, 5, 6, 6, 8, 7,
	/*B*/  4, 4, 4,99, 4, 4, 4, 5, 4, 4, 4, 4, 5, 5, 9, 6,
	/*C*/  2, 2, 2,99, 2, 2, 2,99, 2, 2, 2, 2,99, 3,99,99,
	/*D*/  3, 3, 3,99, 3, 3, 3, 4, 3, 3, 3, 3,99, 4,99, 5,
	/*E*/  5, 5, 5,99, 5, 5, 5, 6, 5, 5, 5, 5, 5, 6,99, 7,
	/*F*/  4, 4, 4,99, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5,99, 6
};

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
	LOG((message, cpustate->device->tag));
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
			(void)(*cpustate->irq_callback)(cpustate->device, M6800_TIN_LINE);
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
		TAKE_SCI;
	}
}


/* check the IRQ lines for pending interrupts */
INLINE void CHECK_IRQ_LINES(m6800_state *cpustate)
{
	if (cpustate->nmi_pending)
	{
		cpustate->nmi_pending = FALSE;
		enter_interrupt(cpustate, "M6800 '%s' take NMI\n",0xfffc);
	}
	else if( !(CC & 0x10) )
	{
		if( cpustate->irq_state[M6800_IRQ_LINE] != CLEAR_LINE )
		{	/* standard IRQ */
			enter_interrupt(cpustate, "M6800 '%s' take IRQ1n",0xfff8);
			if( cpustate->irq_callback )
				(void)(*cpustate->irq_callback)(cpustate->device, M6800_IRQ_LINE);
		}
		else
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

/* include the opcode prototypes and function pointer tables */
#include "6800tbl.c"

/* include the opcode functions */
#include "6800ops.c"

static void m6800_tx(m6800_state *cpustate, int value)
{
	cpustate->port2_data = (cpustate->port2_data & 0xef) | (value << 4);

	if(cpustate->port2_ddr == 0xff)
		memory_write_byte_8be(cpustate->io, M6803_PORT2,cpustate->port2_data);
	else
		memory_write_byte_8be(cpustate->io, M6803_PORT2,(cpustate->port2_data & cpustate->port2_ddr)
			| (memory_read_byte_8be(cpustate->io, M6803_PORT2) & (cpustate->port2_ddr ^ 0xff)));
}

static int m6800_rx(m6800_state *cpustate)
{
	return (memory_read_byte_8be(cpustate->io, M6803_PORT2) & M6800_PORT2_IO3) >> 3;
}

static TIMER_CALLBACK(m6800_tx_tick)
{
    m6800_state *cpustate = (m6800_state *)ptr;

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
				}
				break;

			case M6800_SERIAL_STOP:
				// send stop bit '1'
				cpustate->tx = 1;

				CHECK_IRQ_LINES(cpustate);

				cpustate->txbits = M6800_SERIAL_START;
				break;

			default:
				// send data bit '0' or '1'
				cpustate->tx = cpustate->tsr & 0x01;

				// shift transmit register
				cpustate->tsr >>= 1;

				cpustate->txbits++;
				break;
			}
			break;
		}
	}

	m6800_tx(cpustate, cpustate->tx);
}

static TIMER_CALLBACK(m6800_rx_tick)
{
    m6800_state *cpustate = (m6800_state *)ptr;

	if (cpustate->trcsr & M6800_TRCSR_RE)
	{
		if (cpustate->trcsr & M6800_TRCSR_WU)
		{
			// wait for 10 bits of '1'

			if (m6800_rx(cpustate) == 1)
			{
				cpustate->rxbits++;

				if (cpustate->rxbits == 10)
				{
					cpustate->trcsr &= ~M6800_TRCSR_WU;
					cpustate->rxbits = M6800_SERIAL_START;
				}
			}
			else
			{
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
				}
				break;

			case M6800_SERIAL_STOP:
				if (m6800_rx(cpustate) == 1)
				{
					if (cpustate->trcsr & M6800_TRCSR_RDRF)
					{
						// overrun error

						cpustate->trcsr |= M6800_TRCSR_ORFE;

						CHECK_IRQ_LINES(cpustate);
					}
					else
					{
						if (!(cpustate->trcsr & M6800_TRCSR_ORFE))
						{
							// transfer data into receive register
							cpustate->rdr = cpustate->rsr;

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

					CHECK_IRQ_LINES(cpustate);
				}

				cpustate->rxbits = M6800_SERIAL_START;
				break;

			default:
				// shift receive register
				cpustate->rsr >>= 1;

				// receive bit into register
				cpustate->rsr |= (m6800_rx(cpustate) << 7);

				cpustate->rxbits++;
				break;
			}
		}
	}
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static void state_register(m6800_state *cpustate, const char *type)
{
	state_save_register_device_item(cpustate->device, 0, cpustate->ppc.w.l);
	state_save_register_device_item(cpustate->device, 0, cpustate->pc.w.l);
	state_save_register_device_item(cpustate->device, 0, cpustate->s.w.l);
	state_save_register_device_item(cpustate->device, 0, cpustate->x.w.l);
	state_save_register_device_item(cpustate->device, 0, cpustate->d.w.l);
	state_save_register_device_item(cpustate->device, 0, cpustate->cc);
	state_save_register_device_item(cpustate->device, 0, cpustate->wai_state);
	state_save_register_device_item(cpustate->device, 0, cpustate->nmi_state);
	state_save_register_device_item(cpustate->device, 0, cpustate->nmi_pending);
	state_save_register_device_item_array(cpustate->device, 0, cpustate->irq_state);
	state_save_register_device_item(cpustate->device, 0, cpustate->ic_eddge);

	state_save_register_device_item(cpustate->device, 0, cpustate->port1_ddr);
	state_save_register_device_item(cpustate->device, 0, cpustate->port2_ddr);
	state_save_register_device_item(cpustate->device, 0, cpustate->port3_ddr);
	state_save_register_device_item(cpustate->device, 0, cpustate->port4_ddr);
	state_save_register_device_item(cpustate->device, 0, cpustate->port1_data);
	state_save_register_device_item(cpustate->device, 0, cpustate->port2_data);
	state_save_register_device_item(cpustate->device, 0, cpustate->port3_data);
	state_save_register_device_item(cpustate->device, 0, cpustate->port4_data);
	state_save_register_device_item(cpustate->device, 0, cpustate->tcsr);
	state_save_register_device_item(cpustate->device, 0, cpustate->pending_tcsr);
	state_save_register_device_item(cpustate->device, 0, cpustate->irq2);
	state_save_register_device_item(cpustate->device, 0, cpustate->ram_ctrl);

	state_save_register_device_item(cpustate->device, 0, cpustate->counter.d);
	state_save_register_device_item(cpustate->device, 0, cpustate->output_compare.d);
	state_save_register_device_item(cpustate->device, 0, cpustate->input_capture);
	state_save_register_device_item(cpustate->device, 0, cpustate->timer_over.d);

	state_save_register_device_item(cpustate->device, 0, cpustate->clock);
	state_save_register_device_item(cpustate->device, 0, cpustate->trcsr);
	state_save_register_device_item(cpustate->device, 0, cpustate->rmcr);
	state_save_register_device_item(cpustate->device, 0, cpustate->rdr);
	state_save_register_device_item(cpustate->device, 0, cpustate->tdr);
	state_save_register_device_item(cpustate->device, 0, cpustate->rsr);
	state_save_register_device_item(cpustate->device, 0, cpustate->tsr);
	state_save_register_device_item(cpustate->device, 0, cpustate->rxbits);
	state_save_register_device_item(cpustate->device, 0, cpustate->txbits);
	state_save_register_device_item(cpustate->device, 0, cpustate->txstate);
	state_save_register_device_item(cpustate->device, 0, cpustate->trcsr_read);
	state_save_register_device_item(cpustate->device, 0, cpustate->tx);
}

static CPU_INIT( m6800 )
{
	m6800_state *cpustate = get_safe_token(device);

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

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

	SEI;				/* IRQ disabled */
	PCD = RM16(cpustate,  0xfffe );

	cpustate->wai_state = 0;
	cpustate->nmi_state = 0;
	cpustate->nmi_pending = 0;
	cpustate->irq_state[M6800_IRQ_LINE] = 0;
	cpustate->irq_state[M6800_TIN_LINE] = 0;
	cpustate->ic_eddge = 0;

	cpustate->port1_ddr = 0x00;
	cpustate->port2_ddr = 0x00;
	/* TODO: on reset port 2 should be read to determine the operating mode (bits 0-2) */
	cpustate->tcsr = 0x00;
	cpustate->pending_tcsr = 0x00;
	cpustate->irq2 = 0;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	cpustate->ram_ctrl |= 0x40;

	cpustate->trcsr = M6800_TRCSR_TDRE;
	cpustate->rmcr = 0;
	if (cpustate->m6800_rx_timer) timer_enable(cpustate->m6800_rx_timer, 0);
	if (cpustate->m6800_tx_timer) timer_enable(cpustate->m6800_tx_timer, 0);
	cpustate->txstate = M6800_TX_STATE_INIT;
	cpustate->txbits = cpustate->rxbits = 0;
	cpustate->trcsr_read = 0;
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
	if (irqline == INPUT_LINE_NMI)
	{
		if (!cpustate->nmi_state && state != CLEAR_LINE)
			cpustate->nmi_pending = TRUE;
		cpustate->nmi_state = state;
	}
	else
	{
		//int eddge;

		LOG(("M6800 '%s' set_irq_line %d,%d\n", cpustate->device->tag, irqline, state));
		cpustate->irq_state[irqline] = state;

		if (irqline == M6800_TIN_LINE && state != cpustate->irq_state[irqline])
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
 * Execute one instruction
 ****************************************************************************/

INLINE void m6800_execute_one(m6800_state *cpustate, UINT8 ireg)
{
	switch( ireg )
	{
		case 0x00: illegal(cpustate); break;
		case 0x01: nop(cpustate); break;
		case 0x02: illegal(cpustate); break;
		case 0x03: illegal(cpustate); break;
		case 0x04: illegal(cpustate); break;
		case 0x05: illegal(cpustate); break;
		case 0x06: tap(cpustate); break;
		case 0x07: tpa(cpustate); break;
		case 0x08: inx(cpustate); break;
		case 0x09: dex(cpustate); break;
		case 0x0A: CLV; break;
		case 0x0B: SEV; break;
		case 0x0C: CLC; break;
		case 0x0D: SEC; break;
		case 0x0E: cli(cpustate); break;
		case 0x0F: sei(cpustate); break;
		case 0x10: sba(cpustate); break;
		case 0x11: cba(cpustate); break;
		case 0x12: illegal(cpustate); break;
		case 0x13: illegal(cpustate); break;
		case 0x14: illegal(cpustate); break;
		case 0x15: illegal(cpustate); break;
		case 0x16: tab(cpustate); break;
		case 0x17: tba(cpustate); break;
		case 0x18: illegal(cpustate); break;
		case 0x19: daa(cpustate); break;
		case 0x1a: illegal(cpustate); break;
		case 0x1b: aba(cpustate); break;
		case 0x1c: illegal(cpustate); break;
		case 0x1d: illegal(cpustate); break;
		case 0x1e: illegal(cpustate); break;
		case 0x1f: illegal(cpustate); break;
		case 0x20: bra(cpustate); break;
		case 0x21: brn(cpustate); break;
		case 0x22: bhi(cpustate); break;
		case 0x23: bls(cpustate); break;
		case 0x24: bcc(cpustate); break;
		case 0x25: bcs(cpustate); break;
		case 0x26: bne(cpustate); break;
		case 0x27: beq(cpustate); break;
		case 0x28: bvc(cpustate); break;
		case 0x29: bvs(cpustate); break;
		case 0x2a: bpl(cpustate); break;
		case 0x2b: bmi(cpustate); break;
		case 0x2c: bge(cpustate); break;
		case 0x2d: blt(cpustate); break;
		case 0x2e: bgt(cpustate); break;
		case 0x2f: ble(cpustate); break;
		case 0x30: tsx(cpustate); break;
		case 0x31: ins(cpustate); break;
		case 0x32: pula(cpustate); break;
		case 0x33: pulb(cpustate); break;
		case 0x34: des(cpustate); break;
		case 0x35: txs(cpustate); break;
		case 0x36: psha(cpustate); break;
		case 0x37: pshb(cpustate); break;
		case 0x38: illegal(cpustate); break;
		case 0x39: rts(cpustate); break;
		case 0x3a: illegal(cpustate); break;
		case 0x3b: rti(cpustate); break;
		case 0x3c: illegal(cpustate); break;
		case 0x3d: illegal(cpustate); break;
		case 0x3e: wai(cpustate); break;
		case 0x3f: swi(cpustate); break;
		case 0x40: nega(cpustate); break;
		case 0x41: illegal(cpustate); break;
		case 0x42: illegal(cpustate); break;
		case 0x43: coma(cpustate); break;
		case 0x44: lsra(cpustate); break;
		case 0x45: illegal(cpustate); break;
		case 0x46: rora(cpustate); break;
		case 0x47: asra(cpustate); break;
		case 0x48: asla(cpustate); break;
		case 0x49: rola(cpustate); break;
		case 0x4a: deca(cpustate); break;
		case 0x4b: illegal(cpustate); break;
		case 0x4c: inca(cpustate); break;
		case 0x4d: tsta(cpustate); break;
		case 0x4e: illegal(cpustate); break;
		case 0x4f: clra(cpustate); break;
		case 0x50: negb(cpustate); break;
		case 0x51: illegal(cpustate); break;
		case 0x52: illegal(cpustate); break;
		case 0x53: comb(cpustate); break;
		case 0x54: lsrb(cpustate); break;
		case 0x55: illegal(cpustate); break;
		case 0x56: rorb(cpustate); break;
		case 0x57: asrb(cpustate); break;
		case 0x58: aslb(cpustate); break;
		case 0x59: rolb(cpustate); break;
		case 0x5a: decb(cpustate); break;
		case 0x5b: illegal(cpustate); break;
		case 0x5c: incb(cpustate); break;
		case 0x5d: tstb(cpustate); break;
		case 0x5e: illegal(cpustate); break;
		case 0x5f: clrb(cpustate); break;
		case 0x60: neg_ix(cpustate); break;
		case 0x61: illegal(cpustate); break;
		case 0x62: illegal(cpustate); break;
		case 0x63: com_ix(cpustate); break;
		case 0x64: lsr_ix(cpustate); break;
		case 0x65: illegal(cpustate); break;
		case 0x66: ror_ix(cpustate); break;
		case 0x67: asr_ix(cpustate); break;
		case 0x68: asl_ix(cpustate); break;
		case 0x69: rol_ix(cpustate); break;
		case 0x6a: dec_ix(cpustate); break;
		case 0x6b: illegal(cpustate); break;
		case 0x6c: inc_ix(cpustate); break;
		case 0x6d: tst_ix(cpustate); break;
		case 0x6e: jmp_ix(cpustate); break;
		case 0x6f: clr_ix(cpustate); break;
		case 0x70: neg_ex(cpustate); break;
		case 0x71: illegal(cpustate); break;
		case 0x72: illegal(cpustate); break;
		case 0x73: com_ex(cpustate); break;
		case 0x74: lsr_ex(cpustate); break;
		case 0x75: illegal(cpustate); break;
		case 0x76: ror_ex(cpustate); break;
		case 0x77: asr_ex(cpustate); break;
		case 0x78: asl_ex(cpustate); break;
		case 0x79: rol_ex(cpustate); break;
		case 0x7a: dec_ex(cpustate); break;
		case 0x7b: illegal(cpustate); break;
		case 0x7c: inc_ex(cpustate); break;
		case 0x7d: tst_ex(cpustate); break;
		case 0x7e: jmp_ex(cpustate); break;
		case 0x7f: clr_ex(cpustate); break;
		case 0x80: suba_im(cpustate); break;
		case 0x81: cmpa_im(cpustate); break;
		case 0x82: sbca_im(cpustate); break;
		case 0x83: illegal(cpustate); break;
		case 0x84: anda_im(cpustate); break;
		case 0x85: bita_im(cpustate); break;
		case 0x86: lda_im(cpustate); break;
		case 0x87: sta_im(cpustate); break;
		case 0x88: eora_im(cpustate); break;
		case 0x89: adca_im(cpustate); break;
		case 0x8a: ora_im(cpustate); break;
		case 0x8b: adda_im(cpustate); break;
		case 0x8c: cmpx_im(cpustate); break;
		case 0x8d: bsr(cpustate); break;
		case 0x8e: lds_im(cpustate); break;
		case 0x8f: sts_im(cpustate); /* orthogonality */ break;
		case 0x90: suba_di(cpustate); break;
		case 0x91: cmpa_di(cpustate); break;
		case 0x92: sbca_di(cpustate); break;
		case 0x93: illegal(cpustate); break;
		case 0x94: anda_di(cpustate); break;
		case 0x95: bita_di(cpustate); break;
		case 0x96: lda_di(cpustate); break;
		case 0x97: sta_di(cpustate); break;
		case 0x98: eora_di(cpustate); break;
		case 0x99: adca_di(cpustate); break;
		case 0x9a: ora_di(cpustate); break;
		case 0x9b: adda_di(cpustate); break;
		case 0x9c: cmpx_di(cpustate); break;
		case 0x9d: jsr_di(cpustate); break;
		case 0x9e: lds_di(cpustate); break;
		case 0x9f: sts_di(cpustate); break;
		case 0xa0: suba_ix(cpustate); break;
		case 0xa1: cmpa_ix(cpustate); break;
		case 0xa2: sbca_ix(cpustate); break;
		case 0xa3: illegal(cpustate); break;
		case 0xa4: anda_ix(cpustate); break;
		case 0xa5: bita_ix(cpustate); break;
		case 0xa6: lda_ix(cpustate); break;
		case 0xa7: sta_ix(cpustate); break;
		case 0xa8: eora_ix(cpustate); break;
		case 0xa9: adca_ix(cpustate); break;
		case 0xaa: ora_ix(cpustate); break;
		case 0xab: adda_ix(cpustate); break;
		case 0xac: cmpx_ix(cpustate); break;
		case 0xad: jsr_ix(cpustate); break;
		case 0xae: lds_ix(cpustate); break;
		case 0xaf: sts_ix(cpustate); break;
		case 0xb0: suba_ex(cpustate); break;
		case 0xb1: cmpa_ex(cpustate); break;
		case 0xb2: sbca_ex(cpustate); break;
		case 0xb3: illegal(cpustate); break;
		case 0xb4: anda_ex(cpustate); break;
		case 0xb5: bita_ex(cpustate); break;
		case 0xb6: lda_ex(cpustate); break;
		case 0xb7: sta_ex(cpustate); break;
		case 0xb8: eora_ex(cpustate); break;
		case 0xb9: adca_ex(cpustate); break;
		case 0xba: ora_ex(cpustate); break;
		case 0xbb: adda_ex(cpustate); break;
		case 0xbc: cmpx_ex(cpustate); break;
		case 0xbd: jsr_ex(cpustate); break;
		case 0xbe: lds_ex(cpustate); break;
		case 0xbf: sts_ex(cpustate); break;
		case 0xc0: subb_im(cpustate); break;
		case 0xc1: cmpb_im(cpustate); break;
		case 0xc2: sbcb_im(cpustate); break;
		case 0xc3: illegal(cpustate); break;
		case 0xc4: andb_im(cpustate); break;
		case 0xc5: bitb_im(cpustate); break;
		case 0xc6: ldb_im(cpustate); break;
		case 0xc7: stb_im(cpustate); break;
		case 0xc8: eorb_im(cpustate); break;
		case 0xc9: adcb_im(cpustate); break;
		case 0xca: orb_im(cpustate); break;
		case 0xcb: addb_im(cpustate); break;
		case 0xcc: illegal(cpustate); break;
		case 0xcd: illegal(cpustate); break;
		case 0xce: ldx_im(cpustate); break;
		case 0xcf: stx_im(cpustate); break;
		case 0xd0: subb_di(cpustate); break;
		case 0xd1: cmpb_di(cpustate); break;
		case 0xd2: sbcb_di(cpustate); break;
		case 0xd3: illegal(cpustate); break;
		case 0xd4: andb_di(cpustate); break;
		case 0xd5: bitb_di(cpustate); break;
		case 0xd6: ldb_di(cpustate); break;
		case 0xd7: stb_di(cpustate); break;
		case 0xd8: eorb_di(cpustate); break;
		case 0xd9: adcb_di(cpustate); break;
		case 0xda: orb_di(cpustate); break;
		case 0xdb: addb_di(cpustate); break;
		case 0xdc: illegal(cpustate); break;
		case 0xdd: illegal(cpustate); break;
		case 0xde: ldx_di(cpustate); break;
		case 0xdf: stx_di(cpustate); break;
		case 0xe0: subb_ix(cpustate); break;
		case 0xe1: cmpb_ix(cpustate); break;
		case 0xe2: sbcb_ix(cpustate); break;
		case 0xe3: illegal(cpustate); break;
		case 0xe4: andb_ix(cpustate); break;
		case 0xe5: bitb_ix(cpustate); break;
		case 0xe6: ldb_ix(cpustate); break;
		case 0xe7: stb_ix(cpustate); break;
		case 0xe8: eorb_ix(cpustate); break;
		case 0xe9: adcb_ix(cpustate); break;
		case 0xea: orb_ix(cpustate); break;
		case 0xeb: addb_ix(cpustate); break;
		case 0xec: illegal(cpustate); break;
		case 0xed: illegal(cpustate); break;
		case 0xee: ldx_ix(cpustate); break;
		case 0xef: stx_ix(cpustate); break;
		case 0xf0: subb_ex(cpustate); break;
		case 0xf1: cmpb_ex(cpustate); break;
		case 0xf2: sbcb_ex(cpustate); break;
		case 0xf3: illegal(cpustate); break;
		case 0xf4: andb_ex(cpustate); break;
		case 0xf5: bitb_ex(cpustate); break;
		case 0xf6: ldb_ex(cpustate); break;
		case 0xf7: stb_ex(cpustate); break;
		case 0xf8: eorb_ex(cpustate); break;
		case 0xf9: adcb_ex(cpustate); break;
		case 0xfa: orb_ex(cpustate); break;
		case 0xfb: addb_ex(cpustate); break;
		case 0xfc: addx_ex(cpustate); break;
		case 0xfd: illegal(cpustate); break;
		case 0xfe: ldx_ex(cpustate); break;
		case 0xff: stx_ex(cpustate); break;
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static CPU_EXECUTE( m6800 )
{
	m6800_state *cpustate = get_safe_token(device);
	UINT8 ireg;
	cpustate->icount = cycles;

	CHECK_IRQ_LINES(cpustate); /* HJB 990417 */

	CLEANUP_COUNTERS();

	do
	{
		if( cpustate->wai_state & M6800_WAI )
		{
			EAT_CYCLES;
		}
		else
		{
			pPPC = pPC;
			debugger_instruction_hook(device, PCD);
			ireg=M_RDOP(PCD);
			PC++;
			m6800_execute_one(cpustate, ireg);
			increment_counter(cpustate, cycles_6800[ireg]);
		}
	} while( cpustate->icount>0 );

	return cycles - cpustate->icount;
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

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	cpustate->clock = device->clock / 4;
	cpustate->m6800_rx_timer = timer_alloc(device->machine, m6800_rx_tick, cpustate);
	cpustate->m6800_tx_timer = timer_alloc(device->machine, m6800_tx_tick, cpustate);

	state_register(cpustate, "m6801");
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

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

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

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	cpustate->clock = device->clock / 4;
	cpustate->m6800_rx_timer = timer_alloc(device->machine, m6800_rx_tick, cpustate);
	cpustate->m6800_tx_timer = timer_alloc(device->machine, m6800_tx_tick, cpustate);

	state_register(cpustate, "m6803");
}

/****************************************************************************
 * Execute one instruction
 ****************************************************************************/

INLINE void m6803_execute_one(m6800_state *cpustate, UINT8 ireg)
{
	switch( ireg )
	{
		case 0x00: illegal(cpustate); break;
		case 0x01: nop(cpustate); break;
		case 0x02: illegal(cpustate); break;
		case 0x03: illegal(cpustate); break;
		case 0x04: lsrd(cpustate); /* 6803 only */; break;
		case 0x05: asld(cpustate); /* 6803 only */; break;
		case 0x06: tap(cpustate); break;
		case 0x07: tpa(cpustate); break;
		case 0x08: inx(cpustate); break;
		case 0x09: dex(cpustate); break;
		case 0x0A: CLV; break;
		case 0x0B: SEV; break;
		case 0x0C: CLC; break;
		case 0x0D: SEC; break;
		case 0x0E: cli(cpustate); break;
		case 0x0F: sei(cpustate); break;
		case 0x10: sba(cpustate); break;
		case 0x11: cba(cpustate); break;
		case 0x12: illegal(cpustate); break;
		case 0x13: illegal(cpustate); break;
		case 0x14: illegal(cpustate); break;
		case 0x15: illegal(cpustate); break;
		case 0x16: tab(cpustate); break;
		case 0x17: tba(cpustate); break;
		case 0x18: illegal(cpustate); break;
		case 0x19: daa(cpustate); break;
		case 0x1a: illegal(cpustate); break;
		case 0x1b: aba(cpustate); break;
		case 0x1c: illegal(cpustate); break;
		case 0x1d: illegal(cpustate); break;
		case 0x1e: illegal(cpustate); break;
		case 0x1f: illegal(cpustate); break;
		case 0x20: bra(cpustate); break;
		case 0x21: brn(cpustate); break;
		case 0x22: bhi(cpustate); break;
		case 0x23: bls(cpustate); break;
		case 0x24: bcc(cpustate); break;
		case 0x25: bcs(cpustate); break;
		case 0x26: bne(cpustate); break;
		case 0x27: beq(cpustate); break;
		case 0x28: bvc(cpustate); break;
		case 0x29: bvs(cpustate); break;
		case 0x2a: bpl(cpustate); break;
		case 0x2b: bmi(cpustate); break;
		case 0x2c: bge(cpustate); break;
		case 0x2d: blt(cpustate); break;
		case 0x2e: bgt(cpustate); break;
		case 0x2f: ble(cpustate); break;
		case 0x30: tsx(cpustate); break;
		case 0x31: ins(cpustate); break;
		case 0x32: pula(cpustate); break;
		case 0x33: pulb(cpustate); break;
		case 0x34: des(cpustate); break;
		case 0x35: txs(cpustate); break;
		case 0x36: psha(cpustate); break;
		case 0x37: pshb(cpustate); break;
		case 0x38: pulx(cpustate); /* 6803 only */ break;
		case 0x39: rts(cpustate); break;
		case 0x3a: abx(cpustate); /* 6803 only */ break;
		case 0x3b: rti(cpustate); break;
		case 0x3c: pshx(cpustate); /* 6803 only */ break;
		case 0x3d: mul(cpustate); /* 6803 only */ break;
		case 0x3e: wai(cpustate); break;
		case 0x3f: swi(cpustate); break;
		case 0x40: nega(cpustate); break;
		case 0x41: illegal(cpustate); break;
		case 0x42: illegal(cpustate); break;
		case 0x43: coma(cpustate); break;
		case 0x44: lsra(cpustate); break;
		case 0x45: illegal(cpustate); break;
		case 0x46: rora(cpustate); break;
		case 0x47: asra(cpustate); break;
		case 0x48: asla(cpustate); break;
		case 0x49: rola(cpustate); break;
		case 0x4a: deca(cpustate); break;
		case 0x4b: illegal(cpustate); break;
		case 0x4c: inca(cpustate); break;
		case 0x4d: tsta(cpustate); break;
		case 0x4e: illegal(cpustate); break;
		case 0x4f: clra(cpustate); break;
		case 0x50: negb(cpustate); break;
		case 0x51: illegal(cpustate); break;
		case 0x52: illegal(cpustate); break;
		case 0x53: comb(cpustate); break;
		case 0x54: lsrb(cpustate); break;
		case 0x55: illegal(cpustate); break;
		case 0x56: rorb(cpustate); break;
		case 0x57: asrb(cpustate); break;
		case 0x58: aslb(cpustate); break;
		case 0x59: rolb(cpustate); break;
		case 0x5a: decb(cpustate); break;
		case 0x5b: illegal(cpustate); break;
		case 0x5c: incb(cpustate); break;
		case 0x5d: tstb(cpustate); break;
		case 0x5e: illegal(cpustate); break;
		case 0x5f: clrb(cpustate); break;
		case 0x60: neg_ix(cpustate); break;
		case 0x61: illegal(cpustate); break;
		case 0x62: illegal(cpustate); break;
		case 0x63: com_ix(cpustate); break;
		case 0x64: lsr_ix(cpustate); break;
		case 0x65: illegal(cpustate); break;
		case 0x66: ror_ix(cpustate); break;
		case 0x67: asr_ix(cpustate); break;
		case 0x68: asl_ix(cpustate); break;
		case 0x69: rol_ix(cpustate); break;
		case 0x6a: dec_ix(cpustate); break;
		case 0x6b: illegal(cpustate); break;
		case 0x6c: inc_ix(cpustate); break;
		case 0x6d: tst_ix(cpustate); break;
		case 0x6e: jmp_ix(cpustate); break;
		case 0x6f: clr_ix(cpustate); break;
		case 0x70: neg_ex(cpustate); break;
		case 0x71: illegal(cpustate); break;
		case 0x72: illegal(cpustate); break;
		case 0x73: com_ex(cpustate); break;
		case 0x74: lsr_ex(cpustate); break;
		case 0x75: illegal(cpustate); break;
		case 0x76: ror_ex(cpustate); break;
		case 0x77: asr_ex(cpustate); break;
		case 0x78: asl_ex(cpustate); break;
		case 0x79: rol_ex(cpustate); break;
		case 0x7a: dec_ex(cpustate); break;
		case 0x7b: illegal(cpustate); break;
		case 0x7c: inc_ex(cpustate); break;
		case 0x7d: tst_ex(cpustate); break;
		case 0x7e: jmp_ex(cpustate); break;
		case 0x7f: clr_ex(cpustate); break;
		case 0x80: suba_im(cpustate); break;
		case 0x81: cmpa_im(cpustate); break;
		case 0x82: sbca_im(cpustate); break;
		case 0x83: subd_im(cpustate); /* 6803 only */ break;
		case 0x84: anda_im(cpustate); break;
		case 0x85: bita_im(cpustate); break;
		case 0x86: lda_im(cpustate); break;
		case 0x87: sta_im(cpustate); break;
		case 0x88: eora_im(cpustate); break;
		case 0x89: adca_im(cpustate); break;
		case 0x8a: ora_im(cpustate); break;
		case 0x8b: adda_im(cpustate); break;
		case 0x8c: cpx_im(cpustate); /* 6803 difference */ break;
		case 0x8d: bsr(cpustate); break;
		case 0x8e: lds_im(cpustate); break;
		case 0x8f: sts_im(cpustate); /* orthogonality */ break;
		case 0x90: suba_di(cpustate); break;
		case 0x91: cmpa_di(cpustate); break;
		case 0x92: sbca_di(cpustate); break;
		case 0x93: subd_di(cpustate); /* 6803 only */ break;
		case 0x94: anda_di(cpustate); break;
		case 0x95: bita_di(cpustate); break;
		case 0x96: lda_di(cpustate); break;
		case 0x97: sta_di(cpustate); break;
		case 0x98: eora_di(cpustate); break;
		case 0x99: adca_di(cpustate); break;
		case 0x9a: ora_di(cpustate); break;
		case 0x9b: adda_di(cpustate); break;
		case 0x9c: cpx_di(cpustate); /* 6803 difference */ break;
		case 0x9d: jsr_di(cpustate); break;
		case 0x9e: lds_di(cpustate); break;
		case 0x9f: sts_di(cpustate); break;
		case 0xa0: suba_ix(cpustate); break;
		case 0xa1: cmpa_ix(cpustate); break;
		case 0xa2: sbca_ix(cpustate); break;
		case 0xa3: subd_ix(cpustate); /* 6803 only */ break;
		case 0xa4: anda_ix(cpustate); break;
		case 0xa5: bita_ix(cpustate); break;
		case 0xa6: lda_ix(cpustate); break;
		case 0xa7: sta_ix(cpustate); break;
		case 0xa8: eora_ix(cpustate); break;
		case 0xa9: adca_ix(cpustate); break;
		case 0xaa: ora_ix(cpustate); break;
		case 0xab: adda_ix(cpustate); break;
		case 0xac: cpx_ix(cpustate); /* 6803 difference */ break;
		case 0xad: jsr_ix(cpustate); break;
		case 0xae: lds_ix(cpustate); break;
		case 0xaf: sts_ix(cpustate); break;
		case 0xb0: suba_ex(cpustate); break;
		case 0xb1: cmpa_ex(cpustate); break;
		case 0xb2: sbca_ex(cpustate); break;
		case 0xb3: subd_ex(cpustate); /* 6803 only */ break;
		case 0xb4: anda_ex(cpustate); break;
		case 0xb5: bita_ex(cpustate); break;
		case 0xb6: lda_ex(cpustate); break;
		case 0xb7: sta_ex(cpustate); break;
		case 0xb8: eora_ex(cpustate); break;
		case 0xb9: adca_ex(cpustate); break;
		case 0xba: ora_ex(cpustate); break;
		case 0xbb: adda_ex(cpustate); break;
		case 0xbc: cpx_ex(cpustate); /* 6803 difference */ break;
		case 0xbd: jsr_ex(cpustate); break;
		case 0xbe: lds_ex(cpustate); break;
		case 0xbf: sts_ex(cpustate); break;
		case 0xc0: subb_im(cpustate); break;
		case 0xc1: cmpb_im(cpustate); break;
		case 0xc2: sbcb_im(cpustate); break;
		case 0xc3: addd_im(cpustate); /* 6803 only */ break;
		case 0xc4: andb_im(cpustate); break;
		case 0xc5: bitb_im(cpustate); break;
		case 0xc6: ldb_im(cpustate); break;
		case 0xc7: stb_im(cpustate); break;
		case 0xc8: eorb_im(cpustate); break;
		case 0xc9: adcb_im(cpustate); break;
		case 0xca: orb_im(cpustate); break;
		case 0xcb: addb_im(cpustate); break;
		case 0xcc: ldd_im(cpustate); /* 6803 only */ break;
		case 0xcd: std_im(cpustate); /* 6803 only -- orthogonality */ break;
		case 0xce: ldx_im(cpustate); break;
		case 0xcf: stx_im(cpustate); break;
		case 0xd0: subb_di(cpustate); break;
		case 0xd1: cmpb_di(cpustate); break;
		case 0xd2: sbcb_di(cpustate); break;
		case 0xd3: addd_di(cpustate); /* 6803 only */ break;
		case 0xd4: andb_di(cpustate); break;
		case 0xd5: bitb_di(cpustate); break;
		case 0xd6: ldb_di(cpustate); break;
		case 0xd7: stb_di(cpustate); break;
		case 0xd8: eorb_di(cpustate); break;
		case 0xd9: adcb_di(cpustate); break;
		case 0xda: orb_di(cpustate); break;
		case 0xdb: addb_di(cpustate); break;
		case 0xdc: ldd_di(cpustate); /* 6803 only */ break;
		case 0xdd: std_di(cpustate); /* 6803 only */ break;
		case 0xde: ldx_di(cpustate); break;
		case 0xdf: stx_di(cpustate); break;
		case 0xe0: subb_ix(cpustate); break;
		case 0xe1: cmpb_ix(cpustate); break;
		case 0xe2: sbcb_ix(cpustate); break;
		case 0xe3: addd_ix(cpustate); /* 6803 only */ break;
		case 0xe4: andb_ix(cpustate); break;
		case 0xe5: bitb_ix(cpustate); break;
		case 0xe6: ldb_ix(cpustate); break;
		case 0xe7: stb_ix(cpustate); break;
		case 0xe8: eorb_ix(cpustate); break;
		case 0xe9: adcb_ix(cpustate); break;
		case 0xea: orb_ix(cpustate); break;
		case 0xeb: addb_ix(cpustate); break;
		case 0xec: ldd_ix(cpustate); /* 6803 only */ break;
		case 0xed: std_ix(cpustate); /* 6803 only */ break;
		case 0xee: ldx_ix(cpustate); break;
		case 0xef: stx_ix(cpustate); break;
		case 0xf0: subb_ex(cpustate); break;
		case 0xf1: cmpb_ex(cpustate); break;
		case 0xf2: sbcb_ex(cpustate); break;
		case 0xf3: addd_ex(cpustate); /* 6803 only */ break;
		case 0xf4: andb_ex(cpustate); break;
		case 0xf5: bitb_ex(cpustate); break;
		case 0xf6: ldb_ex(cpustate); break;
		case 0xf7: stb_ex(cpustate); break;
		case 0xf8: eorb_ex(cpustate); break;
		case 0xf9: adcb_ex(cpustate); break;
		case 0xfa: orb_ex(cpustate); break;
		case 0xfb: addb_ex(cpustate); break;
		case 0xfc: ldd_ex(cpustate); /* 6803 only */ break;
		case 0xfd: std_ex(cpustate); /* 6803 only */ break;
		case 0xfe: ldx_ex(cpustate); break;
		case 0xff: stx_ex(cpustate); break;
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static CPU_EXECUTE( m6803 )
{
	m6800_state *cpustate = get_safe_token(device);
	UINT8 ireg;
	cpustate->icount = cycles;

	CHECK_IRQ_LINES(cpustate); /* HJB 990417 */

	CLEANUP_COUNTERS();

	do
	{
		if( cpustate->wai_state & M6800_WAI )
		{
			EAT_CYCLES;
		}
		else
		{
			pPPC = pPC;
			debugger_instruction_hook(device, PCD);
			ireg=M_RDOP(PCD);
			PC++;
			m6803_execute_one(cpustate, ireg);
			increment_counter(cpustate, cycles_6803[ireg]);
		}
	} while( cpustate->icount>0 );

	return cycles - cpustate->icount;
}


static READ8_HANDLER( m6803_internal_registers_r );
static WRITE8_HANDLER( m6803_internal_registers_w );

static ADDRESS_MAP_START(m6803_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(m6803_internal_registers_r, m6803_internal_registers_w)
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

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	state_register(cpustate, "m6808");
}

/****************************************************************************
 * HD63701 similiar to the M6800
 ****************************************************************************/

static CPU_INIT( hd63701 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_HD63701;
	cpustate->insn = hd63701_insn;
	cpustate->cycles = cycles_63701;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	cpustate->clock = device->clock / 4;
	cpustate->m6800_rx_timer = timer_alloc(device->machine, m6800_rx_tick, cpustate);
	cpustate->m6800_tx_timer = timer_alloc(device->machine, m6800_tx_tick, cpustate);

	state_register(cpustate, "hd63701");
}

/****************************************************************************
 * Execute one instruction
 ****************************************************************************/

INLINE void hd63071_execute_one(m6800_state *cpustate, UINT8 ireg)
{
	switch( ireg )
	{
		case 0x00: trap(cpustate); break;
		case 0x01: nop(cpustate); break;
		case 0x02: trap(cpustate); break;
		case 0x03: trap(cpustate); break;
		case 0x04: lsrd(cpustate); /* 6803 only */; break;
		case 0x05: asld(cpustate); /* 6803 only */; break;
		case 0x06: tap(cpustate); break;
		case 0x07: tpa(cpustate); break;
		case 0x08: inx(cpustate); break;
		case 0x09: dex(cpustate); break;
		case 0x0A: CLV; break;
		case 0x0B: SEV; break;
		case 0x0C: CLC; break;
		case 0x0D: SEC; break;
		case 0x0E: cli(cpustate); break;
		case 0x0F: sei(cpustate); break;
		case 0x10: sba(cpustate); break;
		case 0x11: cba(cpustate); break;
		case 0x12: undoc1(cpustate); break;
		case 0x13: undoc2(cpustate); break;
		case 0x14: trap(cpustate); break;
		case 0x15: trap(cpustate); break;
		case 0x16: tab(cpustate); break;
		case 0x17: tba(cpustate); break;
		case 0x18: xgdx(cpustate); /* HD63701YO only */; break;
		case 0x19: daa(cpustate); break;
		case 0x1a: slp(cpustate); break;
		case 0x1b: aba(cpustate); break;
		case 0x1c: trap(cpustate); break;
		case 0x1d: trap(cpustate); break;
		case 0x1e: trap(cpustate); break;
		case 0x1f: trap(cpustate); break;
		case 0x20: bra(cpustate); break;
		case 0x21: brn(cpustate); break;
		case 0x22: bhi(cpustate); break;
		case 0x23: bls(cpustate); break;
		case 0x24: bcc(cpustate); break;
		case 0x25: bcs(cpustate); break;
		case 0x26: bne(cpustate); break;
		case 0x27: beq(cpustate); break;
		case 0x28: bvc(cpustate); break;
		case 0x29: bvs(cpustate); break;
		case 0x2a: bpl(cpustate); break;
		case 0x2b: bmi(cpustate); break;
		case 0x2c: bge(cpustate); break;
		case 0x2d: blt(cpustate); break;
		case 0x2e: bgt(cpustate); break;
		case 0x2f: ble(cpustate); break;
		case 0x30: tsx(cpustate); break;
		case 0x31: ins(cpustate); break;
		case 0x32: pula(cpustate); break;
		case 0x33: pulb(cpustate); break;
		case 0x34: des(cpustate); break;
		case 0x35: txs(cpustate); break;
		case 0x36: psha(cpustate); break;
		case 0x37: pshb(cpustate); break;
		case 0x38: pulx(cpustate); /* 6803 only */ break;
		case 0x39: rts(cpustate); break;
		case 0x3a: abx(cpustate); /* 6803 only */ break;
		case 0x3b: rti(cpustate); break;
		case 0x3c: pshx(cpustate); /* 6803 only */ break;
		case 0x3d: mul(cpustate); /* 6803 only */ break;
		case 0x3e: wai(cpustate); break;
		case 0x3f: swi(cpustate); break;
		case 0x40: nega(cpustate); break;
		case 0x41: trap(cpustate); break;
		case 0x42: trap(cpustate); break;
		case 0x43: coma(cpustate); break;
		case 0x44: lsra(cpustate); break;
		case 0x45: trap(cpustate); break;
		case 0x46: rora(cpustate); break;
		case 0x47: asra(cpustate); break;
		case 0x48: asla(cpustate); break;
		case 0x49: rola(cpustate); break;
		case 0x4a: deca(cpustate); break;
		case 0x4b: trap(cpustate); break;
		case 0x4c: inca(cpustate); break;
		case 0x4d: tsta(cpustate); break;
		case 0x4e: trap(cpustate); break;
		case 0x4f: clra(cpustate); break;
		case 0x50: negb(cpustate); break;
		case 0x51: trap(cpustate); break;
		case 0x52: trap(cpustate); break;
		case 0x53: comb(cpustate); break;
		case 0x54: lsrb(cpustate); break;
		case 0x55: trap(cpustate); break;
		case 0x56: rorb(cpustate); break;
		case 0x57: asrb(cpustate); break;
		case 0x58: aslb(cpustate); break;
		case 0x59: rolb(cpustate); break;
		case 0x5a: decb(cpustate); break;
		case 0x5b: trap(cpustate); break;
		case 0x5c: incb(cpustate); break;
		case 0x5d: tstb(cpustate); break;
		case 0x5e: trap(cpustate); break;
		case 0x5f: clrb(cpustate); break;
		case 0x60: neg_ix(cpustate); break;
		case 0x61: aim_ix(cpustate); /* HD63701YO only */; break;
		case 0x62: oim_ix(cpustate); /* HD63701YO only */; break;
		case 0x63: com_ix(cpustate); break;
		case 0x64: lsr_ix(cpustate); break;
		case 0x65: eim_ix(cpustate); /* HD63701YO only */; break;
		case 0x66: ror_ix(cpustate); break;
		case 0x67: asr_ix(cpustate); break;
		case 0x68: asl_ix(cpustate); break;
		case 0x69: rol_ix(cpustate); break;
		case 0x6a: dec_ix(cpustate); break;
		case 0x6b: tim_ix(cpustate); /* HD63701YO only */; break;
		case 0x6c: inc_ix(cpustate); break;
		case 0x6d: tst_ix(cpustate); break;
		case 0x6e: jmp_ix(cpustate); break;
		case 0x6f: clr_ix(cpustate); break;
		case 0x70: neg_ex(cpustate); break;
		case 0x71: aim_di(cpustate); /* HD63701YO only */; break;
		case 0x72: oim_di(cpustate); /* HD63701YO only */; break;
		case 0x73: com_ex(cpustate); break;
		case 0x74: lsr_ex(cpustate); break;
		case 0x75: eim_di(cpustate); /* HD63701YO only */; break;
		case 0x76: ror_ex(cpustate); break;
		case 0x77: asr_ex(cpustate); break;
		case 0x78: asl_ex(cpustate); break;
		case 0x79: rol_ex(cpustate); break;
		case 0x7a: dec_ex(cpustate); break;
		case 0x7b: tim_di(cpustate); /* HD63701YO only */; break;
		case 0x7c: inc_ex(cpustate); break;
		case 0x7d: tst_ex(cpustate); break;
		case 0x7e: jmp_ex(cpustate); break;
		case 0x7f: clr_ex(cpustate); break;
		case 0x80: suba_im(cpustate); break;
		case 0x81: cmpa_im(cpustate); break;
		case 0x82: sbca_im(cpustate); break;
		case 0x83: subd_im(cpustate); /* 6803 only */ break;
		case 0x84: anda_im(cpustate); break;
		case 0x85: bita_im(cpustate); break;
		case 0x86: lda_im(cpustate); break;
		case 0x87: sta_im(cpustate); break;
		case 0x88: eora_im(cpustate); break;
		case 0x89: adca_im(cpustate); break;
		case 0x8a: ora_im(cpustate); break;
		case 0x8b: adda_im(cpustate); break;
		case 0x8c: cpx_im(cpustate); /* 6803 difference */ break;
		case 0x8d: bsr(cpustate); break;
		case 0x8e: lds_im(cpustate); break;
		case 0x8f: sts_im(cpustate); /* orthogonality */ break;
		case 0x90: suba_di(cpustate); break;
		case 0x91: cmpa_di(cpustate); break;
		case 0x92: sbca_di(cpustate); break;
		case 0x93: subd_di(cpustate); /* 6803 only */ break;
		case 0x94: anda_di(cpustate); break;
		case 0x95: bita_di(cpustate); break;
		case 0x96: lda_di(cpustate); break;
		case 0x97: sta_di(cpustate); break;
		case 0x98: eora_di(cpustate); break;
		case 0x99: adca_di(cpustate); break;
		case 0x9a: ora_di(cpustate); break;
		case 0x9b: adda_di(cpustate); break;
		case 0x9c: cpx_di(cpustate); /* 6803 difference */ break;
		case 0x9d: jsr_di(cpustate); break;
		case 0x9e: lds_di(cpustate); break;
		case 0x9f: sts_di(cpustate); break;
		case 0xa0: suba_ix(cpustate); break;
		case 0xa1: cmpa_ix(cpustate); break;
		case 0xa2: sbca_ix(cpustate); break;
		case 0xa3: subd_ix(cpustate); /* 6803 only */ break;
		case 0xa4: anda_ix(cpustate); break;
		case 0xa5: bita_ix(cpustate); break;
		case 0xa6: lda_ix(cpustate); break;
		case 0xa7: sta_ix(cpustate); break;
		case 0xa8: eora_ix(cpustate); break;
		case 0xa9: adca_ix(cpustate); break;
		case 0xaa: ora_ix(cpustate); break;
		case 0xab: adda_ix(cpustate); break;
		case 0xac: cpx_ix(cpustate); /* 6803 difference */ break;
		case 0xad: jsr_ix(cpustate); break;
		case 0xae: lds_ix(cpustate); break;
		case 0xaf: sts_ix(cpustate); break;
		case 0xb0: suba_ex(cpustate); break;
		case 0xb1: cmpa_ex(cpustate); break;
		case 0xb2: sbca_ex(cpustate); break;
		case 0xb3: subd_ex(cpustate); /* 6803 only */ break;
		case 0xb4: anda_ex(cpustate); break;
		case 0xb5: bita_ex(cpustate); break;
		case 0xb6: lda_ex(cpustate); break;
		case 0xb7: sta_ex(cpustate); break;
		case 0xb8: eora_ex(cpustate); break;
		case 0xb9: adca_ex(cpustate); break;
		case 0xba: ora_ex(cpustate); break;
		case 0xbb: adda_ex(cpustate); break;
		case 0xbc: cpx_ex(cpustate); /* 6803 difference */ break;
		case 0xbd: jsr_ex(cpustate); break;
		case 0xbe: lds_ex(cpustate); break;
		case 0xbf: sts_ex(cpustate); break;
		case 0xc0: subb_im(cpustate); break;
		case 0xc1: cmpb_im(cpustate); break;
		case 0xc2: sbcb_im(cpustate); break;
		case 0xc3: addd_im(cpustate); /* 6803 only */ break;
		case 0xc4: andb_im(cpustate); break;
		case 0xc5: bitb_im(cpustate); break;
		case 0xc6: ldb_im(cpustate); break;
		case 0xc7: stb_im(cpustate); break;
		case 0xc8: eorb_im(cpustate); break;
		case 0xc9: adcb_im(cpustate); break;
		case 0xca: orb_im(cpustate); break;
		case 0xcb: addb_im(cpustate); break;
		case 0xcc: ldd_im(cpustate); /* 6803 only */ break;
		case 0xcd: std_im(cpustate); /* 6803 only -- orthogonality */ break;
		case 0xce: ldx_im(cpustate); break;
		case 0xcf: stx_im(cpustate); break;
		case 0xd0: subb_di(cpustate); break;
		case 0xd1: cmpb_di(cpustate); break;
		case 0xd2: sbcb_di(cpustate); break;
		case 0xd3: addd_di(cpustate); /* 6803 only */ break;
		case 0xd4: andb_di(cpustate); break;
		case 0xd5: bitb_di(cpustate); break;
		case 0xd6: ldb_di(cpustate); break;
		case 0xd7: stb_di(cpustate); break;
		case 0xd8: eorb_di(cpustate); break;
		case 0xd9: adcb_di(cpustate); break;
		case 0xda: orb_di(cpustate); break;
		case 0xdb: addb_di(cpustate); break;
		case 0xdc: ldd_di(cpustate); /* 6803 only */ break;
		case 0xdd: std_di(cpustate); /* 6803 only */ break;
		case 0xde: ldx_di(cpustate); break;
		case 0xdf: stx_di(cpustate); break;
		case 0xe0: subb_ix(cpustate); break;
		case 0xe1: cmpb_ix(cpustate); break;
		case 0xe2: sbcb_ix(cpustate); break;
		case 0xe3: addd_ix(cpustate); /* 6803 only */ break;
		case 0xe4: andb_ix(cpustate); break;
		case 0xe5: bitb_ix(cpustate); break;
		case 0xe6: ldb_ix(cpustate); break;
		case 0xe7: stb_ix(cpustate); break;
		case 0xe8: eorb_ix(cpustate); break;
		case 0xe9: adcb_ix(cpustate); break;
		case 0xea: orb_ix(cpustate); break;
		case 0xeb: addb_ix(cpustate); break;
		case 0xec: ldd_ix(cpustate); /* 6803 only */ break;
		case 0xed: std_ix(cpustate); /* 6803 only */ break;
		case 0xee: ldx_ix(cpustate); break;
		case 0xef: stx_ix(cpustate); break;
		case 0xf0: subb_ex(cpustate); break;
		case 0xf1: cmpb_ex(cpustate); break;
		case 0xf2: sbcb_ex(cpustate); break;
		case 0xf3: addd_ex(cpustate); /* 6803 only */ break;
		case 0xf4: andb_ex(cpustate); break;
		case 0xf5: bitb_ex(cpustate); break;
		case 0xf6: ldb_ex(cpustate); break;
		case 0xf7: stb_ex(cpustate); break;
		case 0xf8: eorb_ex(cpustate); break;
		case 0xf9: adcb_ex(cpustate); break;
		case 0xfa: orb_ex(cpustate); break;
		case 0xfb: addb_ex(cpustate); break;
		case 0xfc: ldd_ex(cpustate); /* 6803 only */ break;
		case 0xfd: std_ex(cpustate); /* 6803 only */ break;
		case 0xfe: ldx_ex(cpustate); break;
		case 0xff: stx_ex(cpustate); break;
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static CPU_EXECUTE( hd63701 )
{
	m6800_state *cpustate = get_safe_token(device);
	UINT8 ireg;
	cpustate->icount = cycles;

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
			hd63071_execute_one(cpustate, ireg);
			increment_counter(cpustate, cycles_63701[ireg]);
		}
	} while( cpustate->icount>0 );

	return cycles - cpustate->icount;
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

static READ8_HANDLER( m6803_internal_registers_r );
static WRITE8_HANDLER( m6803_internal_registers_w );

READ8_HANDLER( hd63701_internal_registers_r )
{
	return m6803_internal_registers_r(space, offset);
}

WRITE8_HANDLER( hd63701_internal_registers_w )
{
	m6803_internal_registers_w(space, offset,data);
}

/****************************************************************************
 * NSC-8105 similiar to the M6800, but the opcodes are scrambled and there
 * is at least one new opcode ($fc)
 ****************************************************************************/
static CPU_INIT( nsc8105 )
{
	m6800_state *cpustate = get_safe_token(device);
	//  cpustate->subtype = SUBTYPE_NSC8105;
	cpustate->device = device;

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	cpustate->insn = nsc8105_insn;
	cpustate->cycles = cycles_nsc8105;
	state_register(cpustate, "nsc8105");
}

/****************************************************************************
 * Execute one instruction
 ****************************************************************************/

INLINE void nsc8105_execute_one(m6800_state *cpustate, UINT8 ireg)
{
	switch( ireg )
	{
		case 0x00: illegal(cpustate); break;
		case 0x01: illegal(cpustate); break;
		case 0x02: nop(cpustate); break;
		case 0x03: illegal(cpustate); break;
		case 0x04: illegal(cpustate); break;
		case 0x05: tap(cpustate); break;
		case 0x06: illegal(cpustate); break;
		case 0x07: tpa(cpustate); break;
		case 0x08: inx(cpustate); break;
		case 0x09: CLV; break;
		case 0x0a: dex(cpustate); break;
		case 0x0b: SEV; break;
		case 0x0c: CLC; break;
		case 0x0d: cli(cpustate); break;
		case 0x0e: SEC; break;
		case 0x0f: sei(cpustate); break;
		case 0x10: sba(cpustate); break;
		case 0x11: illegal(cpustate); break;
		case 0x12: cba(cpustate); break;
		case 0x13: illegal(cpustate); break;
		case 0x14: illegal(cpustate); break;
		case 0x15: tab(cpustate); break;
		case 0x16: illegal(cpustate); break;
		case 0x17: tba(cpustate); break;
		case 0x18: illegal(cpustate); break;
		case 0x19: illegal(cpustate); break;
		case 0x1a: daa(cpustate); break;
		case 0x1b: aba(cpustate); break;
		case 0x1c: illegal(cpustate); break;
		case 0x1d: illegal(cpustate); break;
		case 0x1e: illegal(cpustate); break;
		case 0x1f: illegal(cpustate); break;
		case 0x20: bra(cpustate); break;
		case 0x21: bhi(cpustate); break;
		case 0x22: brn(cpustate); break;
		case 0x23: bls(cpustate); break;
		case 0x24: bcc(cpustate); break;
		case 0x25: bne(cpustate); break;
		case 0x26: bcs(cpustate); break;
		case 0x27: beq(cpustate); break;
		case 0x28: bvc(cpustate); break;
		case 0x29: bpl(cpustate); break;
		case 0x2a: bvs(cpustate); break;
		case 0x2b: bmi(cpustate); break;
		case 0x2c: bge(cpustate); break;
		case 0x2d: bgt(cpustate); break;
		case 0x2e: blt(cpustate); break;
		case 0x2f: ble(cpustate); break;
		case 0x30: tsx(cpustate); break;
		case 0x31: pula(cpustate); break;
		case 0x32: ins(cpustate); break;
		case 0x33: pulb(cpustate); break;
		case 0x34: des(cpustate); break;
		case 0x35: psha(cpustate); break;
		case 0x36: txs(cpustate); break;
		case 0x37: pshb(cpustate); break;
		case 0x38: illegal(cpustate); break;
		case 0x39: illegal(cpustate); break;
		case 0x3a: rts(cpustate); break;
		case 0x3b: rti(cpustate); break;
		case 0x3c: illegal(cpustate); break;
		case 0x3d: wai(cpustate); break;
		case 0x3e: illegal(cpustate); break;
		case 0x3f: swi(cpustate); break;
		case 0x40: suba_im(cpustate); break;
		case 0x41: sbca_im(cpustate); break;
		case 0x42: cmpa_im(cpustate); break;
		case 0x43: illegal(cpustate); break;
		case 0x44: anda_im(cpustate); break;
		case 0x45: lda_im(cpustate); break;
		case 0x46: bita_im(cpustate); break;
		case 0x47: sta_im(cpustate); break;
		case 0x48: eora_im(cpustate); break;
		case 0x49: ora_im(cpustate); break;
		case 0x4a: adca_im(cpustate); break;
		case 0x4b: adda_im(cpustate); break;
		case 0x4c: cmpx_im(cpustate); break;
		case 0x4d: lds_im(cpustate); break;
		case 0x4e: bsr(cpustate); break;
		case 0x4f: sts_im(cpustate); /* orthogonality */ break;
		case 0x50: suba_di(cpustate); break;
		case 0x51: sbca_di(cpustate); break;
		case 0x52: cmpa_di(cpustate); break;
		case 0x53: illegal(cpustate); break;
		case 0x54: anda_di(cpustate); break;
		case 0x55: lda_di(cpustate); break;
		case 0x56: bita_di(cpustate); break;
		case 0x57: sta_di(cpustate); break;
		case 0x58: eora_di(cpustate); break;
		case 0x59: ora_di(cpustate); break;
		case 0x5a: adca_di(cpustate); break;
		case 0x5b: adda_di(cpustate); break;
		case 0x5c: cmpx_di(cpustate); break;
		case 0x5d: lds_di(cpustate); break;
		case 0x5e: jsr_di(cpustate); break;
		case 0x5f: sts_di(cpustate); break;
		case 0x60: suba_ix(cpustate); break;
		case 0x61: sbca_ix(cpustate); break;
		case 0x62: cmpa_ix(cpustate); break;
		case 0x63: illegal(cpustate); break;
		case 0x64: anda_ix(cpustate); break;
		case 0x65: lda_ix(cpustate); break;
		case 0x66: bita_ix(cpustate); break;
		case 0x67: sta_ix(cpustate); break;
		case 0x68: eora_ix(cpustate); break;
		case 0x69: ora_ix(cpustate); break;
		case 0x6a: adca_ix(cpustate); break;
		case 0x6b: adda_ix(cpustate); break;
		case 0x6c: cmpx_ix(cpustate); break;
		case 0x6d: lds_ix(cpustate); break;
		case 0x6e: jsr_ix(cpustate); break;
		case 0x6f: sts_ix(cpustate); break;
		case 0x70: suba_ex(cpustate); break;
		case 0x71: sbca_ex(cpustate); break;
		case 0x72: cmpa_ex(cpustate); break;
		case 0x73: illegal(cpustate); break;
		case 0x74: anda_ex(cpustate); break;
		case 0x75: lda_ex(cpustate); break;
		case 0x76: bita_ex(cpustate); break;
		case 0x77: sta_ex(cpustate); break;
		case 0x78: eora_ex(cpustate); break;
		case 0x79: ora_ex(cpustate); break;
		case 0x7a: adca_ex(cpustate); break;
		case 0x7b: adda_ex(cpustate); break;
		case 0x7c: cmpx_ex(cpustate); break;
		case 0x7d: lds_ex(cpustate); break;
		case 0x7e: jsr_ex(cpustate); break;
		case 0x7f: sts_ex(cpustate); break;
		case 0x80: nega(cpustate); break;
		case 0x81: illegal(cpustate); break;
		case 0x82: illegal(cpustate); break;
		case 0x83: coma(cpustate); break;
		case 0x84: lsra(cpustate); break;
		case 0x85: rora(cpustate); break;
		case 0x86: illegal(cpustate); break;
		case 0x87: asra(cpustate); break;
		case 0x88: asla(cpustate); break;
		case 0x89: deca(cpustate); break;
		case 0x8a: rola(cpustate); break;
		case 0x8b: illegal(cpustate); break;
		case 0x8c: inca(cpustate); break;
		case 0x8d: illegal(cpustate); break;
		case 0x8e: tsta(cpustate); break;
		case 0x8f: clra(cpustate); break;
		case 0x90: negb(cpustate); break;
		case 0x91: illegal(cpustate); break;
		case 0x92: illegal(cpustate); break;
		case 0x93: comb(cpustate); break;
		case 0x94: lsrb(cpustate); break;
		case 0x95: rorb(cpustate); break;
		case 0x96: illegal(cpustate); break;
		case 0x97: asrb(cpustate); break;
		case 0x98: aslb(cpustate); break;
		case 0x99: decb(cpustate); break;
		case 0x9a: rolb(cpustate); break;
		case 0x9b: illegal(cpustate); break;
		case 0x9c: incb(cpustate); break;
		case 0x9d: illegal(cpustate); break;
		case 0x9e: tstb(cpustate); break;
		case 0x9f: clrb(cpustate); break;
		case 0xa0: neg_ix(cpustate); break;
		case 0xa1: illegal(cpustate); break;
		case 0xa2: illegal(cpustate); break;
		case 0xa3: com_ix(cpustate); break;
		case 0xa4: lsr_ix(cpustate); break;
		case 0xa5: ror_ix(cpustate); break;
		case 0xa6: illegal(cpustate); break;
		case 0xa7: asr_ix(cpustate); break;
		case 0xa8: asl_ix(cpustate); break;
		case 0xa9: dec_ix(cpustate); break;
		case 0xaa: rol_ix(cpustate); break;
		case 0xab: illegal(cpustate); break;
		case 0xac: inc_ix(cpustate); break;
		case 0xad: jmp_ix(cpustate); break;
		case 0xae: tst_ix(cpustate); break;
		case 0xaf: clr_ix(cpustate); break;
		case 0xb0: neg_ex(cpustate); break;
		case 0xb1: illegal(cpustate); break;
		case 0xb2: illegal(cpustate); break;
		case 0xb3: com_ex(cpustate); break;
		case 0xb4: lsr_ex(cpustate); break;
		case 0xb5: ror_ex(cpustate); break;
		case 0xb6: illegal(cpustate); break;
		case 0xb7: asr_ex(cpustate); break;
		case 0xb8: asl_ex(cpustate); break;
		case 0xb9: dec_ex(cpustate); break;
		case 0xba: rol_ex(cpustate); break;
		case 0xbb: illegal(cpustate); break;
		case 0xbc: inc_ex(cpustate); break;
		case 0xbd: jmp_ex(cpustate); break;
		case 0xbe: tst_ex(cpustate); break;
		case 0xbf: clr_ex(cpustate); break;
		case 0xc0: subb_im(cpustate); break;
		case 0xc1: sbcb_im(cpustate); break;
		case 0xc2: cmpb_im(cpustate); break;
		case 0xc3: illegal(cpustate); break;
		case 0xc4: andb_im(cpustate); break;
		case 0xc5: ldb_im(cpustate); break;
		case 0xc6: bitb_im(cpustate); break;
		case 0xc7: stb_im(cpustate); break;
		case 0xc8: eorb_im(cpustate); break;
		case 0xc9: orb_im(cpustate); break;
		case 0xca: adcb_im(cpustate); break;
		case 0xcb: addb_im(cpustate); break;
		case 0xcc: illegal(cpustate); break;
		case 0xcd: ldx_im(cpustate); break;
		case 0xce: illegal(cpustate); break;
		case 0xcf: stx_im(cpustate); break;
		case 0xd0: subb_di(cpustate); break;
		case 0xd1: sbcb_di(cpustate); break;
		case 0xd2: cmpb_di(cpustate); break;
		case 0xd3: illegal(cpustate); break;
		case 0xd4: andb_di(cpustate); break;
		case 0xd5: ldb_di(cpustate); break;
		case 0xd6: bitb_di(cpustate); break;
		case 0xd7: stb_di(cpustate); break;
		case 0xd8: eorb_di(cpustate); break;
		case 0xd9: orb_di(cpustate); break;
		case 0xda: adcb_di(cpustate); break;
		case 0xdb: addb_di(cpustate); break;
		case 0xdc: illegal(cpustate); break;
		case 0xdd: ldx_di(cpustate); break;
		case 0xde: illegal(cpustate); break;
		case 0xdf: stx_di(cpustate); break;
		case 0xe0: subb_ix(cpustate); break;
		case 0xe1: sbcb_ix(cpustate); break;
		case 0xe2: cmpb_ix(cpustate); break;
		case 0xe3: illegal(cpustate); break;
		case 0xe4: andb_ix(cpustate); break;
		case 0xe5: ldb_ix(cpustate); break;
		case 0xe6: bitb_ix(cpustate); break;
		case 0xe7: stb_ix(cpustate); break;
		case 0xe8: eorb_ix(cpustate); break;
		case 0xe9: orb_ix(cpustate); break;
		case 0xea: adcb_ix(cpustate); break;
		case 0xeb: addb_ix(cpustate); break;
		case 0xec: adcx_im(cpustate); break; /* NSC8105 only */
		case 0xed: ldx_ix(cpustate); break;
		case 0xee: illegal(cpustate); break;
		case 0xef: stx_ix(cpustate); break;
		case 0xf0: subb_ex(cpustate); break;
		case 0xf1: sbcb_ex(cpustate); break;
		case 0xf2: cmpb_ex(cpustate); break;
		case 0xf3: illegal(cpustate); break;
		case 0xf4: andb_ex(cpustate); break;
		case 0xf5: ldb_ex(cpustate); break;
		case 0xf6: bitb_ex(cpustate); break;
		case 0xf7: stb_ex(cpustate); break;
		case 0xf8: eorb_ex(cpustate); break;
		case 0xf9: orb_ex(cpustate); break;
		case 0xfa: adcb_ex(cpustate); break;
		case 0xfb: addb_ex(cpustate); break;
		case 0xfc: addx_ex(cpustate); break;
		case 0xfd: ldx_ex(cpustate); break;
		case 0xfe: illegal(cpustate); break;
		case 0xff: stx_ex(cpustate); break;
	}
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static CPU_EXECUTE( nsc8105 )
{
	m6800_state *cpustate = get_safe_token(device);
	UINT8 ireg;
	cpustate->icount = cycles;

	CHECK_IRQ_LINES(cpustate); /* HJB 990417 */

	CLEANUP_COUNTERS();

	do
	{
		if( cpustate->wai_state & M6800_WAI )
		{
			EAT_CYCLES;
		}
		else
		{
			pPPC = pPC;
			debugger_instruction_hook(device, PCD);
			ireg=M_RDOP(PCD);
			PC++;
			nsc8105_execute_one(cpustate, ireg);
			increment_counter(cpustate, cycles_nsc8105[ireg]);
		}
	} while( cpustate->icount>0 );

	return cycles - cpustate->icount;
}



static READ8_HANDLER( m6803_internal_registers_r )
{
	m6800_state *cpustate = get_safe_token(space->cpu);

	switch (offset)
	{
		case 0x00:
			return cpustate->port1_ddr;
		case 0x01:
			return cpustate->port2_ddr;
		case 0x02:
			return (memory_read_byte_8be(cpustate->io, M6803_PORT1) & (cpustate->port1_ddr ^ 0xff))
					| (cpustate->port1_data & cpustate->port1_ddr);
		case 0x03:
			return (memory_read_byte_8be(cpustate->io, M6803_PORT2) & (cpustate->port2_ddr ^ 0xff))
					| (cpustate->port2_data & cpustate->port2_ddr);
		case 0x04:
			return cpustate->port3_ddr;
		case 0x05:
			return cpustate->port4_ddr;
		case 0x06:
			return (memory_read_byte_8be(cpustate->io, M6803_PORT3) & (cpustate->port3_ddr ^ 0xff))
					| (cpustate->port3_data & cpustate->port3_ddr);
		case 0x07:
			return (memory_read_byte_8be(cpustate->io, M6803_PORT4) & (cpustate->port4_ddr ^ 0xff))
					| (cpustate->port4_data & cpustate->port4_ddr);
		case 0x08:
			cpustate->pending_tcsr = 0;
			return cpustate->tcsr;
		case 0x09:
			if(!(cpustate->pending_tcsr&TCSR_TOF))
			{
				cpustate->tcsr &= ~TCSR_TOF;
				MODIFIED_tcsr;
			}
			return cpustate->counter.b.h;
		case 0x0a:
			return cpustate->counter.b.l;
		case 0x0b:
			if(!(cpustate->pending_tcsr&TCSR_OCF))
			{
				cpustate->tcsr &= ~TCSR_OCF;
				MODIFIED_tcsr;
			}
			return cpustate->output_compare.b.h;
		case 0x0c:
			if(!(cpustate->pending_tcsr&TCSR_OCF))
			{
				cpustate->tcsr &= ~TCSR_OCF;
				MODIFIED_tcsr;
			}
			return cpustate->output_compare.b.l;
		case 0x0d:
			if(!(cpustate->pending_tcsr&TCSR_ICF))
			{
				cpustate->tcsr &= ~TCSR_ICF;
				MODIFIED_tcsr;
			}
			return (cpustate->input_capture >> 0) & 0xff;
		case 0x0e:
			return (cpustate->input_capture >> 8) & 0xff;
		case 0x0f:
			logerror("CPU '%s' PC %04x: warning - read from unsupported register %02x\n",space->cpu->tag,cpu_get_pc(space->cpu),offset);
			return 0;
		case 0x10:
			return cpustate->rmcr;
		case 0x11:
			cpustate->trcsr_read = 1;
			return cpustate->trcsr;
		case 0x12:
			if (cpustate->trcsr_read)
			{
				cpustate->trcsr_read = 0;
				cpustate->trcsr = cpustate->trcsr & 0x3f;
			}
			return cpustate->rdr;
		case 0x13:
			return cpustate->tdr;
		case 0x14:
			logerror("CPU '%s' PC %04x: read RAM control register\n",space->cpu->tag,cpu_get_pc(space->cpu));
			return cpustate->ram_ctrl;
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
		default:
			logerror("CPU '%s' PC %04x: warning - read from reserved internal register %02x\n",space->cpu->tag,cpu_get_pc(space->cpu),offset);
			return 0;
	}
}

static WRITE8_HANDLER( m6803_internal_registers_w )
{
	m6800_state *cpustate = get_safe_token(space->cpu);

	switch (offset)
	{
		case 0x00:
			if (cpustate->port1_ddr != data)
			{
				cpustate->port1_ddr = data;
				if(cpustate->port1_ddr == 0xff)
					memory_write_byte_8be(cpustate->io, M6803_PORT1,cpustate->port1_data);
				else
					memory_write_byte_8be(cpustate->io, M6803_PORT1,(cpustate->port1_data & cpustate->port1_ddr)
						| (memory_read_byte_8be(cpustate->io, M6803_PORT1) & (cpustate->port1_ddr ^ 0xff)));
			}
			break;
		case 0x01:
			if (cpustate->port2_ddr != data)
			{
				cpustate->port2_ddr = data;
				if(cpustate->port2_ddr == 0xff)
					memory_write_byte_8be(cpustate->io, M6803_PORT2,cpustate->port2_data);
				else
					memory_write_byte_8be(cpustate->io, M6803_PORT2,(cpustate->port2_data & cpustate->port2_ddr)
						| (memory_read_byte_8be(cpustate->io, M6803_PORT2) & (cpustate->port2_ddr ^ 0xff)));

				if (cpustate->port2_ddr & 2)
					logerror("CPU '%s' PC %04x: warning - port 2 bit 1 set as output (OLVL) - not supported\n",space->cpu->tag,cpu_get_pc(space->cpu));
			}
			break;
		case 0x02:
			cpustate->port1_data = data;
			if(cpustate->port1_ddr == 0xff)
				memory_write_byte_8be(cpustate->io, M6803_PORT1,cpustate->port1_data);
			else
				memory_write_byte_8be(cpustate->io, M6803_PORT1,(cpustate->port1_data & cpustate->port1_ddr)
					| (memory_read_byte_8be(cpustate->io, M6803_PORT1) & (cpustate->port1_ddr ^ 0xff)));
			break;
		case 0x03:
			if (cpustate->trcsr & M6800_TRCSR_TE)
			{
				cpustate->port2_data = (data & 0xef) | (cpustate->tx << 4);
			}
			else
			{
				cpustate->port2_data = data;
			}
			if(cpustate->port2_ddr == 0xff)
				memory_write_byte_8be(cpustate->io, M6803_PORT2,cpustate->port2_data);
			else
				memory_write_byte_8be(cpustate->io, M6803_PORT2,(cpustate->port2_data & cpustate->port2_ddr)
					| (memory_read_byte_8be(cpustate->io, M6803_PORT2) & (cpustate->port2_ddr ^ 0xff)));
			break;
		case 0x04:
			if (cpustate->port3_ddr != data)
			{
				cpustate->port3_ddr = data;
				if(cpustate->port3_ddr == 0xff)
					memory_write_byte_8be(cpustate->io, M6803_PORT3,cpustate->port3_data);
				else
					memory_write_byte_8be(cpustate->io, M6803_PORT3,(cpustate->port3_data & cpustate->port3_ddr)
						| (memory_read_byte_8be(cpustate->io, M6803_PORT3) & (cpustate->port3_ddr ^ 0xff)));
			}
			break;
		case 0x05:
			if (cpustate->port4_ddr != data)
			{
				cpustate->port4_ddr = data;
				if(cpustate->port4_ddr == 0xff)
					memory_write_byte_8be(cpustate->io, M6803_PORT4,cpustate->port4_data);
				else
					memory_write_byte_8be(cpustate->io, M6803_PORT4,(cpustate->port4_data & cpustate->port4_ddr)
						| (memory_read_byte_8be(cpustate->io, M6803_PORT4) & (cpustate->port4_ddr ^ 0xff)));
			}
			break;
		case 0x06:
			cpustate->port3_data = data;
			if(cpustate->port3_ddr == 0xff)
				memory_write_byte_8be(cpustate->io, M6803_PORT3,cpustate->port3_data);
			else
				memory_write_byte_8be(cpustate->io, M6803_PORT3,(cpustate->port3_data & cpustate->port3_ddr)
					| (memory_read_byte_8be(cpustate->io, M6803_PORT3) & (cpustate->port3_ddr ^ 0xff)));
			break;
		case 0x07:
			cpustate->port4_data = data;
			if(cpustate->port4_ddr == 0xff)
				memory_write_byte_8be(cpustate->io, M6803_PORT4,cpustate->port4_data);
			else
				memory_write_byte_8be(cpustate->io, M6803_PORT4,(cpustate->port4_data & cpustate->port4_ddr)
					| (memory_read_byte_8be(cpustate->io, M6803_PORT4) & (cpustate->port4_ddr ^ 0xff)));
			break;
		case 0x08:
			cpustate->tcsr = data;
			cpustate->pending_tcsr &= cpustate->tcsr;
			MODIFIED_tcsr;
			if( !(CC & 0x10) )
				m6800_check_irq2(cpustate);
			break;
		case 0x09:
			cpustate->latch09 = data & 0xff;	/* 6301 only */
			CT  = 0xfff8;
			TOH = CTH;
			MODIFIED_counters;
			break;
		case 0x0a:	/* 6301 only */
			CT = (cpustate->latch09 << 8) | (data & 0xff);
			TOH = CTH;
			MODIFIED_counters;
			break;
		case 0x0b:
			if( cpustate->output_compare.b.h != data)
			{
				cpustate->output_compare.b.h = data;
				MODIFIED_counters;
			}
			break;
		case 0x0c:
			if( cpustate->output_compare.b.l != data)
			{
				cpustate->output_compare.b.l = data;
				MODIFIED_counters;
			}
			break;
		case 0x0d:
		case 0x0e:
		case 0x12:
			logerror("CPU '%s' PC %04x: warning - write %02x to read only internal register %02x\n",space->cpu->tag,cpu_get_pc(space->cpu),data,offset);
			break;
		case 0x0f:
			logerror("CPU '%s' PC %04x: warning - write %02x to unsupported internal register %02x\n",space->cpu->tag,cpu_get_pc(space->cpu),data,offset);
			break;
		case 0x10:
			cpustate->rmcr = data & 0x0f;

			switch ((cpustate->rmcr & M6800_RMCR_CC_MASK) >> 2)
			{
			case 0:
			case 3: // not implemented
				timer_enable(cpustate->m6800_rx_timer, 0);
				timer_enable(cpustate->m6800_tx_timer, 0);
				break;

			case 1:
			case 2:
				{
					int divisor = M6800_RMCR_SS[cpustate->rmcr & M6800_RMCR_SS_MASK];

					timer_adjust_periodic(cpustate->m6800_rx_timer, attotime_zero, 0, ATTOTIME_IN_HZ(cpustate->clock / divisor));
					timer_adjust_periodic(cpustate->m6800_tx_timer, attotime_zero, 0, ATTOTIME_IN_HZ(cpustate->clock / divisor));
				}
				break;
			}
			break;
		case 0x11:
			if ((data & M6800_TRCSR_TE) && !(cpustate->trcsr & M6800_TRCSR_TE))
			{
				cpustate->txstate = M6800_TX_STATE_INIT;
			}
			cpustate->trcsr = (cpustate->trcsr & 0xe0) | (data & 0x1f);
			break;
		case 0x13:
			if (cpustate->trcsr_read)
			{
				cpustate->trcsr_read = 0;
				cpustate->trcsr &= ~M6800_TRCSR_TDRE;
			}
			cpustate->tdr = data;
			break;
		case 0x14:
			logerror("CPU '%s' PC %04x: write %02x to RAM control register\n",space->cpu->tag,cpu_get_pc(space->cpu),data);
			cpustate->ram_ctrl = data;
			break;
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
		default:
			logerror("CPU '%s' PC %04x: warning - write %02x to reserved internal register %02x\n",space->cpu->tag,cpu_get_pc(space->cpu),data,offset);
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
		case CPUINFO_INT_INPUT_STATE + M6800_TIN_LINE:	set_irq_line(cpustate, M6800_TIN_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:							PC = info->i; 								break;
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
	m6800_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6800_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 12;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6800_IRQ_LINE:	info->i = cpustate->irq_state[M6800_IRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + M6800_TIN_LINE:	info->i = cpustate->irq_state[M6800_TIN_LINE]; break;
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
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6800");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 6800");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "The MAME team.");		break;

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
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6801);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6803);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6801);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6801");				break;

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
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6802");				break;

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
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6803);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6803);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6803);			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP_PROGRAM: info->internal_map8 = ADDRESS_MAP_NAME(m6803_mem); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6803");				break;

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
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6808");				break;

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
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 9;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(hd63701);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(hd63701);		break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(hd63701);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "HD63701");				break;

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
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(nsc8105);		break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(nsc8105);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "NSC8105");				break;

		default:										CPU_GET_INFO_CALL(m6800);				break;
	}
}
