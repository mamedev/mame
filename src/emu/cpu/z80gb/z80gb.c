/*************************************************************/
/**                                                         **/
/**                          z80gb.c                        **/
/**                                                         **/
/** This file contains implementation for the GameBoy CPU.  **/
/** See z80gb.h for the relevant definitions. Please, note  **/
/** that this code can not be used to emulate a generic Z80 **/
/** because the GameBoy version of it differs from Z80 in   **/
/** many ways.                                              **/
/**                                                         **/
/** Orginal cpu code (PlayBoy)  Carsten Sorensen    1998    **/
/** MESS modifications          Hans de Goede       1998    **/
/** Adapted to new cpuintrf     Juergen Buchmueller 2000    **/
/** Adapted to new cpuintrf     Anthony Kruize      2002    **/
/** Changed reset function to                               **/
/** reset all registers instead                             **/
/** of just AF.                            Wilbert Pol 2004 **/
/**                                                         **/
/** 1.1:                                                    **/
/**   Removed dependency on the mess gameboy driver         **/
/**                                                         **/
/** 1.2:                                                    **/
/**   Fixed cycle count for taking an interrupt             **/
/**   Fixed cycle count for BIT X,(HL) instructions         **/
/**   Fixed flags in RRCA instruction                       **/
/**   Fixed DAA instruction                                 **/
/**   Fixed flags in ADD SP,n8 instruction                  **/
/**   Fixed flags in LD HL,SP+n8 instruction                **/
/**                                                         **/
/** 1.3:                                                    **/
/**   Improved triggering of the HALT bug                   **/
/**   Added 4 cycle penalty when leaving HALT state for     **/
/**   newer versions of the cpu core                        **/
/**                                                         **/
/** 1.4:                                                    **/
/**   Split fetch and execute cycles.                       **/
/**                                                         **/
/*************************************************************/
#include "debugger.h"
#include "deprecat.h"
#include "z80gb.h"

#define FLAG_Z	0x80
#define FLAG_N  0x40
#define FLAG_H  0x20
#define FLAG_C  0x10

#define CYCLES_PASSED(X)		z80gb_ICount -= ((X) / (Regs.w.gb_speed));	\
					if ( Regs.w.timer_callback ) {			\
						Regs.w.timer_callback( X );		\
					}

typedef struct {
	UINT16 AF;
	UINT16 BC;
	UINT16 DE;
	UINT16 HL;

	UINT16 SP;
	UINT16 PC;
	/* Interrupt related */
	UINT8	IE;
	UINT8	IF;
	int	irq_state;
	int	ei_delay;
	int	(*irq_callback)(int irqline);
	/* Timer stuff */
	void	(*timer_callback)(int cycles);
	/* Fetch & execute related */
	int		execution_state;
	UINT8	op;
	/* Others */
	int gb_speed;
	int gb_speed_change_pending;
	int enable;
	int doHALTbug;
	int haltIFstatus;
	UINT8	features;
	const Z80GB_CONFIG *config;
} z80gb_16BitRegs;

#ifdef LSB_FIRST
typedef struct {
	UINT8 F;
	UINT8 A;
	UINT8 C;
	UINT8 B;
	UINT8 E;
	UINT8 D;
	UINT8 L;
	UINT8 H;
} z80gb_8BitRegs;
#else
typedef struct {
	UINT8 A;
	UINT8 F;
	UINT8 B;
	UINT8 C;
	UINT8 D;
	UINT8 E;
	UINT8 H;
	UINT8 L;
} z80gb_8BitRegs;
#endif

typedef union {
	z80gb_16BitRegs w;
	z80gb_8BitRegs b;
} z80gb_regs;

typedef int (*OpcodeEmulator) (void);

static z80gb_regs Regs;

#define IME     0x01
#define HALTED	0x02

/****************************************************************************/
/* Memory functions                                                         */
/****************************************************************************/

#define mem_ReadByte(A)		((UINT8)program_read_byte_8(A))
#define mem_WriteByte(A,V)	(program_write_byte_8(A,V))

INLINE UINT16 mem_ReadWord (UINT32 address)
{
	UINT16 value = (UINT16) mem_ReadByte ((address + 1) & 0xffff) << 8;
	value |= mem_ReadByte (address);
	return value;
}

INLINE void mem_WriteWord (UINT32 address, UINT16 value)
{
	mem_WriteByte (address, value & 0xFF);
	mem_WriteByte ((address + 1) & 0xffff, value >> 8);
}

/* Nr of cycles to run */
static int z80gb_ICount;

static const int Cycles[256] =
{
	 4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,
	 4,12, 8, 8, 4, 4, 8, 4,12, 8, 8, 8, 4, 4, 8, 4,
	 8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,
	 8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	 8,12,12,16,12,16, 8,16, 8,16,12, 0,12,24, 8,16,
	 8,12,12, 4,12,16, 8,16, 8,16,12, 4,12, 4, 8,16,
	12,12, 8, 4, 4,16, 8,16,16, 4,16, 4, 4, 4, 8,16,
	12,12, 8, 4, 4,16, 8,16,12, 8,16, 4, 4, 4, 8,16
};

static const int CyclesCB[256] =
{
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
	 8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8
};

static void z80gb_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	Regs.w.config = (const Z80GB_CONFIG *) config;
	Regs.w.irq_callback = irqcallback;
}

/*** Reset Z80 registers: *********************************/
/*** This function can be used to reset the register    ***/
/*** file before starting execution with z80gb_execute()***/
/*** It sets the registers to their initial values.     ***/
/**********************************************************/
static void z80gb_reset(void)
{
	Regs.w.AF = 0x0000;
	Regs.w.BC = 0x0000;
	Regs.w.DE = 0x0000;
	Regs.w.HL = 0x0000;
	Regs.w.SP = 0x0000;
	Regs.w.PC = 0x0000;
	Regs.w.timer_callback = NULL;
	Regs.w.features = Z80GB_FEATURE_HALT_BUG;
	if (Regs.w.config)
	{
		if ( Regs.w.config->regs ) {
			Regs.w.AF = Regs.w.config->regs[0];
			Regs.w.BC = Regs.w.config->regs[1];
			Regs.w.DE = Regs.w.config->regs[2];
			Regs.w.HL = Regs.w.config->regs[3];
			Regs.w.SP = Regs.w.config->regs[4];
			Regs.w.PC = Regs.w.config->regs[5];
		}
		Regs.w.timer_callback = Regs.w.config->timer_callback;
		Regs.w.features = Regs.w.config->features;
	}
	Regs.w.enable = 0;
	Regs.w.IE = 0;
	Regs.w.IF = 0;

	Regs.w.execution_state = 0;
	Regs.w.doHALTbug = 0;
	Regs.w.ei_delay = 0;
	Regs.w.gb_speed_change_pending = 0;
	Regs.w.gb_speed = 1;
}

INLINE void z80gb_ProcessInterrupts (void)
{
	UINT8 irq = Regs.w.IE & Regs.w.IF;

	/* Interrupts should be taken after the first instruction after an EI instruction */
	if (Regs.w.ei_delay) {
		Regs.w.ei_delay = 0;
		return;
	}

	/*
       logerror("Attempting to process Z80GB Interrupt IRQ $%02X\n", irq);
       logerror("Attempting to process Z80GB Interrupt IE $%02X\n", Regs.w.IE);
       logerror("Attempting to process Z80GB Interrupt IF $%02X\n", Regs.w.IF);
    */
	if (irq)
	{
		int irqline = 0;
		/*
           logerror("Z80GB Interrupt IRQ $%02X\n", irq);
        */

		for( ; irqline < 5; irqline++ )
		{
			if( irq & (1<<irqline) )
			{
				if (Regs.w.enable & HALTED)
				{
					Regs.w.enable &= ~HALTED;
					Regs.w.IF &= ~(1 << irqline);
					Regs.w.PC++;
					if ( Regs.w.features & Z80GB_FEATURE_HALT_BUG ) {
						if ( ! Regs.w.enable & IME ) {
							/* Old cpu core (dmg/mgb/sgb) */
							/* check if the HALT bug should be performed */
							if ( Regs.w.haltIFstatus ) {
								Regs.w.doHALTbug = 1;
							}
						}
					} else {
						/* New cpu core (cgb/agb/ags) */
						/* Adjust for internal syncing with video core */
						/* This feature needs more investigation */
						if ( irqline < 2 ) {
							CYCLES_PASSED( 4 );
						}
					}
				}
				if ( Regs.w.enable & IME ) {
					if ( Regs.w.irq_callback )
						(*Regs.w.irq_callback)(irqline);
					Regs.w.enable &= ~IME;
					Regs.w.IF &= ~(1 << irqline);
					CYCLES_PASSED( 20 );
					Regs.w.SP -= 2;
					mem_WriteWord (Regs.w.SP, Regs.w.PC);
					Regs.w.PC = 0x40 + irqline * 8;
					/*logerror("Z80GB Interrupt PC $%04X\n", Regs.w.PC );*/
					return;
				}
			}
		}
	}
}

/**********************************************************/
/*** Execute z80gb code for cycles cycles, return nr of ***/
/*** cycles actually executed.                          ***/
/**********************************************************/
static int z80gb_execute (int cycles)
{
	z80gb_ICount = cycles;

	do
	{
		if ( Regs.w.execution_state ) {
			UINT8	x;
			/* Execute instruction */
			switch( Regs.w.op ) {
#include "opc_main.h"
			}
		} else {
			/* Fetch and count cycles */
			z80gb_ProcessInterrupts ();
			CALL_DEBUGGER(Regs.w.PC);
			if ( Regs.w.enable & HALTED ) {
				CYCLES_PASSED( Cycles[0x76] );
				Regs.w.execution_state = 1;
			} else {
				Regs.w.op = mem_ReadByte (Regs.w.PC++);
				if ( Regs.w.doHALTbug ) {
					Regs.w.PC--;
					Regs.w.doHALTbug = 0;
				}
				CYCLES_PASSED( Cycles[Regs.w.op] );
			}
		}
		Regs.w.execution_state ^= 1;
	} while (z80gb_ICount > 0);

	return cycles - z80gb_ICount;
}

static void z80gb_burn(int cycles)
{
    if( cycles > 0 )
    {
        /* NOP takes 4 cycles per instruction */
        int n = (cycles + 3) / 4;
        z80gb_ICount -= 4 * n;
    }
}

/****************************************************************************/
/* Set all registers to given values                                        */
/****************************************************************************/
static void z80gb_set_context (void *src)
{
	if( src )
		Regs = *(z80gb_regs *)src;
	change_pc(Regs.w.PC);
}

/****************************************************************************/
/* Get all registers in given buffer                                        */
/****************************************************************************/
static void z80gb_get_context (void *dst)
{
	if( dst )
		*(z80gb_regs *)dst = Regs;
}



static void z80gb_set_irq_line (int irqline, int state)
{
	/*logerror("setting irq line 0x%02x state 0x%08x\n", irqline, state);*/
	//if( Regs.w.irq_state == state )
	//  return;

	Regs.w.irq_state = state;
	if( state == ASSERT_LINE )
	{

		Regs.w.IF |= (0x01 << irqline);
		/*logerror("Z80GB assert irq line %d ($%02X)\n", irqline, Regs.w.IF);*/

	}
	else
	{

		Regs.w.IF &= ~(0x01 << irqline);
		/*logerror("Z80GB clear irq line %d ($%02X)\n", irqline, Regs.w.IF);*/

     }
}

/*static void z80gb_clear_pending_interrupts (void)
{
    Regs.w.IF = 0;
}*/

static void z80gb_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:				z80gb_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i); break;

	case CPUINFO_INT_SP:						Regs.w.SP = info->i;						break;
	case CPUINFO_INT_PC:						Regs.w.PC = info->i; change_pc(Regs.w.PC);	break;

	case CPUINFO_INT_REGISTER + Z80GB_PC:		Regs.w.PC = info->i; change_pc(Regs.w.PC);	break;
	case CPUINFO_INT_REGISTER + Z80GB_SP:		Regs.w.SP = info->i;						break;
	case CPUINFO_INT_REGISTER + Z80GB_AF:		Regs.w.AF = info->i;						break;
	case CPUINFO_INT_REGISTER + Z80GB_BC:		Regs.w.BC = info->i;						break;
	case CPUINFO_INT_REGISTER + Z80GB_DE:		Regs.w.DE = info->i;						break;
	case CPUINFO_INT_REGISTER + Z80GB_HL:		Regs.w.HL = info->i;						break;
	case CPUINFO_INT_REGISTER + Z80GB_IE:		Regs.w.IE = info->i; break;
	case CPUINFO_INT_REGISTER + Z80GB_IF:		Regs.w.IF = info->i; break;
	case CPUINFO_INT_REGISTER + Z80GB_SPEED:	Regs.w.gb_speed_change_pending = info->i & 0x01; break;
	}
}

void z80gb_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(Regs);					break;
	case CPUINFO_INT_INPUT_LINES:						info->i = 5;							break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
	case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1;	/* right? */			break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 16;	/* right? */			break;

	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 16;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

	case CPUINFO_INT_SP:							info->i = Regs.w.SP;					break;
	case CPUINFO_INT_PC:							info->i = Regs.w.PC;					break;
	case CPUINFO_INT_PREVIOUSPC:					info->i = 0;	/* TODO??? */			break;

	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:					info->i = Regs.w.IF & (1 << (state-CPUINFO_INT_INPUT_STATE)); break;

	case CPUINFO_INT_REGISTER + Z80GB_PC:			info->i = Regs.w.PC;					break;
	case CPUINFO_INT_REGISTER + Z80GB_SP:			info->i = Regs.w.SP;					break;
	case CPUINFO_INT_REGISTER + Z80GB_AF:			info->i = Regs.w.AF;					break;
	case CPUINFO_INT_REGISTER + Z80GB_BC:			info->i = Regs.w.BC;					break;
	case CPUINFO_INT_REGISTER + Z80GB_DE:			info->i = Regs.w.DE;					break;
	case CPUINFO_INT_REGISTER + Z80GB_HL:			info->i = Regs.w.HL;					break;
	case CPUINFO_INT_REGISTER + Z80GB_IE:			info->i = Regs.w.IE;					break;
	case CPUINFO_INT_REGISTER + Z80GB_IF:			info->i = Regs.w.IF;					break;
	case CPUINFO_INT_REGISTER + Z80GB_SPEED:		info->i = 0x7E | ( ( Regs.w.gb_speed - 1 ) << 7 ) | Regs.w.gb_speed_change_pending; break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_PTR_SET_INFO:						info->setinfo = z80gb_set_info;			break;
	case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = z80gb_get_context;	break;
	case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = z80gb_set_context;	break;
	case CPUINFO_PTR_INIT:							info->init = z80gb_init;				break;
	case CPUINFO_PTR_RESET:							info->reset = z80gb_reset;				break;
	case CPUINFO_PTR_EXECUTE:						info->execute = z80gb_execute;			break;
	case CPUINFO_PTR_BURN:							info->burn = z80gb_burn;						break;

#ifdef ENABLE_DEBUGGER
	case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = z80gb_dasm;	break;
#endif
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &z80gb_ICount;			break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME: 							strcpy(info->s, "Z80GB"); break;
	case CPUINFO_STR_CORE_FAMILY: 					strcpy(info->s, "Nintendo Z80"); break;
	case CPUINFO_STR_CORE_VERSION: 					strcpy(info->s, "1.4"); break;
	case CPUINFO_STR_CORE_FILE: 					strcpy(info->s, __FILE__); break;
	case CPUINFO_STR_CORE_CREDITS: 					strcpy(info->s, "Copyright The MESS Team."); break;

	case CPUINFO_STR_FLAGS:
		sprintf(info->s, "%c%c%c%c%c%c%c%c",
			Regs.b.F & 0x80 ? 'Z':'.',
			Regs.b.F & 0x40 ? 'N':'.',
			Regs.b.F & 0x20 ? 'H':'.',
			Regs.b.F & 0x10 ? 'C':'.',
			Regs.b.F & 0x08 ? '3':'.',
			Regs.b.F & 0x04 ? '2':'.',
			Regs.b.F & 0x02 ? '1':'.',
			Regs.b.F & 0x01 ? '0':'.');
		break;

	case CPUINFO_STR_REGISTER + Z80GB_PC: sprintf(info->s, "PC:%04X", Regs.w.PC); break;
	case CPUINFO_STR_REGISTER + Z80GB_SP: sprintf(info->s, "SP:%04X", Regs.w.SP); break;
	case CPUINFO_STR_REGISTER + Z80GB_AF: sprintf(info->s, "AF:%04X", Regs.w.AF); break;
	case CPUINFO_STR_REGISTER + Z80GB_BC: sprintf(info->s, "BC:%04X", Regs.w.BC); break;
	case CPUINFO_STR_REGISTER + Z80GB_DE: sprintf(info->s, "DE:%04X", Regs.w.DE); break;
	case CPUINFO_STR_REGISTER + Z80GB_HL: sprintf(info->s, "HL:%04X", Regs.w.HL); break;
	case CPUINFO_STR_REGISTER + Z80GB_IRQ_STATE: sprintf(info->s, "IRQ:%X", Regs.w.enable & IME ); break;
	case CPUINFO_STR_REGISTER + Z80GB_IE: sprintf(info->s, "IE:%02X", Regs.w.IE); break;
	case CPUINFO_STR_REGISTER + Z80GB_IF: sprintf(info->s, "IF:%02X", Regs.w.IF); break;
	}
}
