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

#include "debugger.h"
#include "m6809.h"

/* Enable big switch statement for the main opcodes */
#ifndef BIG_SWITCH
#define BIG_SWITCH  1
#endif

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* 6809 Registers */
typedef struct _m68_state_t m68_state_t;
struct _m68_state_t
{
	PAIR	pc; 		/* Program counter */
	PAIR	ppc;		/* Previous program counter */
	PAIR	d;		/* Accumulator a and b */
	PAIR	dp; 		/* Direct Page register (page in MSB) */
	PAIR	u, s;		/* Stack pointers */
	PAIR	x, y;		/* Index registers */
	UINT8	cc;
	UINT8	ireg;		/* First opcode */
	UINT8	irq_state[2];

	int 	extra_cycles; /* cycles used up by interrupts */
	cpu_irq_callback irq_callback;
	const device_config *device;
	const m6809_config *config;
	int		icount;
	PAIR	ea;			/* effective address */

	/* Memory spaces */
    const address_space *program;

	UINT8	int_state;	/* SYNC and CWAI flags */
	UINT8	nmi_state;
};

INLINE m68_state_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_M6809 ||
		   cpu_get_type(device) == CPU_M6809E);
	return (m68_state_t *)device->token;
}

static void check_irq_lines( m68_state_t *m68_state );
static void IIError(m68_state_t *m68_state);

INLINE void fetch_effective_address( m68_state_t *m68_state );

/* flag bits in the cc register */
#define CC_C	0x01        /* Carry */
#define CC_V	0x02        /* Overflow */
#define CC_Z    0x04        /* Zero */
#define CC_N    0x08        /* Negative */
#define CC_II   0x10        /* Inhibit IRQ */
#define CC_H    0x20        /* Half (auxiliary) carry */
#define CC_IF   0x40        /* Inhibit FIRQ */
#define CC_E    0x80        /* entire state pushed */

#define pPPC    m68_state->ppc
#define pPC 	m68_state->pc
#define pU		m68_state->u
#define pS		m68_state->s
#define pX		m68_state->x
#define pY		m68_state->y
#define pD		m68_state->d

#define	PPC		m68_state->ppc.w.l
#define PC  	m68_state->pc.w.l
#define PCD 	m68_state->pc.d
#define U		m68_state->u.w.l
#define UD		m68_state->u.d
#define S		m68_state->s.w.l
#define SD		m68_state->s.d
#define X		m68_state->x.w.l
#define XD		m68_state->x.d
#define Y		m68_state->y.w.l
#define YD		m68_state->y.d
#define D   	m68_state->d.w.l
#define A   	m68_state->d.b.h
#define B		m68_state->d.b.l
#define DP		m68_state->dp.b.h
#define DPD 	m68_state->dp.d
#define CC  	m68_state->cc

#define EA	m68_state->ea.w.l
#define EAD m68_state->ea.d
#define EAP m68_state->ea

#define M6809_CWAI		8	/* set when CWAI is waiting for an interrupt */
#define M6809_SYNC		16	/* set when SYNC is waiting for an interrupt */
#define M6809_LDS		32	/* set when LDS occured at least once */


/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(Addr) ((unsigned)memory_read_byte_8be(m68_state->program, Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (memory_write_byte_8be(m68_state->program, Addr,Value))

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define ROP(Addr) ((unsigned)memory_decrypted_read_byte(m68_state->program, Addr))

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define ROP_ARG(Addr) ((unsigned)memory_raw_read_byte(m68_state->program, Addr))

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

#define SET_FLAGS8I(a)		{CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z(a);}
#define SET_NZ16(a)			{SET_N16(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#define NXORV			((CC&CC_N)^((CC&CC_V)<<2))

/* for treating an unsigned byte as a signed word */
#define SIGNED(b) ((UINT16)(b&0x80?b|0xff00:b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT	EAD = DPD; IMMBYTE(m68_state->ea.b.l)
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
#define DIRWORD(w) {DIRECT;w.d=RM16(m68_state, EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(m68_state, EAD);}

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
		m68_state->icount -= 1;				\
		PC += t.w.l;					\
	}									\
}

/* macros for setting/getting registers in TFR/EXG instructions */

INLINE UINT32 RM16(m68_state_t *m68_state, UINT32 Addr )
{
	UINT32 result = RM(Addr) << 8;
	return result | RM((Addr+1)&0xffff);
}

INLINE void WM16( m68_state_t *m68_state, UINT32 Addr, PAIR *p )
{
	WM( Addr, p->b.h );
	WM( (Addr+1)&0xffff, p->b.l );
}

static void UpdateState(m68_state_t *m68_state)
{
	/* compatibility with 6309 */
}

static void check_irq_lines(m68_state_t *m68_state)
{
	if( m68_state->irq_state[M6809_IRQ_LINE] != CLEAR_LINE ||
		m68_state->irq_state[M6809_FIRQ_LINE] != CLEAR_LINE )
		m68_state->int_state &= ~M6809_SYNC; /* clear SYNC flag */
	if( m68_state->irq_state[M6809_FIRQ_LINE]!=CLEAR_LINE && !(CC & CC_IF) )
	{
		/* fast IRQ */
		/* HJB 990225: state already saved by CWAI? */
		if( m68_state->int_state & M6809_CWAI )
		{
			m68_state->int_state &= ~M6809_CWAI;  /* clear CWAI */
			m68_state->extra_cycles += 7;		 /* subtract +7 cycles */
        }
		else
		{
			CC &= ~CC_E;				/* save 'short' state */
			PUSHWORD(pPC);
			PUSHBYTE(CC);
			m68_state->extra_cycles += 10;	/* subtract +10 cycles */
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD=RM16(m68_state, 0xfff6);
		(void)(*m68_state->irq_callback)(m68_state->device, M6809_FIRQ_LINE);
	}
	else
	if( m68_state->irq_state[M6809_IRQ_LINE]!=CLEAR_LINE && !(CC & CC_II) )
	{
		/* standard IRQ */
		/* HJB 990225: state already saved by CWAI? */
		if( m68_state->int_state & M6809_CWAI )
		{
			m68_state->int_state &= ~M6809_CWAI;  /* clear CWAI flag */
			m68_state->extra_cycles += 7;		 /* subtract +7 cycles */
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
			m68_state->extra_cycles += 19;	 /* subtract +19 cycles */
		}
		CC |= CC_II;					/* inhibit IRQ */
		PCD=RM16(m68_state, 0xfff8);
		(void)(*m68_state->irq_callback)(m68_state->device, M6809_IRQ_LINE);
	}
}



/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
static CPU_INIT( m6809 )
{
	/* default configuration */
	static const m6809_config default_config =
	{
		0
	};

	const m6809_config *configdata = device->static_config ? (const m6809_config *)device->static_config : &default_config;
	m68_state_t *m68_state = get_safe_token(device);

	m68_state->config = configdata;
	m68_state->irq_callback = irqcallback;
	m68_state->device = device;

	m68_state->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

	/* setup regtable */

	state_save_register_device_item(device, 0, PC);
	state_save_register_device_item(device, 0, PPC);
	state_save_register_device_item(device, 0, D);
	state_save_register_device_item(device, 0, DP);
	state_save_register_device_item(device, 0, U);
	state_save_register_device_item(device, 0, S);
	state_save_register_device_item(device, 0, X);
	state_save_register_device_item(device, 0, Y);
	state_save_register_device_item(device, 0, CC);
	state_save_register_device_item_array(device, 0, m68_state->irq_state);
	state_save_register_device_item(device, 0, m68_state->int_state);
	state_save_register_device_item(device, 0, m68_state->nmi_state);

}

/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
static CPU_RESET( m6809 )
{
	m68_state_t *m68_state = get_safe_token(device);

	m68_state->int_state = 0;
	m68_state->nmi_state = CLEAR_LINE;
	m68_state->irq_state[0] = CLEAR_LINE;
	m68_state->irq_state[1] = CLEAR_LINE;

	DPD = 0;			/* Reset direct page register */

	CC |= CC_II;        /* IRQ disabled */
	CC |= CC_IF;        /* FIRQ disabled */

	PCD = RM16(m68_state, 0xfffe);
	UpdateState(m68_state);
}

static CPU_EXIT( m6809 )
{
	/* nothing to do ? */
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(m68_state_t *m68_state, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m68_state->nmi_state == state) return;
		m68_state->nmi_state = state;
		LOG(("M6809 '%s' set_irq_line (NMI) %d\n", m68_state->device->tag, state));
		if( state == CLEAR_LINE ) return;

		/* if the stack was not yet initialized */
	    if( !(m68_state->int_state & M6809_LDS) ) return;

	    m68_state->int_state &= ~M6809_SYNC;
		/* HJB 990225: state already saved by CWAI? */
		if( m68_state->int_state & M6809_CWAI )
		{
			m68_state->int_state &= ~M6809_CWAI;
			m68_state->extra_cycles += 7;	/* subtract +7 cycles next time */
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
			m68_state->extra_cycles += 19;	/* subtract +19 cycles next time */
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD = RM16(m68_state, 0xfffc);
	}
	else if (irqline < 2)
	{
	    LOG(("M6809 '%s' set_irq_line %d, %d\n", m68_state->device->tag, irqline, state));
		m68_state->irq_state[irqline] = state;
		if (state == CLEAR_LINE) return;
		check_irq_lines(m68_state);
	}
}

/****************************************************************************
 * includes the actual opcode implementations
 ****************************************************************************/
#include "6809tbl.c"

#include "6809ops.c"

/* execute instructions on this CPU until icount expires */
static CPU_EXECUTE( m6809 )	/* NS 970908 */
{
	m68_state_t *m68_state = get_safe_token(device);

    m68_state->icount = cycles - m68_state->extra_cycles;
	m68_state->extra_cycles = 0;

	check_irq_lines(m68_state);

	if (m68_state->int_state & (M6809_CWAI | M6809_SYNC))
	{
		debugger_instruction_hook(device, PCD);
		m68_state->icount = 0;
	}
	else
	{
		do
		{
			pPPC = pPC;

			debugger_instruction_hook(device, PCD);

			m68_state->ireg = ROP(PCD);
			PC++;
#if BIG_SWITCH
            switch( m68_state->ireg )
			{
			case 0x00: neg_di(m68_state);    break;
			case 0x01: neg_di(m68_state);    break;	/* undocumented */
			case 0x02: IIError(m68_state);   break;
			case 0x03: com_di(m68_state);    break;
			case 0x04: lsr_di(m68_state);    break;
			case 0x05: IIError(m68_state);   break;
			case 0x06: ror_di(m68_state);    break;
			case 0x07: asr_di(m68_state);    break;
			case 0x08: asl_di(m68_state);    break;
			case 0x09: rol_di(m68_state);    break;
			case 0x0a: dec_di(m68_state);    break;
			case 0x0b: IIError(m68_state);   break;
			case 0x0c: inc_di(m68_state);    break;
			case 0x0d: tst_di(m68_state);    break;
			case 0x0e: jmp_di(m68_state);    break;
			case 0x0f: clr_di(m68_state);    break;
			case 0x10: pref10(m68_state);					 break;
			case 0x11: pref11(m68_state);					 break;
			case 0x12: nop(m68_state);	    break;
			case 0x13: sync(m68_state);	    break;
			case 0x14: IIError(m68_state);   break;
			case 0x15: IIError(m68_state);   break;
			case 0x16: lbra(m68_state);	    break;
			case 0x17: lbsr(m68_state);	    break;
			case 0x18: IIError(m68_state);   break;
			case 0x19: daa(m68_state);	    break;
			case 0x1a: orcc(m68_state);	    break;
			case 0x1b: IIError(m68_state);   break;
			case 0x1c: andcc(m68_state);     break;
			case 0x1d: sex(m68_state);	    break;
			case 0x1e: exg(m68_state);	    break;
			case 0x1f: tfr(m68_state);	    break;
			case 0x20: bra(m68_state);	    break;
			case 0x21: brn(m68_state);	    break;
			case 0x22: bhi(m68_state);	    break;
			case 0x23: bls(m68_state);	    break;
			case 0x24: bcc(m68_state);	    break;
			case 0x25: bcs(m68_state);	    break;
			case 0x26: bne(m68_state);	    break;
			case 0x27: beq(m68_state);	    break;
			case 0x28: bvc(m68_state);	    break;
			case 0x29: bvs(m68_state);	    break;
			case 0x2a: bpl(m68_state);	    break;
			case 0x2b: bmi(m68_state);	    break;
			case 0x2c: bge(m68_state);	    break;
			case 0x2d: blt(m68_state);	    break;
			case 0x2e: bgt(m68_state);	    break;
			case 0x2f: ble(m68_state);	    break;
			case 0x30: leax(m68_state);	    break;
			case 0x31: leay(m68_state);	    break;
			case 0x32: leas(m68_state);	    break;
			case 0x33: leau(m68_state);	    break;
			case 0x34: pshs(m68_state);	    break;
			case 0x35: puls(m68_state);	    break;
			case 0x36: pshu(m68_state);	    break;
			case 0x37: pulu(m68_state);	    break;
			case 0x38: IIError(m68_state);   break;
			case 0x39: rts(m68_state);	    break;
			case 0x3a: abx(m68_state);	    break;
			case 0x3b: rti(m68_state);	    break;
			case 0x3c: cwai(m68_state);	    break;
			case 0x3d: mul(m68_state);	    break;
			case 0x3e: IIError(m68_state);   break;
			case 0x3f: swi(m68_state);	    break;
			case 0x40: nega(m68_state);	    break;
			case 0x41: IIError(m68_state);   break;
			case 0x42: IIError(m68_state);   break;
			case 0x43: coma(m68_state);	    break;
			case 0x44: lsra(m68_state);	    break;
			case 0x45: IIError(m68_state);   break;
			case 0x46: rora(m68_state);	    break;
			case 0x47: asra(m68_state);	    break;
			case 0x48: asla(m68_state);	    break;
			case 0x49: rola(m68_state);	    break;
			case 0x4a: deca(m68_state);	    break;
			case 0x4b: IIError(m68_state);   break;
			case 0x4c: inca(m68_state);	    break;
			case 0x4d: tsta(m68_state);	    break;
			case 0x4e: IIError(m68_state);   break;
			case 0x4f: clra(m68_state);	    break;
			case 0x50: negb(m68_state);	    break;
			case 0x51: IIError(m68_state);   break;
			case 0x52: IIError(m68_state);   break;
			case 0x53: comb(m68_state);	    break;
			case 0x54: lsrb(m68_state);	    break;
			case 0x55: IIError(m68_state);   break;
			case 0x56: rorb(m68_state);	    break;
			case 0x57: asrb(m68_state);	    break;
			case 0x58: aslb(m68_state);	    break;
			case 0x59: rolb(m68_state);	    break;
			case 0x5a: decb(m68_state);	    break;
			case 0x5b: IIError(m68_state);   break;
			case 0x5c: incb(m68_state);	    break;
			case 0x5d: tstb(m68_state);	    break;
			case 0x5e: IIError(m68_state);   break;
			case 0x5f: clrb(m68_state);	    break;
			case 0x60: neg_ix(m68_state);    break;
			case 0x61: IIError(m68_state);   break;
			case 0x62: IIError(m68_state);   break;
			case 0x63: com_ix(m68_state);    break;
			case 0x64: lsr_ix(m68_state);    break;
			case 0x65: IIError(m68_state);   break;
			case 0x66: ror_ix(m68_state);    break;
			case 0x67: asr_ix(m68_state);    break;
			case 0x68: asl_ix(m68_state);    break;
			case 0x69: rol_ix(m68_state);    break;
			case 0x6a: dec_ix(m68_state);    break;
			case 0x6b: IIError(m68_state);   break;
			case 0x6c: inc_ix(m68_state);    break;
			case 0x6d: tst_ix(m68_state);    break;
			case 0x6e: jmp_ix(m68_state);    break;
			case 0x6f: clr_ix(m68_state);    break;
			case 0x70: neg_ex(m68_state);    break;
			case 0x71: IIError(m68_state);   break;
			case 0x72: IIError(m68_state);   break;
			case 0x73: com_ex(m68_state);    break;
			case 0x74: lsr_ex(m68_state);    break;
			case 0x75: IIError(m68_state);   break;
			case 0x76: ror_ex(m68_state);    break;
			case 0x77: asr_ex(m68_state);    break;
			case 0x78: asl_ex(m68_state);    break;
			case 0x79: rol_ex(m68_state);    break;
			case 0x7a: dec_ex(m68_state);    break;
			case 0x7b: IIError(m68_state);   break;
			case 0x7c: inc_ex(m68_state);    break;
			case 0x7d: tst_ex(m68_state);    break;
			case 0x7e: jmp_ex(m68_state);    break;
			case 0x7f: clr_ex(m68_state);    break;
			case 0x80: suba_im(m68_state);   break;
			case 0x81: cmpa_im(m68_state);   break;
			case 0x82: sbca_im(m68_state);   break;
			case 0x83: subd_im(m68_state);   break;
			case 0x84: anda_im(m68_state);   break;
			case 0x85: bita_im(m68_state);   break;
			case 0x86: lda_im(m68_state);    break;
			case 0x87: sta_im(m68_state);    break;
			case 0x88: eora_im(m68_state);   break;
			case 0x89: adca_im(m68_state);   break;
			case 0x8a: ora_im(m68_state);    break;
			case 0x8b: adda_im(m68_state);   break;
			case 0x8c: cmpx_im(m68_state);   break;
			case 0x8d: bsr(m68_state);	    break;
			case 0x8e: ldx_im(m68_state);    break;
			case 0x8f: stx_im(m68_state);    break;
			case 0x90: suba_di(m68_state);   break;
			case 0x91: cmpa_di(m68_state);   break;
			case 0x92: sbca_di(m68_state);   break;
			case 0x93: subd_di(m68_state);   break;
			case 0x94: anda_di(m68_state);   break;
			case 0x95: bita_di(m68_state);   break;
			case 0x96: lda_di(m68_state);    break;
			case 0x97: sta_di(m68_state);    break;
			case 0x98: eora_di(m68_state);   break;
			case 0x99: adca_di(m68_state);   break;
			case 0x9a: ora_di(m68_state);    break;
			case 0x9b: adda_di(m68_state);   break;
			case 0x9c: cmpx_di(m68_state);   break;
			case 0x9d: jsr_di(m68_state);    break;
			case 0x9e: ldx_di(m68_state);    break;
			case 0x9f: stx_di(m68_state);    break;
			case 0xa0: suba_ix(m68_state);   break;
			case 0xa1: cmpa_ix(m68_state);   break;
			case 0xa2: sbca_ix(m68_state);   break;
			case 0xa3: subd_ix(m68_state);   break;
			case 0xa4: anda_ix(m68_state);   break;
			case 0xa5: bita_ix(m68_state);   break;
			case 0xa6: lda_ix(m68_state);    break;
			case 0xa7: sta_ix(m68_state);    break;
			case 0xa8: eora_ix(m68_state);   break;
			case 0xa9: adca_ix(m68_state);   break;
			case 0xaa: ora_ix(m68_state);    break;
			case 0xab: adda_ix(m68_state);   break;
			case 0xac: cmpx_ix(m68_state);   break;
			case 0xad: jsr_ix(m68_state);    break;
			case 0xae: ldx_ix(m68_state);    break;
			case 0xaf: stx_ix(m68_state);    break;
			case 0xb0: suba_ex(m68_state);   break;
			case 0xb1: cmpa_ex(m68_state);   break;
			case 0xb2: sbca_ex(m68_state);   break;
			case 0xb3: subd_ex(m68_state);   break;
			case 0xb4: anda_ex(m68_state);   break;
			case 0xb5: bita_ex(m68_state);   break;
			case 0xb6: lda_ex(m68_state);    break;
			case 0xb7: sta_ex(m68_state);    break;
			case 0xb8: eora_ex(m68_state);   break;
			case 0xb9: adca_ex(m68_state);   break;
			case 0xba: ora_ex(m68_state);    break;
			case 0xbb: adda_ex(m68_state);   break;
			case 0xbc: cmpx_ex(m68_state);   break;
			case 0xbd: jsr_ex(m68_state);    break;
			case 0xbe: ldx_ex(m68_state);    break;
			case 0xbf: stx_ex(m68_state);    break;
			case 0xc0: subb_im(m68_state);   break;
			case 0xc1: cmpb_im(m68_state);   break;
			case 0xc2: sbcb_im(m68_state);   break;
			case 0xc3: addd_im(m68_state);   break;
			case 0xc4: andb_im(m68_state);   break;
			case 0xc5: bitb_im(m68_state);   break;
			case 0xc6: ldb_im(m68_state);    break;
			case 0xc7: stb_im(m68_state);    break;
			case 0xc8: eorb_im(m68_state);   break;
			case 0xc9: adcb_im(m68_state);   break;
			case 0xca: orb_im(m68_state);    break;
			case 0xcb: addb_im(m68_state);   break;
			case 0xcc: ldd_im(m68_state);    break;
			case 0xcd: std_im(m68_state);    break;
			case 0xce: ldu_im(m68_state);    break;
			case 0xcf: stu_im(m68_state);    break;
			case 0xd0: subb_di(m68_state);   break;
			case 0xd1: cmpb_di(m68_state);   break;
			case 0xd2: sbcb_di(m68_state);   break;
			case 0xd3: addd_di(m68_state);   break;
			case 0xd4: andb_di(m68_state);   break;
			case 0xd5: bitb_di(m68_state);   break;
			case 0xd6: ldb_di(m68_state);    break;
			case 0xd7: stb_di(m68_state);    break;
			case 0xd8: eorb_di(m68_state);   break;
			case 0xd9: adcb_di(m68_state);   break;
			case 0xda: orb_di(m68_state);    break;
			case 0xdb: addb_di(m68_state);   break;
			case 0xdc: ldd_di(m68_state);    break;
			case 0xdd: std_di(m68_state);    break;
			case 0xde: ldu_di(m68_state);    break;
			case 0xdf: stu_di(m68_state);    break;
			case 0xe0: subb_ix(m68_state);   break;
			case 0xe1: cmpb_ix(m68_state);   break;
			case 0xe2: sbcb_ix(m68_state);   break;
			case 0xe3: addd_ix(m68_state);   break;
			case 0xe4: andb_ix(m68_state);   break;
			case 0xe5: bitb_ix(m68_state);   break;
			case 0xe6: ldb_ix(m68_state);    break;
			case 0xe7: stb_ix(m68_state);    break;
			case 0xe8: eorb_ix(m68_state);   break;
			case 0xe9: adcb_ix(m68_state);   break;
			case 0xea: orb_ix(m68_state);    break;
			case 0xeb: addb_ix(m68_state);   break;
			case 0xec: ldd_ix(m68_state);    break;
			case 0xed: std_ix(m68_state);    break;
			case 0xee: ldu_ix(m68_state);    break;
			case 0xef: stu_ix(m68_state);    break;
			case 0xf0: subb_ex(m68_state);   break;
			case 0xf1: cmpb_ex(m68_state);   break;
			case 0xf2: sbcb_ex(m68_state);   break;
			case 0xf3: addd_ex(m68_state);   break;
			case 0xf4: andb_ex(m68_state);   break;
			case 0xf5: bitb_ex(m68_state);   break;
			case 0xf6: ldb_ex(m68_state);    break;
			case 0xf7: stb_ex(m68_state);    break;
			case 0xf8: eorb_ex(m68_state);   break;
			case 0xf9: adcb_ex(m68_state);   break;
			case 0xfa: orb_ex(m68_state);    break;
			case 0xfb: addb_ex(m68_state);   break;
			case 0xfc: ldd_ex(m68_state);    break;
			case 0xfd: std_ex(m68_state);    break;
			case 0xfe: ldu_ex(m68_state);    break;
			case 0xff: stu_ex(m68_state);    break;
			}
#else
            		(*m6809_main[m68_state->ireg])(m68_state);
#endif    /* BIG_SWITCH */
        		m68_state->icount -= cycles1[m68_state->ireg];

		} while( m68_state->icount > 0 );

        m68_state->icount -= m68_state->extra_cycles;
		m68_state->extra_cycles = 0;
    }

    return cycles - m68_state->icount;   /* NS 970908 */
}

INLINE void fetch_effective_address( m68_state_t *m68_state )
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
	case 0x88: IMMBYTE(EA); 	EA=X+SIGNED(EA);					   break; /* this is a hack to make Vectrex work. It should be m68_state->icount-=1. Dunno where the cycle was lost :( */
	case 0x89: IMMWORD(EAP);	EA+=X;								   break;
	case 0x8a: EA=0;												   break; /*   IIError*/
	case 0x8b: EA=X+D;												   break;
	case 0x8c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0x8d: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0x8e: EA=0;												   break; /*   ILLEGAL*/
	case 0x8f: IMMWORD(EAP);										   break;

	case 0x90: EA=X;	X++;						EAD=RM16(m68_state, EAD);	   break; /* Indirect ,R+ not in my specs */
	case 0x91: EA=X;	X+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0x92: X--; 	EA=X;						EAD=RM16(m68_state, EAD);	   break;
	case 0x93: X-=2;	EA=X;						EAD=RM16(m68_state, EAD);	   break;
	case 0x94: EA=X;								EAD=RM16(m68_state, EAD);	   break;
	case 0x95: EA=X+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0x96: EA=X+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0x97: EA=0;												   break; /*   ILLEGAL*/
	case 0x98: IMMBYTE(EA); 	EA=X+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0x99: IMMWORD(EAP);	EA+=X;				EAD=RM16(m68_state, EAD);	   break;
	case 0x9a: EA=0;												   break; /*   ILLEGAL*/
	case 0x9b: EA=X+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0x9c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0x9d: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0x9e: EA=0;												   break; /*   ILLEGAL*/
	case 0x9f: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;

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

	case 0xb0: EA=Y;	Y++;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb1: EA=Y;	Y+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb2: Y--; 	EA=Y;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb3: Y-=2;	EA=Y;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb4: EA=Y;								EAD=RM16(m68_state, EAD);	   break;
	case 0xb5: EA=Y+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0xb6: EA=Y+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0xb7: EA=0;												   break; /*   ILLEGAL*/
	case 0xb8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xb9: IMMWORD(EAP);	EA+=Y;				EAD=RM16(m68_state, EAD);	   break;
	case 0xba: EA=0;												   break; /*   ILLEGAL*/
	case 0xbb: EA=Y+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0xbc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xbd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0xbe: EA=0;												   break; /*   ILLEGAL*/
	case 0xbf: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;

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

	case 0xd0: EA=U;	U++;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd1: EA=U;	U+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd2: U--; 	EA=U;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd3: U-=2;	EA=U;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd4: EA=U;								EAD=RM16(m68_state, EAD);	   break;
	case 0xd5: EA=U+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0xd6: EA=U+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0xd7: EA=0;												   break; /*ILLEGAL*/
	case 0xd8: IMMBYTE(EA); 	EA=U+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xd9: IMMWORD(EAP);	EA+=U;				EAD=RM16(m68_state, EAD);	   break;
	case 0xda: EA=0;												   break; /*ILLEGAL*/
	case 0xdb: EA=U+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0xdc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xdd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0xde: EA=0;												   break; /*ILLEGAL*/
	case 0xdf: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;

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

	case 0xf0: EA=S;	S++;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf1: EA=S;	S+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf2: S--; 	EA=S;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf3: S-=2;	EA=S;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf4: EA=S;								EAD=RM16(m68_state, EAD);	   break;
	case 0xf5: EA=S+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0xf6: EA=S+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0xf7: EA=0;												   break; /*ILLEGAL*/
	case 0xf8: IMMBYTE(EA); 	EA=S+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xf9: IMMWORD(EAP);	EA+=S;				EAD=RM16(m68_state, EAD);	   break;
	case 0xfa: EA=0;												   break; /*ILLEGAL*/
	case 0xfb: EA=S+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0xfc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xfd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0xfe: EA=0;												   break; /*ILLEGAL*/
	case 0xff: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;
	}
	m68_state->icount -= index_cycle_em[postbyte];
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m6809 )
{
	m68_state_t *m68_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6809_IRQ_LINE:	set_irq_line(m68_state, M6809_IRQ_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + M6809_FIRQ_LINE:	set_irq_line(m68_state, M6809_FIRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(m68_state, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M6809_PC:			PC = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M6809_S:			S = info->i;							break;
		case CPUINFO_INT_REGISTER + M6809_CC:			CC = info->i; check_irq_lines(m68_state);			break;
		case CPUINFO_INT_REGISTER + M6809_U:			U = info->i;							break;
		case CPUINFO_INT_REGISTER + M6809_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + M6809_B:			B = info->i;							break;
		case CPUINFO_INT_REGISTER + M6809_X:			X = info->i;							break;
		case CPUINFO_INT_REGISTER + M6809_Y:			Y = info->i;							break;
		case CPUINFO_INT_REGISTER + M6809_DP:			DP = info->i;							break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m6809 )
{
	m68_state_t *m68_state = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m68_state_t);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 19;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6809_IRQ_LINE:	info->i = m68_state->irq_state[M6809_IRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + M6809_FIRQ_LINE:	info->i = m68_state->irq_state[M6809_FIRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = m68_state->nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M6809_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M6809_S:			info->i = S;							break;
		case CPUINFO_INT_REGISTER + M6809_CC:			info->i = CC;							break;
		case CPUINFO_INT_REGISTER + M6809_U:			info->i = U;							break;
		case CPUINFO_INT_REGISTER + M6809_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + M6809_B:			info->i = B;							break;
		case CPUINFO_INT_REGISTER + M6809_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + M6809_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + M6809_DP:			info->i = DP;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m6809);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6809);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m6809);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m6809);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6809);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6809);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68_state->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6809");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Motorola 6809");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.11");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright John Butler"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				m68_state->cc & 0x80 ? 'E':'.',
				m68_state->cc & 0x40 ? 'F':'.',
                m68_state->cc & 0x20 ? 'H':'.',
                m68_state->cc & 0x10 ? 'I':'.',
                m68_state->cc & 0x08 ? 'N':'.',
                m68_state->cc & 0x04 ? 'Z':'.',
                m68_state->cc & 0x02 ? 'V':'.',
                m68_state->cc & 0x01 ? 'C':'.');
            break;

		case CPUINFO_STR_REGISTER + M6809_PC:			sprintf(info->s, "PC:%04X", m68_state->pc.w.l); break;
		case CPUINFO_STR_REGISTER + M6809_S:			sprintf(info->s, "S:%04X", m68_state->s.w.l); break;
		case CPUINFO_STR_REGISTER + M6809_CC:			sprintf(info->s, "CC:%02X", m68_state->cc); break;
		case CPUINFO_STR_REGISTER + M6809_U:			sprintf(info->s, "U:%04X", m68_state->u.w.l); break;
		case CPUINFO_STR_REGISTER + M6809_A:			sprintf(info->s, "A:%02X", m68_state->d.b.h); break;
		case CPUINFO_STR_REGISTER + M6809_B:			sprintf(info->s, "B:%02X", m68_state->d.b.l); break;
		case CPUINFO_STR_REGISTER + M6809_X:			sprintf(info->s, "X:%04X", m68_state->x.w.l); break;
		case CPUINFO_STR_REGISTER + M6809_Y:			sprintf(info->s, "Y:%04X", m68_state->y.w.l); break;
		case CPUINFO_STR_REGISTER + M6809_DP:			sprintf(info->s, "DP:%02X", m68_state->dp.b.h); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( m6809e )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M6809E");				break;

		default:										CPU_GET_INFO_CALL(m6809);				break;
	}
}
