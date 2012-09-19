/*** konami: Portable Konami cpu emulator ******************************************

    Copyright Nicola Salmoria and the MAME Team

    Based on M6809 cpu core copyright John Butler

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

#include "emu.h"
#include "debugger.h"
#include "konami.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* Konami Registers */
struct konami_state
{
	PAIR	pc; 		/* Program counter */
    PAIR    ppc;        /* Previous program counter */
    PAIR    d;          /* Accumulator a and b */
    PAIR    dp;         /* Direct Page register (page in MSB) */
	PAIR	u, s;		/* Stack pointers */
	PAIR	x, y;		/* Index registers */
	PAIR	ea;
    UINT8   cc;
    UINT8	ireg;
    UINT8   irq_state[2];
	device_irq_acknowledge_callback irq_callback;
    UINT8   int_state;  /* SYNC and CWAI flags */
	UINT8	nmi_state;
	UINT8	nmi_pending;
	int		icount;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	konami_set_lines_func setlines_callback;
};

INLINE konami_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == KONAMI);
	return (konami_state *)downcast<legacy_cpu_device *>(device)->token();
}

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
#define	pPPC    cpustate->ppc
#define pPC 	cpustate->pc
#define pU		cpustate->u
#define pS		cpustate->s
#define pX		cpustate->x
#define pY		cpustate->y
#define pD		cpustate->d

#define	PPC		cpustate->ppc.w.l
#define PC  	cpustate->pc.w.l
#define PCD 	cpustate->pc.d
#define U		cpustate->u.w.l
#define UD		cpustate->u.d
#define S		cpustate->s.w.l
#define SD		cpustate->s.d
#define X		cpustate->x.w.l
#define XD		cpustate->x.d
#define Y		cpustate->y.w.l
#define YD		cpustate->y.d
#define D   	cpustate->d.w.l
#define A   	cpustate->d.b.h
#define B		cpustate->d.b.l
#define DP		cpustate->dp.b.h
#define DPD 	cpustate->dp.d
#define CC  	cpustate->cc

#define EAB		cpustate->ea.b.l
#define EA		cpustate->ea.w.l
#define EAD 	cpustate->ea.d

#define KONAMI_CWAI		8	/* set when CWAI is waiting for an interrupt */
#define KONAMI_SYNC		16	/* set when SYNC is waiting for an interrupt */
#define KONAMI_LDS		32	/* set when LDS occurred at least once */

#define RM(cs,Addr)				(cs)->program->read_byte(Addr)
#define WM(cs,Addr,Value)		(cs)->program->write_byte(Addr,Value)
#define ROP(cs,Addr)			(cs)->direct->read_decrypted_byte(Addr)
#define ROP_ARG(cs,Addr)		(cs)->direct->read_raw_byte(Addr)

#define SIGNED(a)	(UINT16)(INT16)(INT8)(a)

/* macros to access memory */
#define IMMBYTE(cs,b)	{ b = ROP_ARG(cs,PCD); PC++; }
#define IMMWORD(cs,w)	{ w.d = (ROP_ARG(cs,PCD)<<8) | ROP_ARG(cs,PCD+1); PC += 2; }

#define PUSHBYTE(cs,b) --S; WM(cs,SD,b)
#define PUSHWORD(cs,w) --S; WM(cs,SD,w.b.l); --S; WM(cs,SD,w.b.h)
#define PULLBYTE(cs,b) b=RM(cs,SD); S++
#define PULLWORD(cs,w) w=RM(cs,SD)<<8; S++; w|=RM(cs,SD); S++

#define PSHUBYTE(cs,b) --U; WM(cs,UD,b);
#define PSHUWORD(cs,w) --U; WM(cs,UD,w.b.l); --U; WM(cs,UD,w.b.h)
#define PULUBYTE(cs,b) b=RM(cs,UD); U++
#define PULUWORD(cs,w) w=RM(cs,UD)<<8; U++; w|=RM(cs,UD); U++

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

static const UINT8 flags8i[256]=	 /* increment */
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
static const UINT8 flags8d[256]= /* decrement */
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
#define DIRECT(cs)	EAD = DPD; IMMBYTE(cs,EAB)
#define IMM8(cs)	EAD = PCD; PC++
#define IMM16(cs)	EAD = PCD; PC+=2
#define EXTENDED(cs) IMMWORD(cs,(cs)->ea)

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
#define DIRBYTE(cs,b) DIRECT(cs); b=RM(cs,EAD)
#define DIRWORD(cs,w) DIRECT(cs); w.d=RM16(cs,EAD)
#define EXTBYTE(cs,b) EXTENDED(cs); b=RM(cs,EAD)
#define EXTWORD(cs,w) EXTENDED(cs); w.d=RM16(cs,EAD)

/* macros for branch instructions */
#define BRANCH(cs,f) {					\
	UINT8 t;							\
	IMMBYTE(cs,t);						\
	if( f ) 							\
	{									\
		PC += SIGNED(t);				\
	}									\
}

#define LBRANCH(cs,f) {                 \
	PAIR t; 							\
	IMMWORD(cs,t);						\
	if( f ) 							\
	{									\
		cpustate->icount -= 1;			\
		PC += t.w.l;					\
	}									\
}

#define NXORV  ((CC&CC_N)^((CC&CC_V)<<2))

/* macros for setting/getting registers in TFR/EXG instructions */
#define GETREG(val,reg) 				\
	switch(reg) {						\
	case 0: val = A;	break;			\
	case 1: val = B;	break;			\
	case 2: val = X;	break;			\
	case 3: val = Y;	break;			\
	case 4: val = S;	break; /* ? */	\
	case 5: val = U;	break;			\
	default: val = 0xff; logerror("Unknown TFR/EXG idx at PC:%04x\n", PC ); break; \
	}

#define SETREG(val,reg) 				\
	switch(reg) {						\
	case 0: A = val;	break;			\
	case 1: B = val;	break;			\
	case 2: X = val;	break;			\
	case 3: Y = val;	break;			\
	case 4: S = val;	break; /* ? */	\
	case 5: U = val;	break;			\
	default: logerror("Unknown TFR/EXG idx at PC:%04x\n", PC ); break; \
	}

/* opcode timings */
static const UINT8 cycles1[] =
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

INLINE UINT32 RM16( konami_state *cpustate, UINT32 Addr )
{
	UINT32 result = RM(cpustate, Addr) << 8;
	return result | RM(cpustate, (Addr+1)&0xffff);
}

INLINE void WM16( konami_state *cpustate, UINT32 Addr, PAIR *p )
{
	WM(cpustate,  Addr, p->b.h );
	WM(cpustate,  (Addr+1)&0xffff, p->b.l );
}


static void check_irq_lines(konami_state *cpustate)
{
	if (cpustate->nmi_pending && (cpustate->int_state & KONAMI_LDS))
	{
		cpustate->nmi_pending = FALSE;

		/* state already saved by CWAI? */
		if (cpustate->int_state & KONAMI_CWAI)
		{
			cpustate->int_state &= ~KONAMI_CWAI;
			cpustate->icount -= 7;
	    }
		else
		{
			CC |= CC_E; 				/* save entire state */
			PUSHWORD(cpustate, pPC);
			PUSHWORD(cpustate, pU);
			PUSHWORD(cpustate, pY);
			PUSHWORD(cpustate, pX);
			PUSHBYTE(cpustate, DP);
			PUSHBYTE(cpustate, B);
			PUSHBYTE(cpustate, A);
			PUSHBYTE(cpustate, CC);
			cpustate->icount -= 19;
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD = RM16(cpustate, 0xfffc);
		(void)(*cpustate->irq_callback)(cpustate->device, INPUT_LINE_NMI);
	}

	else if (cpustate->irq_state[KONAMI_FIRQ_LINE] !=CLEAR_LINE && !(CC & CC_IF))
	{
		/* fast IRQ */
		/* state already saved by CWAI? */
		if (cpustate->int_state & KONAMI_CWAI)
		{
			cpustate->int_state &= ~KONAMI_CWAI;  /* clear CWAI */
			cpustate->icount -= 7;
        }
		else
		{
			CC &= ~CC_E;				/* save 'short' state */
			PUSHWORD(cpustate, pPC);
			PUSHBYTE(cpustate, CC);
			cpustate->icount -= 10;
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD = RM16(cpustate, 0xfff6);
		(void)(*cpustate->irq_callback)(cpustate->device, KONAMI_FIRQ_LINE);
	}

	else if (cpustate->irq_state[KONAMI_IRQ_LINE] != CLEAR_LINE && !(CC & CC_II))
	{
		/* standard IRQ */
		/* state already saved by CWAI? */
		if (cpustate->int_state & KONAMI_CWAI)
		{
			cpustate->int_state &= ~KONAMI_CWAI;  /* clear CWAI flag */
			cpustate->icount -= 7;
		}
		else
		{
			CC |= CC_E; 				/* save entire state */
			PUSHWORD(cpustate, pPC);
			PUSHWORD(cpustate, pU);
			PUSHWORD(cpustate, pY);
			PUSHWORD(cpustate, pX);
			PUSHBYTE(cpustate, DP);
			PUSHBYTE(cpustate, B);
			PUSHBYTE(cpustate, A);
			PUSHBYTE(cpustate, CC);
			cpustate->icount -= 19;
		}
		CC |= CC_II;					/* inhibit IRQ */
		PCD = RM16(cpustate, 0xfff8);
		(void)(*cpustate->irq_callback)(cpustate->device, KONAMI_IRQ_LINE);
	}
}


/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
static CPU_INIT( konami )
{
	konami_state *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	device->save_item(NAME(PC));
	device->save_item(NAME(U));
	device->save_item(NAME(S));
	device->save_item(NAME(X));
	device->save_item(NAME(Y));
	device->save_item(NAME(D));
	device->save_item(NAME(DP));
	device->save_item(NAME(CC));
	device->save_item(NAME(cpustate->int_state));
	device->save_item(NAME(cpustate->nmi_state));
	device->save_item(NAME(cpustate->nmi_pending));
	device->save_item(NAME(cpustate->irq_state[0]));
	device->save_item(NAME(cpustate->irq_state[1]));
}

static CPU_RESET( konami )
{
	konami_state *cpustate = get_safe_token(device);

	cpustate->int_state = 0;
	cpustate->nmi_state = CLEAR_LINE;
	cpustate->nmi_pending = FALSE;
	cpustate->irq_state[0] = CLEAR_LINE;
	cpustate->irq_state[1] = CLEAR_LINE;

	DPD = 0;			/* Reset direct page register */

    CC |= CC_II;        /* IRQ disabled */
    CC |= CC_IF;        /* FIRQ disabled */

	PCD = RM16(cpustate, 0xfffe);
}

static CPU_EXIT( konami )
{
}

/* Generate interrupts */
/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(konami_state *cpustate, int irqline, int state)
{
	if (state != CLEAR_LINE)
		cpustate->int_state &= ~KONAMI_SYNC;

	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			cpustate->nmi_pending = TRUE;
		cpustate->nmi_state = state;
	}
	else if (irqline < ARRAY_LENGTH(cpustate->irq_state))
		cpustate->irq_state[irqline] = state;
}

/* includes the static function prototypes and the master opcode table */
#include "konamtbl.c"

/* includes the actual opcode implementations */
#include "konamops.c"

/* execute instructions on this CPU until icount expires */
static CPU_EXECUTE( konami )
{
	konami_state *cpustate = get_safe_token(device);

	check_irq_lines(cpustate);

	if( cpustate->int_state & (KONAMI_CWAI | KONAMI_SYNC) )
	{
		cpustate->icount = 0;
	}
	else
	{
		do
		{
			UINT8 ireg;

			pPPC = pPC;

			debugger_instruction_hook(device, PCD);

			cpustate->ireg = ireg = ROP(cpustate, PCD);
			PC++;

            (*konami_main[ireg])(cpustate);

            cpustate->icount -= cycles1[ireg];

        } while( cpustate->icount > 0 );
    }
}


void konami_configure_set_lines(device_t *device, konami_set_lines_func func)
{
	konami_state *cpustate = get_safe_token(device);
	cpustate->setlines_callback = func;
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( konami )
{
	konami_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + KONAMI_IRQ_LINE:	set_irq_line(cpustate, KONAMI_IRQ_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + KONAMI_FIRQ_LINE:set_irq_line(cpustate, KONAMI_FIRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + KONAMI_PC:			PC = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + KONAMI_S:			S = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_CC:			CC = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_U:			U = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_B:			B = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_X:			X = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_Y:			Y = info->i;							break;
		case CPUINFO_INT_REGISTER + KONAMI_DP:			DP = info->i;							break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( konami )
{
	konami_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(konami_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 13;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + KONAMI_IRQ_LINE:	info->i = cpustate->irq_state[KONAMI_IRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + KONAMI_FIRQ_LINE:info->i = cpustate->irq_state[KONAMI_FIRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;				break;

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
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(konami);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(konami);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(konami);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(konami);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(konami);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(konami);break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "KONAMI");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "KONAMI 5000x");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->cc & 0x80 ? 'E':'.',
				cpustate->cc & 0x40 ? 'F':'.',
                cpustate->cc & 0x20 ? 'H':'.',
                cpustate->cc & 0x10 ? 'I':'.',
                cpustate->cc & 0x08 ? 'N':'.',
                cpustate->cc & 0x04 ? 'Z':'.',
                cpustate->cc & 0x02 ? 'V':'.',
                cpustate->cc & 0x01 ? 'C':'.');
            break;

		case CPUINFO_STR_REGISTER + KONAMI_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_S:			sprintf(info->s, "S:%04X", cpustate->s.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_CC:			sprintf(info->s, "CC:%02X", cpustate->cc); break;
		case CPUINFO_STR_REGISTER + KONAMI_U:			sprintf(info->s, "U:%04X", cpustate->u.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_A:			sprintf(info->s, "A:%02X", cpustate->d.b.h); break;
		case CPUINFO_STR_REGISTER + KONAMI_B:			sprintf(info->s, "B:%02X", cpustate->d.b.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_X:			sprintf(info->s, "X:%04X", cpustate->x.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_Y:			sprintf(info->s, "Y:%04X", cpustate->y.w.l); break;
		case CPUINFO_STR_REGISTER + KONAMI_DP:			sprintf(info->s, "DP:%02X", cpustate->dp.b.h); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(KONAMI, konami);
