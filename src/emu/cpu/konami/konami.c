/*** konami: Portable Konami cpu emulator ******************************************

    Copyright (C) The MAME Team 1999

    Based on M6809 cpu core copyright (C) John Butler 1997

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
991022 HJB:
    Tried to improve speed: Using bit7 of cycles1 as flag for multi
    byte opcodes is gone, those opcodes now instead go through opcode2().
    Inlined fetch_effective_address() into that function as well.
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990720 EHC:
    Created this file

*****************************************************************************/

#include "debugger.h"
#include "konami.h"

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif


/* Konami Registers */
typedef struct
{
	PAIR	pc; 		/* Program counter */
    PAIR    ppc;        /* Previous program counter */
    PAIR    d;          /* Accumulator a and b */
    PAIR    dp;         /* Direct Page register (page in MSB) */
	PAIR	u, s;		/* Stack pointers */
	PAIR	x, y;		/* Index registers */
    UINT8   cc;
	UINT8	ireg;		/* first opcode */
    UINT8   irq_state[2];
    int     extra_cycles; /* cycles used up by interrupts */
    int     (*irq_callback)(int irqline);
    UINT8   int_state;  /* SYNC and CWAI flags */
	UINT8	nmi_state;
	void 	(*setlines_callback)( int lines ); /* callback called when A16-A23 are set */
} konami_Regs;

/* flag bits in the cc register */
#define CC_C    0x01        /* Carry */
#define CC_V    0x02        /* Overflow */
#define CC_Z    0x04        /* Zero */
#define CC_N    0x08        /* Negative */
#define CC_II   0x10        /* Inhibit IRQ */
#define CC_H    0x20        /* Half (auxiliary) carry */
#define CC_IF   0x40        /* Inhibit FIRQ */
#define CC_E    0x80        /* entire state pushed */

/* Konami registers */
static konami_Regs konami;

#define	pPPC    konami.ppc
#define pPC 	konami.pc
#define pU		konami.u
#define pS		konami.s
#define pX		konami.x
#define pY		konami.y
#define pD		konami.d

#define	PPC		konami.ppc.w.l
#define PC  	konami.pc.w.l
#define PCD 	konami.pc.d
#define U		konami.u.w.l
#define UD		konami.u.d
#define S		konami.s.w.l
#define SD		konami.s.d
#define X		konami.x.w.l
#define XD		konami.x.d
#define Y		konami.y.w.l
#define YD		konami.y.d
#define D   	konami.d.w.l
#define A   	konami.d.b.h
#define B		konami.d.b.l
#define DP		konami.dp.b.h
#define DPD 	konami.dp.d
#define CC  	konami.cc

static PAIR ea;         /* effective address */
#define EA	ea.w.l
#define EAD ea.d

#define KONAMI_CWAI		8	/* set when CWAI is waiting for an interrupt */
#define KONAMI_SYNC		16	/* set when SYNC is waiting for an interrupt */
#define KONAMI_LDS		32	/* set when LDS occured at least once */

#define CHECK_IRQ_LINES 												\
	if( konami.irq_state[KONAMI_IRQ_LINE] != CLEAR_LINE ||				\
		konami.irq_state[KONAMI_FIRQ_LINE] != CLEAR_LINE )				\
		konami.int_state &= ~KONAMI_SYNC; /* clear SYNC flag */			\
	if( konami.irq_state[KONAMI_FIRQ_LINE]!=CLEAR_LINE && !(CC & CC_IF) ) \
	{																	\
		/* fast IRQ */													\
		/* state already saved by CWAI? */								\
		if( konami.int_state & KONAMI_CWAI )							\
		{																\
			konami.int_state &= ~KONAMI_CWAI;  /* clear CWAI */			\
			konami.extra_cycles += 7;		 /* subtract +7 cycles */	\
        }                                                               \
		else															\
		{																\
			CC &= ~CC_E;				/* save 'short' state */        \
			PUSHWORD(pPC);												\
			PUSHBYTE(CC);												\
			konami.extra_cycles += 10;	/* subtract +10 cycles */		\
		}																\
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */		\
		PCD = RM16(0xfff6); 											\
		change_pc(PC);					/* TS 971002 */ 				\
		(void)(*konami.irq_callback)(KONAMI_FIRQ_LINE);					\
	}																	\
	else																\
	if( konami.irq_state[KONAMI_IRQ_LINE]!=CLEAR_LINE && !(CC & CC_II) )\
	{																	\
		/* standard IRQ */												\
		/* state already saved by CWAI? */								\
		if( konami.int_state & KONAMI_CWAI )							\
		{																\
			konami.int_state &= ~KONAMI_CWAI;  /* clear CWAI flag */	\
			konami.extra_cycles += 7;		 /* subtract +7 cycles */	\
		}																\
		else															\
		{																\
			CC |= CC_E; 				/* save entire state */ 		\
			PUSHWORD(pPC);												\
			PUSHWORD(pU);												\
			PUSHWORD(pY);												\
			PUSHWORD(pX);												\
			PUSHBYTE(DP);												\
			PUSHBYTE(B);												\
			PUSHBYTE(A);												\
			PUSHBYTE(CC);												\
			konami.extra_cycles += 19;	 /* subtract +19 cycles */		\
		}																\
		CC |= CC_II;					/* inhibit IRQ */				\
		PCD = RM16(0xfff8); 											\
		change_pc(PC);					/* TS 971002 */ 				\
		(void)(*konami.irq_callback)(KONAMI_IRQ_LINE);					\
	}

/* public globals */
int konami_ICount=50000;
int konami_Flags;	/* flags for speed optimization (obsolete!!) */

/* these are re-defined in konami.h TO RAM, ROM or functions in memory.c */
#define RM(Addr)			KONAMI_RDMEM(Addr)
#define WM(Addr,Value)		KONAMI_WRMEM(Addr,Value)
#define ROP(Addr)			KONAMI_RDOP(Addr)
#define ROP_ARG(Addr)		KONAMI_RDOP_ARG(Addr)

#define SIGNED(a)	(UINT16)(INT16)(INT8)(a)

/* macros to access memory */
#define IMMBYTE(b)	{ b = ROP_ARG(PCD); PC++; }
#define IMMWORD(w)	{ w.d = (ROP_ARG(PCD)<<8) | ROP_ARG(PCD+1); PC += 2; }

#define PUSHBYTE(b) --S; WM(SD,b)
#define PUSHWORD(w) --S; WM(SD,w.b.l); --S; WM(SD,w.b.h)
#define PULLBYTE(b) b=KONAMI_RDMEM(SD); S++
#define PULLWORD(w) w=KONAMI_RDMEM(SD)<<8; S++; w|=KONAMI_RDMEM(SD); S++

#define PSHUBYTE(b) --U; WM(UD,b);
#define PSHUWORD(w) --U; WM(UD,w.b.l); --U; WM(UD,w.b.h)
#define PULUBYTE(b) b=KONAMI_RDMEM(UD); U++
#define PULUWORD(w) w=KONAMI_RDMEM(UD)<<8; U++; w|=KONAMI_RDMEM(UD); U++

#define CLR_HNZVC	CC&=~(CC_H|CC_N|CC_Z|CC_V|CC_C)
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

static UINT8 flags8i[256]=	 /* increment */
{
CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
CC_N|CC_V,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};
static UINT8 flags8d[256]= /* decrement */
{
CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,CC_V,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};
#define SET_FLAGS8I(a)		{CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z(a);}
#define SET_NZ16(a)			{SET_N16(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT	EAD = DPD; IMMBYTE(ea.b.l)
#define IMM8	EAD = PCD; PC++
#define IMM16	EAD = PCD; PC+=2
#define EXTENDED IMMWORD(ea)

/* macros to set status flags */
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
#define DIRBYTE(b) DIRECT; b=RM(EAD)
#define DIRWORD(w) DIRECT; w.d=RM16(EAD)
#define EXTBYTE(b) EXTENDED; b=RM(EAD)
#define EXTWORD(w) EXTENDED; w.d=RM16(EAD)

/* macros for branch instructions */
#define BRANCH(f) { 					\
	UINT8 t;							\
	IMMBYTE(t); 						\
	if( f ) 							\
	{									\
		PC += SIGNED(t);				\
		change_pc(PC);	/* TS 971002 */ \
	}									\
}

#define LBRANCH(f) {                    \
	PAIR t; 							\
	IMMWORD(t); 						\
	if( f ) 							\
	{									\
		konami_ICount -= 1;				\
		PC += t.w.l;					\
		change_pc(PC);	/* TS 971002 */ \
	}									\
}

#define NXORV  ((CC&CC_N)^((CC&CC_V)<<2))

/* macros for setting/getting registers in TFR/EXG instructions */
#define GETREG(val,reg) 				\
	switch(reg) {						\
	case 0: val = A;	break;			\
	case 1: val = B; 	break; 			\
	case 2: val = X; 	break;			\
	case 3: val = Y;	break; 			\
	case 4: val = S; 	break; /* ? */	\
	case 5: val = U;	break;			\
	default: val = 0xff; logerror("Unknown TFR/EXG idx at PC:%04x\n", PC ); break; \
}

#define SETREG(val,reg) 				\
	switch(reg) {						\
	case 0: A = val;	break;			\
	case 1: B = val;	break;			\
	case 2: X = val; 	break;			\
	case 3: Y = val;	break;			\
	case 4: S = val;	break; /* ? */	\
	case 5: U = val; 	break;			\
	default: logerror("Unknown TFR/EXG idx at PC:%04x\n", PC ); break; \
}

/* opcode timings */
static UINT8 cycles1[] =
{
	/*   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
  /*0*/  1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 5, 5, 5, 5,
  /*1*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  /*2*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  /*3*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 7, 6,
  /*4*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 3, 3, 4, 4,
  /*5*/  4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 1, 1, 1,
  /*6*/  3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5,
  /*7*/  3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5,
  /*8*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5,
  /*9*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6,
  /*A*/  2, 2, 2, 4, 4, 4, 4, 4, 2, 2, 2, 2, 3, 3, 2, 1,
  /*B*/  3, 2, 2,11,22,11, 2, 4, 3, 3, 3, 3, 3, 3, 3, 3,
  /*C*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 2,
  /*D*/  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  /*E*/  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  /*F*/  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

INLINE UINT32 RM16( UINT32 Addr )
{
	UINT32 result = RM(Addr) << 8;
	return result | RM((Addr+1)&0xffff);
}

INLINE void WM16( UINT32 Addr, PAIR *p )
{
	WM( Addr, p->b.h );
	WM( (Addr+1)&0xffff, p->b.l );
}

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
static void konami_get_context(void *dst)
{
	if( dst )
		*(konami_Regs*)dst = konami;
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
static void konami_set_context(void *src)
{
	if( src )
		konami = *(konami_Regs*)src;
    change_pc(PC);    /* TS 971002 */

    CHECK_IRQ_LINES;
}

/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
static void konami_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	konami.irq_callback = irqcallback;
}

static void konami_reset(void)
{
	konami.int_state = 0;
	konami.nmi_state = CLEAR_LINE;
	konami.irq_state[0] = CLEAR_LINE;
	konami.irq_state[0] = CLEAR_LINE;

	DPD = 0;			/* Reset direct page register */

    CC |= CC_II;        /* IRQ disabled */
    CC |= CC_IF;        /* FIRQ disabled */

	PCD = RM16(0xfffe);
    change_pc(PC);    /* TS 971002 */
}

static void konami_exit(void)
{
}

/* Generate interrupts */
/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (konami.nmi_state == state) return;
		konami.nmi_state = state;
		LOG(("KONAMI#%d set_nmi_line %d\n", cpu_getactivecpu(), state));
		if( state == CLEAR_LINE ) return;

		/* if the stack was not yet initialized */
	    if( !(konami.int_state & KONAMI_LDS) ) return;

	    konami.int_state &= ~KONAMI_SYNC;
		/* state already saved by CWAI? */
		if( konami.int_state & KONAMI_CWAI )
		{
			konami.int_state &= ~KONAMI_CWAI;
			konami.extra_cycles += 7;	/* subtract +7 cycles next time */
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
			konami.extra_cycles += 19;	/* subtract +19 cycles next time */
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD = RM16(0xfffc);
		change_pc(PC);					/* TS 971002 */
	}
	else if (irqline < 2)
	{
	    LOG(("KONAMI#%d set_irq_line %d, %d\n", cpu_getactivecpu(), irqline, state));
		konami.irq_state[irqline] = state;
		if (state == CLEAR_LINE) return;
		CHECK_IRQ_LINES;
	}
}

#if 0
/****************************************************************************
 * Save CPU state
 ****************************************************************************/
static void state_save(void *file, const char *module)
{
	int cpu = cpu_getactivecpu();
	state_save_register_item(file, cpu, module, PC);
	state_save_register_item(file, cpu, module, U);
	state_save_register_item(file, cpu, module, S);
	state_save_register_item(file, cpu, module, X);
	state_save_register_item(file, cpu, module, Y);
	state_save_register_item(file, cpu, module, DP);
	state_save_register_item(file, cpu, module, CC);
	state_save_register_item(file, cpu, module, konami.int_state);
	state_save_register_item(file, cpu, module, konami.nmi_state);
	state_save_register_item(file, cpu, module, konami.irq_state[0]);
	state_save_register_item(file, cpu, module, konami.irq_state[1]);
}
#endif

/* includes the static function prototypes and the master opcode table */
#include "konamtbl.c"

/* includes the actual opcode implementations */
#include "konamops.c"

/* execute instructions on this CPU until icount expires */
static int konami_execute(int cycles)
{
	konami_ICount = cycles - konami.extra_cycles;
	konami.extra_cycles = 0;

	if( konami.int_state & (KONAMI_CWAI | KONAMI_SYNC) )
	{
		konami_ICount = 0;
	}
	else
	{
		do
		{
			pPPC = pPC;

			CALL_MAME_DEBUG;

			konami.ireg = ROP(PCD);
			PC++;

            (*konami_main[konami.ireg])();

            konami_ICount -= cycles1[konami.ireg];

        } while( konami_ICount > 0 );

        konami_ICount -= konami.extra_cycles;
		konami.extra_cycles = 0;
    }

	return cycles - konami_ICount;
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void konami_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + KONAMI_IRQ_LINE:	set_irq_line(KONAMI_IRQ_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + KONAMI_FIRQ_LINE:set_irq_line(KONAMI_FIRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + KONAMI_PC:			PC = info->i; change_pc(PC);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + KONAMI_S:			S = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_CC:			CC = info->i; CHECK_IRQ_LINES;			break;
		case CPUINFO_INT_REGISTER + KONAMI_U:			U = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_B:			B = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_X:			X = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_Y:			Y = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_DP:			DP = info->i;							break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_KONAMI_SETLINES_CALLBACK:		konami.setlines_callback = (void (*)(int))info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void konami_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(konami);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 13;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + KONAMI_IRQ_LINE:	info->i = konami.irq_state[KONAMI_IRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + KONAMI_FIRQ_LINE:info->i = konami.irq_state[KONAMI_FIRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = konami.nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + KONAMI_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + KONAMI_S:			info->i = S;							break;
		case CPUINFO_INT_REGISTER + KONAMI_CC:			info->i = CC;							break;
		case CPUINFO_INT_REGISTER + KONAMI_U:			info->i = U;							break;
		case CPUINFO_INT_REGISTER + KONAMI_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + KONAMI_B:			info->i = B;							break;
		case CPUINFO_INT_REGISTER + KONAMI_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + KONAMI_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + KONAMI_DP:			info->i = DP;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = konami_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = konami_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = konami_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = konami_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = konami_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = konami_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = konami_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = konami_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &konami_ICount;			break;
		case CPUINFO_PTR_KONAMI_SETLINES_CALLBACK:		info->f = (genf *)konami.setlines_callback;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "KONAMI");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "KONAMI 5000x");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C) The MAME Team 1999"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				konami.cc & 0x80 ? 'E':'.',
				konami.cc & 0x40 ? 'F':'.',
                konami.cc & 0x20 ? 'H':'.',
                konami.cc & 0x10 ? 'I':'.',
                konami.cc & 0x08 ? 'N':'.',
                konami.cc & 0x04 ? 'Z':'.',
                konami.cc & 0x02 ? 'V':'.',
                konami.cc & 0x01 ? 'C':'.');
            break;

		case CPUINFO_STR_REGISTER + KONAMI_PC:			sprintf(info->s, "PC:%04X", konami.pc.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_S:			sprintf(info->s, "S:%04X", konami.s.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_CC:			sprintf(info->s, "CC:%02X", konami.cc); break;
		case CPUINFO_STR_REGISTER + KONAMI_U:			sprintf(info->s, "U:%04X", konami.u.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_A:			sprintf(info->s, "A:%02X", konami.d.b.h); break;
		case CPUINFO_STR_REGISTER + KONAMI_B:			sprintf(info->s, "B:%02X", konami.d.b.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_X:			sprintf(info->s, "X:%04X", konami.x.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_Y:			sprintf(info->s, "Y:%04X", konami.y.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_DP:			sprintf(info->s, "DP:%02X", konami.dp.b.h); break;
	}
}
