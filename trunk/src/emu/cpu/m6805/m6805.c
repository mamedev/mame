/*** m6805: Portable 6805 emulator ******************************************

    m6805.c (Also supports hd68705 and hd63705 variants)

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

  Additional Notes:

  K.Wilkins 18/03/99 - Added 63705 functonality and modified all CPU functions
                       necessary to support:
                           Variable width address bus
                           Different stack pointer
                           Alternate boot vectors
                           Alternate interrups vectors


*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "m6805.h"

#define IRQ_LEVEL_DETECT 0

enum
{
	SUBTYPE_M6805,
	SUBTYPE_M68705,
	SUBTYPE_HD63705,
	SUBTYPE_M68HC05EG
};

/* 6805 Registers */
typedef struct
{
	/* Pre-pointerafied public globals */
	int iCount;
	PAIR ea;				/* effective address */

	int 	subtype;		/* Which sub-type is being emulated */
	UINT32	sp_mask;		/* Stack pointer address mask */
	UINT32	sp_low; 		/* Stack pointer low water mark (or floor) */
    PAIR    pc;             /* Program counter */
	PAIR	s;				/* Stack pointer */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* Index register */
	UINT8	cc; 			/* Condition codes */

	UINT16	pending_interrupts; /* MB */
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int 	irq_state[9];		/* KW Additional lines for HD63705 */
	int		nmi_state;
} m6805_Regs;

INLINE m6805_Regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == M6805 ||
		   device->type() == M68705 ||
		   device->type() == HD63705 ||
		   device->type() == M68HC05EG);
	return (m6805_Regs *)downcast<legacy_cpu_device *>(device)->token();
}

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define M6805_RDMEM(Addr) ((unsigned)cpustate->program->read_byte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6805_WRMEM(Addr,Value) (cpustate->program->write_byte(Addr,Value))

/****************************************************************************/
/* M6805_RDOP() is identical to M6805_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M6805_RDOP(Addr) ((unsigned)cpustate->direct->read_decrypted_byte(Addr))

/****************************************************************************/
/* M6805_RDOP_ARG() is identical to M6805_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M6805_RDOP_ARG(Addr) ((unsigned)cpustate->direct->read_raw_byte(Addr))

#define SUBTYPE	cpustate->subtype	/* CPU Type */
#define SP_MASK cpustate->sp_mask	/* stack pointer mask */
#define SP_LOW	cpustate->sp_low	/* stack pointer low water mark */
#define pPC 	cpustate->pc		/* program counter PAIR */
#define PC		cpustate->pc.w.l	/* program counter lower word */
#define S		cpustate->s.w.l 	/* stack pointer lower word */
#define A		cpustate->a 		/* accumulator */
#define X		cpustate->x 		/* index register */
#define CC		cpustate->cc		/* condition codes */

#define EAD cpustate->ea.d
#define EA  cpustate->ea.w.l


/* DS -- THESE ARE RE-DEFINED IN m6805.h TO RAM, ROM or FUNCTIONS IN cpuintrf.c */
#define RM(Addr)			M6805_RDMEM(Addr)
#define WM(Addr,Value)		M6805_WRMEM(Addr,Value)
#define M_RDOP(Addr)        M6805_RDOP(Addr)
#define M_RDOP_ARG(Addr)	M6805_RDOP_ARG(Addr)

/* macros to tweak the PC and SP */
#define SP_INC	if( ++S > SP_MASK) S = SP_LOW
#define SP_DEC	if( --S < SP_LOW) S = SP_MASK
#define SP_ADJUST(s) ( ( (s) & SP_MASK ) | SP_LOW )

/* macros to access memory */
#define IMMBYTE(b) {b = M_RDOP_ARG(PC++);}
#define IMMWORD(w) {w.d = 0; w.b.h = M_RDOP_ARG(PC); w.b.l = M_RDOP_ARG(PC+1); PC+=2;}

#define PUSHBYTE(b) wr_s_handler_b(cpustate, &b)
#define PUSHWORD(w) wr_s_handler_w(cpustate, &w)
#define PULLBYTE(b) rd_s_handler_b(cpustate, &b)
#define PULLWORD(w) rd_s_handler_w(cpustate, &w)

/* CC masks      H INZC
              7654 3210 */
#define CFLAG 0x01
#define ZFLAG 0x02
#define NFLAG 0x04
#define IFLAG 0x08
#define HFLAG 0x10

#define CLR_NZ	  CC&=~(NFLAG|ZFLAG)
#define CLR_HNZC  CC&=~(HFLAG|NFLAG|ZFLAG|CFLAG)
#define CLR_Z	  CC&=~(ZFLAG)
#define CLR_NZC   CC&=~(NFLAG|ZFLAG|CFLAG)
#define CLR_ZC	  CC&=~(ZFLAG|CFLAG)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)       if(!a)SEZ
#define SET_Z8(a)	   SET_Z((UINT8)a)
#define SET_N8(a)	   CC|=((a&0x80)>>5)
#define SET_H(a,b,r)   CC|=((a^b^r)&0x10)
#define SET_C8(a)	   CC|=((a&0x100)>>8)

static const UINT8 flags8i[256]=	 /* increment */
{
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
};
static const UINT8 flags8d[256]= /* decrement */
{
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
};
#define SET_FLAGS8I(a)		{CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_C8(r);}

/* for treating an unsigned UINT8 as a signed INT16 */
#define SIGNED(b) ((INT16)(b&0x80?b|0xff00:b))

/* Macros for addressing modes */
#define DIRECT EAD=0;IMMBYTE(cpustate->ea.b.l)
#define IMM8 EA=PC++
#define EXTENDED IMMWORD(cpustate->ea)
#define INDEXED EA=X
#define INDEXED1 {EAD=0; IMMBYTE(cpustate->ea.b.l); EA+=X;}
#define INDEXED2 {IMMWORD(cpustate->ea); EA+=X;}

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=CFLAG
#define CLC CC&=~CFLAG
#define SEZ CC|=ZFLAG
#define CLZ CC&=~ZFLAG
#define SEN CC|=NFLAG
#define CLN CC&=~NFLAG
#define SEH CC|=HFLAG
#define CLH CC&=~HFLAG
#define SEI CC|=IFLAG
#define CLI CC&=~IFLAG

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDX1BYTE(b) {INDEXED1;b=RM(EAD);}
#define IDX2BYTE(b) {INDEXED2;b=RM(EAD);}
/* Macros for branch instructions */
#define BRANCH(f) { UINT8 t; IMMBYTE(t); if(f) { PC+=SIGNED(t); if (t==0xfe) { /* speed up busy loops */ if(cpustate->iCount > 0) cpustate->iCount = 0; } } }

/* what they say it is ... */
static const unsigned char cycles1[] =
{
      /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
  /*0*/ 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
  /*1*/  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  /*2*/  4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  /*3*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0,
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


/* pre-clear a PAIR union; clearing h2 and h3 only might be faster? */
#define CLEAR_PAIR(p)   p->d = 0

INLINE void rd_s_handler_b( m6805_Regs *cpustate, UINT8 *b )
{
	SP_INC;
	*b = RM( S );
}

INLINE void rd_s_handler_w( m6805_Regs *cpustate, PAIR *p )
{
	CLEAR_PAIR(p);
	SP_INC;
	p->b.h = RM( S );
	SP_INC;
	p->b.l = RM( S );
}

INLINE void wr_s_handler_b( m6805_Regs *cpustate, UINT8 *b )
{
	WM( S, *b );
	SP_DEC;
}

INLINE void wr_s_handler_w( m6805_Regs *cpustate, PAIR *p )
{
	WM( S, p->b.l );
    SP_DEC;
	WM( S, p->b.h );
    SP_DEC;
}

INLINE void RM16( m6805_Regs *cpustate, UINT32 Addr, PAIR *p )
{
	CLEAR_PAIR(p);
    p->b.h = RM(Addr);
    ++Addr;
//  if( ++Addr > AMASK ) Addr = 0;
	p->b.l = RM(Addr);
}

#ifdef UNUSED_FUNCTION
INLINE void WM16( m6805_Regs *cpustate, UINT32 Addr, PAIR *p )
{
	WM( Addr, p->b.h );
    ++Addr;
//  if( ++Addr > AMASK ) Addr = 0;
	WM( Addr, p->b.l );
}
#endif


/* Generate interrupt - m68705 version */
static void m68705_Interrupt( m6805_Regs *cpustate )
{
	if( (cpustate->pending_interrupts & ((1<<M6805_IRQ_LINE)|M68705_INT_MASK)) != 0 )
	{
		if ( (CC & IFLAG) == 0 )
		{
			PUSHWORD(cpustate->pc);
			PUSHBYTE(cpustate->x);
			PUSHBYTE(cpustate->a);
			PUSHBYTE(cpustate->cc);
			SEI;
			if (cpustate->irq_callback)
				(*cpustate->irq_callback)(cpustate->device, 0);

			if ((cpustate->pending_interrupts & (1<<M68705_IRQ_LINE)) != 0 )
			{
				cpustate->pending_interrupts &= ~(1<<M68705_IRQ_LINE);
				RM16( cpustate, 0xfffa, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<M68705_INT_TIMER))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<M68705_INT_TIMER);
				RM16( cpustate, 0xfff8, &pPC);
			}
		}
		cpustate->iCount -= 11;
	}
}

/* Generate interrupts */
static void Interrupt( m6805_Regs *cpustate )
{
	/* the 6805 latches interrupt requests internally, so we don't clear */
	/* pending_interrupts until the interrupt is taken, no matter what the */
	/* external IRQ pin does. */

	if( (cpustate->pending_interrupts & (1<<HD63705_INT_NMI)) != 0)
	{
		PUSHWORD(cpustate->pc);
		PUSHBYTE(cpustate->x);
		PUSHBYTE(cpustate->a);
		PUSHBYTE(cpustate->cc);
		SEI;
		/* no vectors supported, just do the callback to clear irq_state if needed */
		if (cpustate->irq_callback)
			(*cpustate->irq_callback)(cpustate->device, 0);

		RM16( cpustate, 0x1ffc, &pPC);
		cpustate->pending_interrupts &= ~(1<<HD63705_INT_NMI);

		cpustate->iCount -= 11;

	}
	else if( (cpustate->pending_interrupts & ((1<<M6805_IRQ_LINE)|HD63705_INT_MASK)) != 0 ) {
		if ( (CC & IFLAG) == 0 ) {
	{
        /* standard IRQ */
//#if (HAS_HD63705)
//      if(SUBTYPE!=SUBTYPE_HD63705)
//#endif
//          PC |= ~AMASK;
		PUSHWORD(cpustate->pc);
		PUSHBYTE(cpustate->x);
		PUSHBYTE(cpustate->a);
		PUSHBYTE(cpustate->cc);
		SEI;
		/* no vectors supported, just do the callback to clear irq_state if needed */
		if (cpustate->irq_callback)
			(*cpustate->irq_callback)(cpustate->device, 0);


		if(SUBTYPE==SUBTYPE_HD63705)
		{
			/* Need to add emulation of other interrupt sources here KW-2/4/99 */
			/* This is just a quick patch for Namco System 2 operation         */

			if((cpustate->pending_interrupts&(1<<HD63705_INT_IRQ1))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_IRQ1);
				RM16( cpustate, 0x1ff8, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_IRQ2))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_IRQ2);
				RM16( cpustate, 0x1fec, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_ADCONV))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_ADCONV);
				RM16( cpustate, 0x1fea, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_TIMER1))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_TIMER1);
				RM16( cpustate, 0x1ff6, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_TIMER2))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_TIMER2);
				RM16( cpustate, 0x1ff4, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_TIMER3))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_TIMER3);
				RM16( cpustate, 0x1ff2, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_PCI))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_PCI);
				RM16( cpustate, 0x1ff0, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<HD63705_INT_SCI))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<HD63705_INT_SCI);
				RM16( cpustate, 0x1fee, &pPC);
			}
		}
		else if (SUBTYPE == SUBTYPE_M68HC05EG)
		{
			if((cpustate->pending_interrupts&(1<<M68HC05EG_INT_IRQ))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<M68HC05EG_INT_IRQ);
				RM16( cpustate, 0x1ffa, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<M68HC05EG_INT_TIMER))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<M68HC05EG_INT_TIMER);
				RM16( cpustate, 0x1ff8, &pPC);
			}
			else if((cpustate->pending_interrupts&(1<<M68HC05EG_INT_CPI))!=0)
			{
				cpustate->pending_interrupts &= ~(1<<M68HC05EG_INT_CPI);
				RM16( cpustate, 0x1ff6, &pPC);
			}
		}
		else
		{
			RM16( cpustate, 0xffff - 5, &pPC );
		}

	}	// CC & IFLAG
			cpustate->pending_interrupts &= ~(1<<M6805_IRQ_LINE);
		}
		cpustate->iCount -= 11;
	}
}

static void state_register(m6805_Regs *cpustate, const char *type, legacy_cpu_device *device)
{
	device->save_item(NAME(A));
	device->save_item(NAME(PC));
	device->save_item(NAME(S));
	device->save_item(NAME(X));
	device->save_item(NAME(CC));
	device->save_item(NAME(cpustate->pending_interrupts));
	device->save_item(NAME(cpustate->irq_state));
}

static CPU_INIT( m6805 )
{
	m6805_Regs *cpustate = get_safe_token(device);

	state_register(cpustate, "m6805", device);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
}

static CPU_RESET( m6805 )
{
	m6805_Regs *cpustate = get_safe_token(device);

	device_irq_acknowledge_callback save_irqcallback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(*cpustate));

	cpustate->iCount=50000;		/* Used to be global */
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	/* Force CPU sub-type and relevant masks */
	cpustate->subtype = SUBTYPE_M6805;
	SP_MASK = 0x07f;
	SP_LOW	= 0x060;

	/* Initial stack pointer */
	S = SP_MASK;

	/* IRQ disabled */
    SEI;
	RM16( cpustate, 0xfffe, &pPC );
}

static CPU_EXIT( m6805 )
{
	/* nothing to do */
}


static void set_irq_line( m6805_Regs *cpustate,	int irqline, int state )
{
	/* Basic 6805 only has one IRQ line */
	/* See HD63705 specific version     */
	if (cpustate->irq_state[0] == state) return;

	cpustate->irq_state[0] = state;
	if (state != CLEAR_LINE)
		cpustate->pending_interrupts |= 1<<M6805_IRQ_LINE;
}

#include "6805ops.c"


/* execute instructions on this CPU until icount expires */
static CPU_EXECUTE( m6805 )
{
	UINT8 ireg;
	m6805_Regs *cpustate = get_safe_token(device);

	S = SP_ADJUST( S );		/* Taken from CPU_SET_CONTEXT when pointer'afying */

	do
	{
		if (cpustate->pending_interrupts != 0)
		{
			if (SUBTYPE==SUBTYPE_M68705)
			{
				m68705_Interrupt(cpustate);
			}
			else
			{
				Interrupt(cpustate);
			}
		}

		debugger_instruction_hook(device, PC);

		ireg=M_RDOP(PC++);

		switch( ireg )
		{
			case 0x00: brset(cpustate, 0x01); break;
			case 0x01: brclr(cpustate, 0x01); break;
			case 0x02: brset(cpustate, 0x02); break;
			case 0x03: brclr(cpustate, 0x02); break;
			case 0x04: brset(cpustate, 0x04); break;
			case 0x05: brclr(cpustate, 0x04); break;
			case 0x06: brset(cpustate, 0x08); break;
			case 0x07: brclr(cpustate, 0x08); break;
			case 0x08: brset(cpustate, 0x10); break;
			case 0x09: brclr(cpustate, 0x10); break;
			case 0x0A: brset(cpustate, 0x20); break;
			case 0x0B: brclr(cpustate, 0x20); break;
			case 0x0C: brset(cpustate, 0x40); break;
			case 0x0D: brclr(cpustate, 0x40); break;
			case 0x0E: brset(cpustate, 0x80); break;
			case 0x0F: brclr(cpustate, 0x80); break;
			case 0x10: bset(cpustate, 0x01); break;
			case 0x11: bclr(cpustate, 0x01); break;
			case 0x12: bset(cpustate, 0x02); break;
			case 0x13: bclr(cpustate, 0x02); break;
			case 0x14: bset(cpustate, 0x04); break;
			case 0x15: bclr(cpustate, 0x04); break;
			case 0x16: bset(cpustate, 0x08); break;
			case 0x17: bclr(cpustate, 0x08); break;
			case 0x18: bset(cpustate, 0x10); break;
			case 0x19: bclr(cpustate, 0x10); break;
			case 0x1a: bset(cpustate, 0x20); break;
			case 0x1b: bclr(cpustate, 0x20); break;
			case 0x1c: bset(cpustate, 0x40); break;
			case 0x1d: bclr(cpustate, 0x40); break;
			case 0x1e: bset(cpustate, 0x80); break;
			case 0x1f: bclr(cpustate, 0x80); break;
			case 0x20: bra(cpustate); break;
			case 0x21: brn(cpustate); break;
			case 0x22: bhi(cpustate); break;
			case 0x23: bls(cpustate); break;
			case 0x24: bcc(cpustate); break;
			case 0x25: bcs(cpustate); break;
			case 0x26: bne(cpustate); break;
			case 0x27: beq(cpustate); break;
			case 0x28: bhcc(cpustate); break;
			case 0x29: bhcs(cpustate); break;
			case 0x2a: bpl(cpustate); break;
			case 0x2b: bmi(cpustate); break;
			case 0x2c: bmc(cpustate); break;
			case 0x2d: bms(cpustate); break;
			case 0x2e: bil(cpustate); break;
			case 0x2f: bih(cpustate); break;
			case 0x30: neg_di(cpustate); break;
			case 0x31: illegal(cpustate); break;
			case 0x32: illegal(cpustate); break;
			case 0x33: com_di(cpustate); break;
			case 0x34: lsr_di(cpustate); break;
			case 0x35: illegal(cpustate); break;
			case 0x36: ror_di(cpustate); break;
			case 0x37: asr_di(cpustate); break;
			case 0x38: lsl_di(cpustate); break;
			case 0x39: rol_di(cpustate); break;
			case 0x3a: dec_di(cpustate); break;
			case 0x3b: illegal(cpustate); break;
			case 0x3c: inc_di(cpustate); break;
			case 0x3d: tst_di(cpustate); break;
			case 0x3e: illegal(cpustate); break;
			case 0x3f: clr_di(cpustate); break;
			case 0x40: nega(cpustate); break;
			case 0x41: illegal(cpustate); break;
			case 0x42: illegal(cpustate); break;
			case 0x43: coma(cpustate); break;
			case 0x44: lsra(cpustate); break;
			case 0x45: illegal(cpustate); break;
			case 0x46: rora(cpustate); break;
			case 0x47: asra(cpustate); break;
			case 0x48: lsla(cpustate); break;
			case 0x49: rola(cpustate); break;
			case 0x4a: deca(cpustate); break;
			case 0x4b: illegal(cpustate); break;
			case 0x4c: inca(cpustate); break;
			case 0x4d: tsta(cpustate); break;
			case 0x4e: illegal(cpustate); break;
			case 0x4f: clra(cpustate); break;
			case 0x50: negx(cpustate); break;
			case 0x51: illegal(cpustate); break;
			case 0x52: illegal(cpustate); break;
			case 0x53: comx(cpustate); break;
			case 0x54: lsrx(cpustate); break;
			case 0x55: illegal(cpustate); break;
			case 0x56: rorx(cpustate); break;
			case 0x57: asrx(cpustate); break;
			case 0x58: aslx(cpustate); break;
			case 0x59: rolx(cpustate); break;
			case 0x5a: decx(cpustate); break;
			case 0x5b: illegal(cpustate); break;
			case 0x5c: incx(cpustate); break;
			case 0x5d: tstx(cpustate); break;
			case 0x5e: illegal(cpustate); break;
			case 0x5f: clrx(cpustate); break;
			case 0x60: neg_ix1(cpustate); break;
			case 0x61: illegal(cpustate); break;
			case 0x62: illegal(cpustate); break;
			case 0x63: com_ix1(cpustate); break;
			case 0x64: lsr_ix1(cpustate); break;
			case 0x65: illegal(cpustate); break;
			case 0x66: ror_ix1(cpustate); break;
			case 0x67: asr_ix1(cpustate); break;
			case 0x68: lsl_ix1(cpustate); break;
			case 0x69: rol_ix1(cpustate); break;
			case 0x6a: dec_ix1(cpustate); break;
			case 0x6b: illegal(cpustate); break;
			case 0x6c: inc_ix1(cpustate); break;
			case 0x6d: tst_ix1(cpustate); break;
			case 0x6e: illegal(cpustate); break;
			case 0x6f: clr_ix1(cpustate); break;
			case 0x70: neg_ix(cpustate); break;
			case 0x71: illegal(cpustate); break;
			case 0x72: illegal(cpustate); break;
			case 0x73: com_ix(cpustate); break;
			case 0x74: lsr_ix(cpustate); break;
			case 0x75: illegal(cpustate); break;
			case 0x76: ror_ix(cpustate); break;
			case 0x77: asr_ix(cpustate); break;
			case 0x78: lsl_ix(cpustate); break;
			case 0x79: rol_ix(cpustate); break;
			case 0x7a: dec_ix(cpustate); break;
			case 0x7b: illegal(cpustate); break;
			case 0x7c: inc_ix(cpustate); break;
			case 0x7d: tst_ix(cpustate); break;
			case 0x7e: illegal(cpustate); break;
			case 0x7f: clr_ix(cpustate); break;
			case 0x80: rti(cpustate); break;
			case 0x81: rts(cpustate); break;
			case 0x82: illegal(cpustate); break;
			case 0x83: swi(cpustate); break;
			case 0x84: illegal(cpustate); break;
			case 0x85: illegal(cpustate); break;
			case 0x86: illegal(cpustate); break;
			case 0x87: illegal(cpustate); break;
			case 0x88: illegal(cpustate); break;
			case 0x89: illegal(cpustate); break;
			case 0x8a: illegal(cpustate); break;
			case 0x8b: illegal(cpustate); break;
			case 0x8c: illegal(cpustate); break;
			case 0x8d: illegal(cpustate); break;
			case 0x8e: illegal(cpustate); break;
			case 0x8f: illegal(cpustate); break;
			case 0x90: illegal(cpustate); break;
			case 0x91: illegal(cpustate); break;
			case 0x92: illegal(cpustate); break;
			case 0x93: illegal(cpustate); break;
			case 0x94: illegal(cpustate); break;
			case 0x95: illegal(cpustate); break;
			case 0x96: illegal(cpustate); break;
			case 0x97: tax(cpustate); break;
			case 0x98: CLC; break;
			case 0x99: SEC; break;
#if IRQ_LEVEL_DETECT
			case 0x9a: CLI; if (m6805.irq_state != CLEAR_LINE) m6805.pending_interrupts |= 1<<M6805_IRQ_LINE; break;
#else
			case 0x9a: CLI; break;
#endif
			case 0x9b: SEI; break;
			case 0x9c: rsp(cpustate); break;
			case 0x9d: nop(cpustate); break;
			case 0x9e: illegal(cpustate); break;
			case 0x9f: txa(cpustate); break;
			case 0xa0: suba_im(cpustate); break;
			case 0xa1: cmpa_im(cpustate); break;
			case 0xa2: sbca_im(cpustate); break;
			case 0xa3: cpx_im(cpustate); break;
			case 0xa4: anda_im(cpustate); break;
			case 0xa5: bita_im(cpustate); break;
			case 0xa6: lda_im(cpustate); break;
			case 0xa7: illegal(cpustate); break;
			case 0xa8: eora_im(cpustate); break;
			case 0xa9: adca_im(cpustate); break;
			case 0xaa: ora_im(cpustate); break;
			case 0xab: adda_im(cpustate); break;
			case 0xac: illegal(cpustate); break;
			case 0xad: bsr(cpustate); break;
			case 0xae: ldx_im(cpustate); break;
			case 0xaf: illegal(cpustate); break;
			case 0xb0: suba_di(cpustate); break;
			case 0xb1: cmpa_di(cpustate); break;
			case 0xb2: sbca_di(cpustate); break;
			case 0xb3: cpx_di(cpustate); break;
			case 0xb4: anda_di(cpustate); break;
			case 0xb5: bita_di(cpustate); break;
			case 0xb6: lda_di(cpustate); break;
			case 0xb7: sta_di(cpustate); break;
			case 0xb8: eora_di(cpustate); break;
			case 0xb9: adca_di(cpustate); break;
			case 0xba: ora_di(cpustate); break;
			case 0xbb: adda_di(cpustate); break;
			case 0xbc: jmp_di(cpustate); break;
			case 0xbd: jsr_di(cpustate); break;
			case 0xbe: ldx_di(cpustate); break;
			case 0xbf: stx_di(cpustate); break;
			case 0xc0: suba_ex(cpustate); break;
			case 0xc1: cmpa_ex(cpustate); break;
			case 0xc2: sbca_ex(cpustate); break;
			case 0xc3: cpx_ex(cpustate); break;
			case 0xc4: anda_ex(cpustate); break;
			case 0xc5: bita_ex(cpustate); break;
			case 0xc6: lda_ex(cpustate); break;
			case 0xc7: sta_ex(cpustate); break;
			case 0xc8: eora_ex(cpustate); break;
			case 0xc9: adca_ex(cpustate); break;
			case 0xca: ora_ex(cpustate); break;
			case 0xcb: adda_ex(cpustate); break;
			case 0xcc: jmp_ex(cpustate); break;
			case 0xcd: jsr_ex(cpustate); break;
			case 0xce: ldx_ex(cpustate); break;
			case 0xcf: stx_ex(cpustate); break;
			case 0xd0: suba_ix2(cpustate); break;
			case 0xd1: cmpa_ix2(cpustate); break;
			case 0xd2: sbca_ix2(cpustate); break;
			case 0xd3: cpx_ix2(cpustate); break;
			case 0xd4: anda_ix2(cpustate); break;
			case 0xd5: bita_ix2(cpustate); break;
			case 0xd6: lda_ix2(cpustate); break;
			case 0xd7: sta_ix2(cpustate); break;
			case 0xd8: eora_ix2(cpustate); break;
			case 0xd9: adca_ix2(cpustate); break;
			case 0xda: ora_ix2(cpustate); break;
			case 0xdb: adda_ix2(cpustate); break;
			case 0xdc: jmp_ix2(cpustate); break;
			case 0xdd: jsr_ix2(cpustate); break;
			case 0xde: ldx_ix2(cpustate); break;
			case 0xdf: stx_ix2(cpustate); break;
			case 0xe0: suba_ix1(cpustate); break;
			case 0xe1: cmpa_ix1(cpustate); break;
			case 0xe2: sbca_ix1(cpustate); break;
			case 0xe3: cpx_ix1(cpustate); break;
			case 0xe4: anda_ix1(cpustate); break;
			case 0xe5: bita_ix1(cpustate); break;
			case 0xe6: lda_ix1(cpustate); break;
			case 0xe7: sta_ix1(cpustate); break;
			case 0xe8: eora_ix1(cpustate); break;
			case 0xe9: adca_ix1(cpustate); break;
			case 0xea: ora_ix1(cpustate); break;
			case 0xeb: adda_ix1(cpustate); break;
			case 0xec: jmp_ix1(cpustate); break;
			case 0xed: jsr_ix1(cpustate); break;
			case 0xee: ldx_ix1(cpustate); break;
			case 0xef: stx_ix1(cpustate); break;
			case 0xf0: suba_ix(cpustate); break;
			case 0xf1: cmpa_ix(cpustate); break;
			case 0xf2: sbca_ix(cpustate); break;
			case 0xf3: cpx_ix(cpustate); break;
			case 0xf4: anda_ix(cpustate); break;
			case 0xf5: bita_ix(cpustate); break;
			case 0xf6: lda_ix(cpustate); break;
			case 0xf7: sta_ix(cpustate); break;
			case 0xf8: eora_ix(cpustate); break;
			case 0xf9: adca_ix(cpustate); break;
			case 0xfa: ora_ix(cpustate); break;
			case 0xfb: adda_ix(cpustate); break;
			case 0xfc: jmp_ix(cpustate); break;
			case 0xfd: jsr_ix(cpustate); break;
			case 0xfe: ldx_ix(cpustate); break;
			case 0xff: stx_ix(cpustate); break;
		}
		cpustate->iCount -= cycles1[ireg];
	} while( cpustate->iCount > 0 );
}

/****************************************************************************
 * M68HC05EG section
 ****************************************************************************/
static CPU_INIT( m68hc05eg )
{
	m6805_Regs *cpustate = get_safe_token(device);
	state_register(cpustate, "m68hc05eg", device);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
}

static CPU_RESET( m68hc05eg )
{
	m6805_Regs *cpustate = get_safe_token(device);
	CPU_RESET_CALL(m6805);

	/* Overide default 6805 type */
	cpustate->subtype = SUBTYPE_M68HC05EG;
	SP_MASK = 0xff;
	SP_LOW	= 0xc0;
	RM16( cpustate, 0x1ffe, &cpustate->pc );
}

static void m68hc05eg_set_irq_line(m6805_Regs *cpustate, int irqline, int state)
{
	if (cpustate->irq_state[irqline] != state)
	{
		cpustate->irq_state[irqline] = state;

		if (state != CLEAR_LINE)
		{
			cpustate->pending_interrupts |= 1<<irqline;
		}
	}
}


/****************************************************************************
 * M68705 section
 ****************************************************************************/
static CPU_INIT( m68705 )
{
	m6805_Regs *cpustate = get_safe_token(device);
	state_register(cpustate, "m68705", device);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
}

static CPU_RESET( m68705 )
{
	m6805_Regs *cpustate = get_safe_token(device);
	CPU_RESET_CALL(m6805);

	/* Overide default 6805 type */
	cpustate->subtype = SUBTYPE_M68705;
	RM16( cpustate, 0xfffe, &cpustate->pc );
}

static void m68705_set_irq_line(m6805_Regs *cpustate, int irqline, int state)
{
	if (cpustate->irq_state[irqline] == state ) return;
	cpustate->irq_state[irqline] = state;
	if (state != CLEAR_LINE) cpustate->pending_interrupts |= 1<<irqline;
}


/****************************************************************************
 * HD63705 section
 ****************************************************************************/
static CPU_INIT( hd63705 )
{
	m6805_Regs *cpustate = get_safe_token(device);
	state_register(cpustate, "hd63705", device);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
}

static CPU_RESET( hd63705 )
{
	m6805_Regs *cpustate = get_safe_token(device);
	CPU_RESET_CALL(m6805);

	/* Overide default 6805 types */
	cpustate->subtype = SUBTYPE_HD63705;
	SP_MASK = 0x17f;
	SP_LOW	= 0x100;
	RM16( cpustate, 0x1ffe, &cpustate->pc );
	S = 0x17f;
}

static void hd63705_set_irq_line(m6805_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;

		cpustate->nmi_state = state;
		if (state != CLEAR_LINE)
			cpustate->pending_interrupts |= 1<<HD63705_INT_NMI;
	}
	else if (irqline <= HD63705_INT_ADCONV)
	{
		if (cpustate->irq_state[irqline] == state) return;
		cpustate->irq_state[irqline] = state;
		if (state != CLEAR_LINE) cpustate->pending_interrupts |= 1<<irqline;
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m6805 )
{
	m6805_Regs *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6805_IRQ_LINE:	set_irq_line(cpustate, M6805_IRQ_LINE, info->i); break;

		case CPUINFO_INT_REGISTER + M6805_A:			A = info->i;			break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M6805_PC:			PC = info->i;			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M6805_S:			S = SP_ADJUST(info->i);	break;
		case CPUINFO_INT_REGISTER + M6805_X:			X = info->i;			break;
		case CPUINFO_INT_REGISTER + M6805_CC:			CC = info->i;			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m6805 )
{
	m6805_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6805_Regs);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 12;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6805_IRQ_LINE:	info->i = cpustate->irq_state[M6805_IRQ_LINE]; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_REGISTER + M6805_A:			info->i = A;							break;
		case CPUINFO_INT_PC:							info->i = PC;							break;
		case CPUINFO_INT_REGISTER + M6805_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M6805_S:			info->i = SP_ADJUST(S);					break;
		case CPUINFO_INT_REGISTER + M6805_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + M6805_CC:			info->i = CC;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m6805);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6805);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m6805);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m6805);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6805);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6805);break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->iCount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6805");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Motorola 6805");	break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);			break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "The MAME team.");	break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->cc & 0x80 ? '?':'.',
                cpustate->cc & 0x40 ? '?':'.',
                cpustate->cc & 0x20 ? '?':'.',
                cpustate->cc & 0x10 ? 'H':'.',
                cpustate->cc & 0x08 ? 'I':'.',
                cpustate->cc & 0x04 ? 'N':'.',
                cpustate->cc & 0x02 ? 'Z':'.',
                cpustate->cc & 0x01 ? 'C':'.');
            break;

		case CPUINFO_STR_REGISTER + M6805_A:			sprintf(info->s, "A:%02X", cpustate->a);		break;
		case CPUINFO_STR_REGISTER + M6805_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l);	break;
		case CPUINFO_STR_REGISTER + M6805_S:			sprintf(info->s, "S:%02X", cpustate->s.w.l);	break;
		case CPUINFO_STR_REGISTER + M6805_X:			sprintf(info->s, "X:%02X", cpustate->x);		break;
		case CPUINFO_STR_REGISTER + M6805_CC:			sprintf(info->s, "CC:%02X", cpustate->cc);		break;
	}
}

/**************************************************************************
 * CPU-specific set_info for 68HC05EG
 **************************************************************************/
static CPU_SET_INFO( m68hc05eg )
{
	m6805_Regs *cpustate = get_safe_token(device);

	switch(state)
	{
		case CPUINFO_INT_INPUT_STATE + M68HC05EG_INT_IRQ:	m68hc05eg_set_irq_line(cpustate, M68HC05EG_INT_IRQ, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M68HC05EG_INT_TIMER:	m68hc05eg_set_irq_line(cpustate, M68HC05EG_INT_TIMER, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M68HC05EG_INT_CPI:	m68hc05eg_set_irq_line(cpustate, M68HC05EG_INT_CPI, info->i); break;

		default:						CPU_SET_INFO_CALL(m6805); break;
	}
}

CPU_GET_INFO( m68hc05eg )
{
	m6805_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M68HC05EG_INT_IRQ:	info->i = cpustate->irq_state[M68HC05EG_INT_IRQ]; break;
		case CPUINFO_INT_INPUT_STATE + M68HC05EG_INT_TIMER:	info->i = cpustate->irq_state[M68HC05EG_INT_TIMER]; break;
		case CPUINFO_INT_INPUT_STATE + M68HC05EG_INT_CPI:	info->i = cpustate->irq_state[M68HC05EG_INT_CPI]; break;

		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 13;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(m68hc05eg);	break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68hc05eg);			break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(m68hc05eg);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:			strcpy(info->s, "M68HC05EG");	break;

		default:    				CPU_GET_INFO_CALL(m6805);	break;
	}
}

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/
static CPU_SET_INFO( m68705 )
{
	m6805_Regs *cpustate = get_safe_token(device);

	switch(state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M68705_INT_TIMER:	m68705_set_irq_line(cpustate, M68705_INT_TIMER, info->i); break;

		default:											CPU_SET_INFO_CALL(m6805); break;
	}
}

CPU_GET_INFO( m68705 )
{
	m6805_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M68705_INT_TIMER:	info->i = cpustate->irq_state[M68705_INT_TIMER]; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:							info->setinfo = CPU_SET_INFO_NAME(m68705);	break;
		case CPUINFO_FCT_INIT:								info->init = CPU_INIT_NAME(m68705);			break;
		case CPUINFO_FCT_RESET:								info->reset = CPU_RESET_NAME(m68705);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:								strcpy(info->s, "M68705");	break;

		default:											CPU_GET_INFO_CALL(m6805);	break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static CPU_SET_INFO( hd63705 )
{
	m6805_Regs *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_IRQ1:	hd63705_set_irq_line(cpustate, HD63705_INT_IRQ1, info->i);	 break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_IRQ2:	hd63705_set_irq_line(cpustate, HD63705_INT_IRQ2, info->i);	 break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_TIMER1:	hd63705_set_irq_line(cpustate, HD63705_INT_TIMER1, info->i); break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_TIMER2:	hd63705_set_irq_line(cpustate, HD63705_INT_TIMER2, info->i); break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_TIMER3:	hd63705_set_irq_line(cpustate, HD63705_INT_TIMER3, info->i); break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_PCI:		hd63705_set_irq_line(cpustate, HD63705_INT_PCI, info->i);	 break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_SCI:		hd63705_set_irq_line(cpustate, HD63705_INT_SCI, info->i);	 break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_ADCONV:	hd63705_set_irq_line(cpustate, HD63705_INT_ADCONV, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		hd63705_set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	 break;

		default:											CPU_SET_INFO_CALL(m6805);break;
	}
}

CPU_GET_INFO( hd63705 )
{
	m6805_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_IRQ1:	info->i = cpustate->irq_state[HD63705_INT_IRQ1];	break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_IRQ2:	info->i = cpustate->irq_state[HD63705_INT_IRQ2];	break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_TIMER1:	info->i = cpustate->irq_state[HD63705_INT_TIMER1];	break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_TIMER2:	info->i = cpustate->irq_state[HD63705_INT_TIMER2];	break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_TIMER3:	info->i = cpustate->irq_state[HD63705_INT_TIMER3];	break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_PCI:		info->i = cpustate->irq_state[HD63705_INT_PCI];		break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_SCI:		info->i = cpustate->irq_state[HD63705_INT_SCI];		break;
		case CPUINFO_INT_INPUT_STATE + HD63705_INT_ADCONV:	info->i = cpustate->irq_state[HD63705_INT_ADCONV];	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		info->i = cpustate->irq_state[HD63705_INT_NMI];		break;

		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(hd63705);	break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(hd63705);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(hd63705);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "HD63705");	break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");		break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Keith Wilkins, Juergen Buchmueller"); break;

		default:										CPU_GET_INFO_CALL(m6805);	break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(M6805, m6805);
DEFINE_LEGACY_CPU_DEVICE(M68HC05EG, m68hc05eg);
DEFINE_LEGACY_CPU_DEVICE(M68705, m68705);
DEFINE_LEGACY_CPU_DEVICE(HD63705, hd63705);
