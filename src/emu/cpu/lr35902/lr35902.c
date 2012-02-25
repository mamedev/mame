/*************************************************************/
/**                                                         **/
/**                        lr35902.c                        **/
/**                                                         **/
/** This file contains implementation for the GameBoy CPU.  **/
/** See lr35902.h for the relevant definitions. Please, note**/
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

#include "emu.h"
#include "debugger.h"
#include "lr35902.h"

#define FLAG_Z	0x80
#define FLAG_N  0x40
#define FLAG_H  0x20
#define FLAG_C  0x10

#define CYCLES_PASSED(X)		cpustate->w.icount -= ((X) / (cpustate->w.gb_speed));	\
					if ( cpustate->w.timer_expired_func ) {			\
						cpustate->w.timer_expired_func( cpustate->w.device, X );		\
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
	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	int icount;
	/* Timer stuff */
	lr35902_timer_fired_func timer_expired_func;
	/* Fetch & execute related */
	int		execution_state;
	UINT8	op;
	/* Others */
	int gb_speed;
	int gb_speed_change_pending;
	int enable;
	int doHALTbug;
	UINT8	features;
	const lr35902_cpu_core *config;
} lr35902_16BitRegs;

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
} lr35902_8BitRegs;
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
} lr35902_8BitRegs;
#endif


typedef union _lr35902_state lr35902_state;
union _lr35902_state {
	lr35902_16BitRegs w;
	lr35902_8BitRegs b;
};

INLINE lr35902_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == LR35902);
	return (lr35902_state *)downcast<legacy_cpu_device *>(device)->token();
}

typedef int (*OpcodeEmulator) (lr35902_state *cpustate);

#define IME     0x01
#define HALTED	0x02

/****************************************************************************/
/* Memory functions                                                         */
/****************************************************************************/

#define mem_ReadByte(cs,A)		((UINT8)(cs)->w.program->read_byte(A)); CYCLES_PASSED(4);
#define mem_WriteByte(cs,A,V)	((cs)->w.program->write_byte(A,V)); CYCLES_PASSED(4);

INLINE UINT16 mem_ReadWord (lr35902_state *cpustate, UINT32 address)
{
	UINT16 value = mem_ReadByte (cpustate, (address + 1) & 0xffff);
	value <<= 8;
	value |= mem_ReadByte (cpustate, address);
	return value;
}

INLINE void mem_WriteWord (lr35902_state *cpustate, UINT32 address, UINT16 value)
{
	mem_WriteByte (cpustate, address, value & 0xFF);
	mem_WriteByte (cpustate, (address + 1) & 0xffff, value >> 8);
}

static CPU_INIT( lr35902 )
{
	lr35902_state *cpustate = get_safe_token(device);

	cpustate->w.config = (const lr35902_cpu_core *) device->static_config();
	cpustate->w.irq_callback = irqcallback;
	cpustate->w.device = device;
	cpustate->w.program = device->space(AS_PROGRAM);
}

/*** Reset lr353902 registers: ******************************/
/*** This function can be used to reset the register      ***/
/*** file before starting execution with lr35902_execute(cpustate)***/
/*** It sets the registers to their initial values.       ***/
/************************************************************/
static CPU_RESET( lr35902 )
{
	lr35902_state *cpustate = get_safe_token(device);

	cpustate->w.AF = 0x0000;
	cpustate->w.BC = 0x0000;
	cpustate->w.DE = 0x0000;
	cpustate->w.HL = 0x0000;
	cpustate->w.SP = 0x0000;
	cpustate->w.PC = 0x0000;
	cpustate->w.timer_expired_func = NULL;
	cpustate->w.features = LR35902_FEATURE_HALT_BUG;
	if (cpustate->w.config)
	{
		if ( cpustate->w.config->regs ) {
			cpustate->w.AF = cpustate->w.config->regs[0];
			cpustate->w.BC = cpustate->w.config->regs[1];
			cpustate->w.DE = cpustate->w.config->regs[2];
			cpustate->w.HL = cpustate->w.config->regs[3];
			cpustate->w.SP = cpustate->w.config->regs[4];
			cpustate->w.PC = cpustate->w.config->regs[5];
		}
		cpustate->w.timer_expired_func = cpustate->w.config->timer_expired_func;
		cpustate->w.features = cpustate->w.config->features;
	}
	cpustate->w.enable = 0;
	cpustate->w.IE = 0;
	cpustate->w.IF = 0;

	cpustate->w.execution_state = 0;
	cpustate->w.doHALTbug = 0;
	cpustate->w.ei_delay = 0;
	cpustate->w.gb_speed_change_pending = 0;
	cpustate->w.gb_speed = 1;
}

INLINE void lr35902_ProcessInterrupts (lr35902_state *cpustate)
{
	UINT8 irq = cpustate->w.IE & cpustate->w.IF;

	/* Interrupts should be taken after the first instruction after an EI instruction */
	if (cpustate->w.ei_delay) {
		cpustate->w.ei_delay = 0;
		return;
	}

	/*
       logerror("Attempting to process LR35902 Interrupt IRQ $%02X\n", irq);
       logerror("Attempting to process LR35902 Interrupt IE $%02X\n", cpustate->w.IE);
       logerror("Attempting to process LR35902 Interrupt IF $%02X\n", cpustate->w.IF);
    */
	if (irq)
	{
		int irqline = 0;
		/*
           logerror("LR35902 Interrupt IRQ $%02X\n", irq);
        */

		for( ; irqline < 5; irqline++ )
		{
			if( irq & (1<<irqline) )
			{
				if (cpustate->w.enable & HALTED)
				{
					cpustate->w.enable &= ~HALTED;
					cpustate->w.PC++;
					if ( cpustate->w.features & LR35902_FEATURE_HALT_BUG ) {
						if ( ! ( cpustate->w.enable & IME ) ) {
							/* Old cpu core (dmg/mgb/sgb) */
							cpustate->w.doHALTbug = 1;
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
				if ( cpustate->w.enable & IME ) {
					if ( cpustate->w.irq_callback )
						(*cpustate->w.irq_callback)(cpustate->w.device, irqline);
					cpustate->w.enable &= ~IME;
					cpustate->w.IF &= ~(1 << irqline);
					CYCLES_PASSED( 12 );
					cpustate->w.SP -= 2;
					mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
					cpustate->w.PC = 0x40 + irqline * 8;
					/*logerror("LR35902 Interrupt PC $%04X\n", cpustate->w.PC );*/
					return;
				}
			}
		}
	}
}

/************************************************************/
/*** Execute lr35902 code for cycles cycles, return nr of ***/
/*** cycles actually executed.                            ***/
/************************************************************/
static CPU_EXECUTE( lr35902 )
{
	lr35902_state *cpustate = get_safe_token(device);

	do
	{
		if ( cpustate->w.execution_state ) {
			UINT8	x;
			/* Execute instruction */
			switch( cpustate->w.op ) {
#include "opc_main.h"
			}
		} else {
			/* Fetch and count cycles */
			lr35902_ProcessInterrupts (cpustate);
			debugger_instruction_hook(device, cpustate->w.PC);
			if ( cpustate->w.enable & HALTED ) {
				CYCLES_PASSED( 4 );
				cpustate->w.execution_state = 1;
			} else {
				cpustate->w.op = mem_ReadByte (cpustate, cpustate->w.PC++);
				if ( cpustate->w.doHALTbug ) {
					cpustate->w.PC--;
					cpustate->w.doHALTbug = 0;
				}
			}
		}
		cpustate->w.execution_state ^= 1;
	} while (cpustate->w.icount > 0);
}

static CPU_BURN( lr35902 )
{
	lr35902_state *cpustate = get_safe_token(device);

    if( cycles > 0 )
    {
        /* NOP takes 4 cycles per instruction */
        int n = (cycles + 3) / 4;
        cpustate->w.icount -= 4 * n;
    }
}

static void lr35902_set_irq_line (lr35902_state *cpustate, int irqline, int state)
{
	/*logerror("setting irq line 0x%02x state 0x%08x\n", irqline, state);*/
	//if( cpustate->w.irq_state == state )
	//  return;

	cpustate->w.irq_state = state;
	if( state == ASSERT_LINE )
	{

		cpustate->w.IF |= (0x01 << irqline);
		/*logerror("LR35902 assert irq line %d ($%02X)\n", irqline, cpustate->w.IF);*/

	}
	else
	{

		cpustate->w.IF &= ~(0x01 << irqline);
		/*logerror("LR35902 clear irq line %d ($%02X)\n", irqline, cpustate->w.IF);*/

	}
}

#ifdef UNUSED_FUNCTION
static void lr35902_clear_pending_interrupts (lr35902_state *cpustate)
{
    cpustate->w.IF = 0;
}
#endif

static CPU_SET_INFO( lr35902 )
{
	lr35902_state *cpustate = get_safe_token(device);

	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:			lr35902_set_irq_line(cpustate, state-CPUINFO_INT_INPUT_STATE, info->i); break;

	case CPUINFO_INT_SP:						cpustate->w.SP = info->i;						break;
	case CPUINFO_INT_PC:						cpustate->w.PC = info->i;						break;

	case CPUINFO_INT_REGISTER + LR35902_PC:		cpustate->w.PC = info->i;						break;
	case CPUINFO_INT_REGISTER + LR35902_SP:		cpustate->w.SP = info->i;						break;
	case CPUINFO_INT_REGISTER + LR35902_AF:		cpustate->w.AF = info->i;						break;
	case CPUINFO_INT_REGISTER + LR35902_BC:		cpustate->w.BC = info->i;						break;
	case CPUINFO_INT_REGISTER + LR35902_DE:		cpustate->w.DE = info->i;						break;
	case CPUINFO_INT_REGISTER + LR35902_HL:		cpustate->w.HL = info->i;						break;
	case CPUINFO_INT_REGISTER + LR35902_IE:		cpustate->w.IE = info->i; break;
	case CPUINFO_INT_REGISTER + LR35902_IF:		cpustate->w.IF = info->i; break;
	case CPUINFO_INT_REGISTER + LR35902_SPEED:	cpustate->w.gb_speed_change_pending = info->i & 0x01; break;
	}
}

CPU_GET_INFO( lr35902 )
{
	lr35902_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(lr35902_state);					break;
	case CPUINFO_INT_INPUT_LINES:						info->i = 5;							break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
	case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1;	/* right? */			break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 16;	/* right? */			break;

	case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

	case CPUINFO_INT_SP:							info->i = cpustate->w.SP;					break;
	case CPUINFO_INT_PC:							info->i = cpustate->w.PC;					break;
	case CPUINFO_INT_PREVIOUSPC:					info->i = 0;	/* TODO??? */			break;

	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:					info->i = cpustate->w.IF & (1 << (state-CPUINFO_INT_INPUT_STATE)); break;

	case CPUINFO_INT_REGISTER + LR35902_PC:			info->i = cpustate->w.PC;					break;
	case CPUINFO_INT_REGISTER + LR35902_SP:			info->i = cpustate->w.SP;					break;
	case CPUINFO_INT_REGISTER + LR35902_AF:			info->i = cpustate->w.AF;					break;
	case CPUINFO_INT_REGISTER + LR35902_BC:			info->i = cpustate->w.BC;					break;
	case CPUINFO_INT_REGISTER + LR35902_DE:			info->i = cpustate->w.DE;					break;
	case CPUINFO_INT_REGISTER + LR35902_HL:			info->i = cpustate->w.HL;					break;
	case CPUINFO_INT_REGISTER + LR35902_IE:			info->i = cpustate->w.IE;					break;
	case CPUINFO_INT_REGISTER + LR35902_IF:			info->i = cpustate->w.IF;					break;
	case CPUINFO_INT_REGISTER + LR35902_SPEED:		info->i = 0x7E | ( ( cpustate->w.gb_speed - 1 ) << 7 ) | cpustate->w.gb_speed_change_pending; break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(lr35902);		break;
	case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(lr35902);				break;
	case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(lr35902);			break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(lr35902);		break;
	case CPUINFO_FCT_BURN:							info->burn = CPU_BURN_NAME(lr35902);				break;
	case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(lr35902);		break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->w.icount;			break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case DEVINFO_STR_NAME:							strcpy(info->s, "LR35902"); break;
	case DEVINFO_STR_FAMILY:					strcpy(info->s, "Sharp LR35902"); break;
	case DEVINFO_STR_VERSION:					strcpy(info->s, "1.4"); break;
	case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__); break;
	case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright The MESS Team."); break;

	case CPUINFO_STR_FLAGS:
		sprintf(info->s, "%c%c%c%c%c%c%c%c",
			cpustate->b.F & 0x80 ? 'Z':'.',
			cpustate->b.F & 0x40 ? 'N':'.',
			cpustate->b.F & 0x20 ? 'H':'.',
			cpustate->b.F & 0x10 ? 'C':'.',
			cpustate->b.F & 0x08 ? '3':'.',
			cpustate->b.F & 0x04 ? '2':'.',
			cpustate->b.F & 0x02 ? '1':'.',
			cpustate->b.F & 0x01 ? '0':'.');
		break;

	case CPUINFO_STR_REGISTER + LR35902_PC: sprintf(info->s, "PC:%04X", cpustate->w.PC); break;
	case CPUINFO_STR_REGISTER + LR35902_SP: sprintf(info->s, "SP:%04X", cpustate->w.SP); break;
	case CPUINFO_STR_REGISTER + LR35902_AF: sprintf(info->s, "AF:%04X", cpustate->w.AF); break;
	case CPUINFO_STR_REGISTER + LR35902_BC: sprintf(info->s, "BC:%04X", cpustate->w.BC); break;
	case CPUINFO_STR_REGISTER + LR35902_DE: sprintf(info->s, "DE:%04X", cpustate->w.DE); break;
	case CPUINFO_STR_REGISTER + LR35902_HL: sprintf(info->s, "HL:%04X", cpustate->w.HL); break;
	case CPUINFO_STR_REGISTER + LR35902_IRQ_STATE: sprintf(info->s, "IRQ:%X", cpustate->w.enable & IME ); break;
	case CPUINFO_STR_REGISTER + LR35902_IE: sprintf(info->s, "IE:%02X", cpustate->w.IE); break;
	case CPUINFO_STR_REGISTER + LR35902_IF: sprintf(info->s, "IF:%02X", cpustate->w.IF); break;
	case CPUINFO_STR_REGISTER + LR35902_SPEED: sprintf(info->s, "SPD:%02x", 0x7E | ( ( cpustate->w.gb_speed - 1 ) << 7 ) | cpustate->w.gb_speed_change_pending ); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(LR35902, lr35902);
