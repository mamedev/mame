/*
  Implementation for the Nintendo Minx CPU.

  Registers (mindX13.txt):
  8bit:   A B H L FLAGS N U V
  16bit:  BA
  24bit:      HL, X1, X2, NN, SP


  "sunlab":
  16bit:
    V:PC if high bit set, extended to 23 bits, upper 8 bits V
    SP
    BA
    I:HL
    (XI:)X
    (YI:)Y

  8bit:
    A, B, H, L
    U - delayed jump bank. When a jump occurs, V is set to this value
    V
    F - flags IDLBSOCZ
        I - Interrupt branch
        D - Interrupt disable
        L - low mask mode enable
        B - bcd decimal mode enable
        S - sign flag
        O - overflow flag
        C - carry flag
        Z - zero flag
    E - exception register
    I
    N
    XI - index/extension
    YI - index/extension

TODO:
- Add support for O and C flags in NEG8 instruction
- Verify MUL (CE D8) and DIV (CE D9)
- Doublecheck behaviour of CMPN instructions ( CF 60 .. CF 63 )

*/

#include "emu.h"
#include "debugger.h"
#include "minx.h"

#define FLAG_I  0x80
#define FLAG_D  0x40
#define FLAG_L  0x20
#define FLAG_B  0x10
#define FLAG_S  0x08
#define FLAG_O  0x04
#define FLAG_C  0x02
#define FLAG_Z  0x01

#define EXEC_X0 0x80
#define EXEC_X1 0x40
#define EXEC_X2 0x20
#define EXEC_DZ 0x10
#define EXEC_EN 0x08
#define EXEC_04 0x04
#define EXEC_02 0x02
#define EXEC_01 0x01


typedef struct {
//  MINX_CONFIG  config;
	UINT16	PC;
	UINT16	SP;
	UINT16	BA;
	UINT16	HL;
	UINT16	X;
	UINT16	Y;
	UINT8	U;
	UINT8	V;
	UINT8	F;
	UINT8	E;
	UINT8	N;
	UINT8	I;
	UINT8	XI;
	UINT8	YI;
	UINT8	halted;
	UINT8	interrupt_pending;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	int icount;
} minx_state;

#define RD(offset)		minx->program->read_byte( offset )
#define WR(offset,data)	minx->program->write_byte( offset, data )
#define GET_MINX_PC		( ( minx->PC & 0x8000 ) ? ( minx->V << 15 ) | (minx->PC & 0x7FFF ) : minx->PC )

INLINE minx_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MINX);

	return (minx_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE UINT16 rd16( minx_state *minx, UINT32 offset )
{
	return RD( offset ) | ( RD( offset + 1 ) << 8 );
}


INLINE void wr16( minx_state *minx, UINT32 offset, UINT16 data )
{
	WR( offset, ( data & 0x00FF ) );
	WR( offset + 1, ( data >> 8 ) );
}


static CPU_INIT( minx )
{
	minx_state *minx = get_safe_token(device);
	minx->irq_callback = irqcallback;
	minx->device = device;
	minx->program = device->space(AS_PROGRAM);
	if ( device->static_config() != NULL )
	{
	}
	else
	{
	}
}


static CPU_RESET( minx )
{
	minx_state *minx = get_safe_token(device);
	minx->SP = minx->BA = minx->HL = minx->X = minx->Y = 0;
	minx->U = minx->V = minx->F = minx->E = minx->I = minx->XI = minx->YI = 0;
	minx->halted = minx->interrupt_pending = 0;

	minx->PC = rd16( minx, 0 );
}


static CPU_EXIT( minx )
{
}


INLINE UINT8 rdop( minx_state *minx )
{
	UINT8 op = RD( GET_MINX_PC );
	minx->PC++;
	return op;
}


INLINE UINT16 rdop16( minx_state *minx )
{
	UINT16 op = rdop(minx);
	op = op | ( rdop(minx) << 8 );
	return op;
}


#include "minxfunc.h"
#include "minxopce.h"
#include "minxopcf.h"
#include "minxops.h"


static CPU_EXECUTE( minx )
{
//  UINT32  oldpc;
	UINT8	op;
	minx_state *minx = get_safe_token(device);

	do
	{
		debugger_instruction_hook(device, GET_MINX_PC);
//      oldpc = GET_MINX_PC;

		if ( minx->interrupt_pending )
		{
			minx->halted = 0;
			if ( ! ( minx->F & 0xc0 ) && minx->U == minx->V )
			{
				//logerror("minx_execute(): taking IRQ\n");
				PUSH8( minx, minx->V );
				PUSH16( minx, minx->PC );
				PUSH8( minx, minx->F );

				/* Set Interrupt Branch flag */
				minx->F |= 0x80;
				minx->V = 0;
				minx->PC = rd16( minx, minx->irq_callback( minx->device, 0 ) << 1 );
				minx->icount -= 28;		/* This cycle count is a guess */
			}
		}

		if ( minx->halted )
		{
			minx->icount -= insnminx_cycles_CE[0xAE];
		}
		else
		{
			op = rdop(minx);
			insnminx[op](minx);
			minx->icount -= insnminx_cycles[op];
		}
	} while ( minx->icount > 0 );
}


static CPU_BURN( minx )
{
	minx_state *minx = get_safe_token(device);
	minx->icount = 0;
}


static unsigned minx_get_reg( minx_state *minx, int regnum )
{
	switch( regnum )
	{
	case STATE_GENPC:	return GET_MINX_PC;
	case MINX_PC:	return minx->PC;
	case STATE_GENSP:
	case MINX_SP:	return minx->SP;
	case MINX_BA:	return minx->BA;
	case MINX_HL:	return minx->HL;
	case MINX_X:	return minx->X;
	case MINX_Y:	return minx->Y;
	case MINX_U:	return minx->U;
	case MINX_V:	return minx->V;
	case MINX_F:	return minx->F;
	case MINX_E:	return minx->E;
	case MINX_N:	return minx->N;
	case MINX_I:	return minx->I;
	case MINX_XI:	return minx->XI;
	case MINX_YI:	return minx->YI;
	}
	return 0;
}


static void minx_set_reg( minx_state *minx, int regnum, unsigned val )
{
	switch( regnum )
	{
	case STATE_GENPC:	break;
	case MINX_PC:	minx->PC = val; break;
	case STATE_GENSP:
	case MINX_SP:	minx->SP = val; break;
	case MINX_BA:	minx->BA = val; break;
	case MINX_HL:	minx->HL = val; break;
	case MINX_X:	minx->X = val; break;
	case MINX_Y:	minx->Y = val; break;
	case MINX_U:	minx->U = val; break;
	case MINX_V:	minx->V = val; break;
	case MINX_F:	minx->F = val; break;
	case MINX_E:	minx->E = val; break;
	case MINX_N:	minx->N = val; break;
	case MINX_I:	minx->I = val; break;
	case MINX_XI:	minx->XI = val; break;
	case MINX_YI:	minx->YI = val; break;
	}
}


static void minx_set_irq_line( minx_state *minx, int irqline, int state )
{
	if ( state == ASSERT_LINE )
	{
		minx->interrupt_pending = 1;
	}
	else
	{
		minx->interrupt_pending = 0;
	}
}


static CPU_SET_INFO( minx )
{
	minx_state *minx = get_safe_token(device);
	switch( state )
	{
	case CPUINFO_INT_INPUT_STATE + 0:
		minx_set_irq_line( minx, state - CPUINFO_INT_INPUT_STATE, info->i ); break;

	case CPUINFO_INT_REGISTER + MINX_PC:
	case CPUINFO_INT_REGISTER + MINX_SP:
	case CPUINFO_INT_REGISTER + MINX_BA:
	case CPUINFO_INT_REGISTER + MINX_HL:
	case CPUINFO_INT_REGISTER + MINX_X:
	case CPUINFO_INT_REGISTER + MINX_Y:
	case CPUINFO_INT_REGISTER + MINX_U:
	case CPUINFO_INT_REGISTER + MINX_V:
	case CPUINFO_INT_REGISTER + MINX_F:
	case CPUINFO_INT_REGISTER + MINX_E:
	case CPUINFO_INT_REGISTER + MINX_N:
	case CPUINFO_INT_REGISTER + MINX_I:
	case CPUINFO_INT_REGISTER + MINX_XI:
	case CPUINFO_INT_REGISTER + MINX_YI:
		minx_set_reg( minx, state - CPUINFO_INT_REGISTER, info->i ); break;
	}
}


CPU_GET_INFO( minx )
{
	minx_state *minx = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch( state )
	{
	case CPUINFO_INT_CONTEXT_SIZE:								info->i = sizeof(minx_state); break;
	case CPUINFO_INT_INPUT_LINES:								info->i = 1; break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:						info->i = 0x00; break;
	case DEVINFO_INT_ENDIANNESS:								info->i = ENDIANNESS_BIG; break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:							info->i = 1; break;
	case CPUINFO_INT_CLOCK_DIVIDER:								info->i = 1; break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:						info->i = 1; break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:						info->i = 5; break;
	case CPUINFO_INT_MIN_CYCLES:								info->i = 1; break;
	case CPUINFO_INT_MAX_CYCLES:								info->i = 4; break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:		info->i = 8; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 24; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0; break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:		info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:		info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:		info->i = 0; break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_IO:			info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:			info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:			info->i = 0; break;
	case CPUINFO_INT_INPUT_STATE + 0:							info->i = 0; break;
	case CPUINFO_INT_REGISTER + STATE_GENPC:							info->i = GET_MINX_PC; break;
	case CPUINFO_INT_REGISTER + STATE_GENSP:
	case CPUINFO_INT_REGISTER + MINX_PC:
	case CPUINFO_INT_REGISTER + MINX_SP:
	case CPUINFO_INT_REGISTER + MINX_BA:
	case CPUINFO_INT_REGISTER + MINX_HL:
	case CPUINFO_INT_REGISTER + MINX_X:
	case CPUINFO_INT_REGISTER + MINX_Y:
	case CPUINFO_INT_REGISTER + MINX_U:
	case CPUINFO_INT_REGISTER + MINX_V:
	case CPUINFO_INT_REGISTER + MINX_F:
	case CPUINFO_INT_REGISTER + MINX_E:
	case CPUINFO_INT_REGISTER + MINX_N:
	case CPUINFO_INT_REGISTER + MINX_I:
	case CPUINFO_INT_REGISTER + MINX_XI:
	case CPUINFO_INT_REGISTER + MINX_YI:						info->i = minx_get_reg( minx, state - CPUINFO_INT_REGISTER ); break;
	case CPUINFO_INT_PREVIOUSPC:								info->i = 0x0000; break;
	case CPUINFO_FCT_SET_INFO:									info->setinfo = CPU_SET_INFO_NAME(minx); break;
	case CPUINFO_FCT_INIT:										info->init = CPU_INIT_NAME(minx); break;
	case CPUINFO_FCT_RESET:										info->reset = CPU_RESET_NAME(minx); break;
	case CPUINFO_FCT_EXIT:										info->exit = CPU_EXIT_NAME(minx); break;
	case CPUINFO_FCT_EXECUTE:									info->execute = CPU_EXECUTE_NAME(minx); break;
	case CPUINFO_FCT_BURN:										info->burn = CPU_BURN_NAME(minx); break;
	case CPUINFO_FCT_DISASSEMBLE:								info->disassemble = CPU_DISASSEMBLE_NAME(minx); break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:						info->icount = &minx->icount; break;
	case DEVINFO_STR_NAME:										strcpy( info->s, "Minx" ); break;
	case DEVINFO_STR_FAMILY:								strcpy( info->s, "Nintendo Minx" ); break;
	case DEVINFO_STR_VERSION:								strcpy( info->s, "0.1" ); break;
	case DEVINFO_STR_SOURCE_FILE:									strcpy( info->s, __FILE__ ); break;
	case DEVINFO_STR_CREDITS:								strcpy( info->s, "Copyright The MESS Team." ); break;
	case CPUINFO_STR_FLAGS:
		sprintf( info->s, "%c%c%c%c%c%c%c%c-%c%c%c%c%c",
			minx->F & FLAG_I ? 'I' : '.',
			minx->F & FLAG_D ? 'D' : '.',
			minx->F & FLAG_L ? 'L' : '.',
			minx->F & FLAG_B ? 'B' : '.',
			minx->F & FLAG_S ? 'S' : '.',
			minx->F & FLAG_O ? 'O' : '.',
			minx->F & FLAG_C ? 'C' : '.',
			minx->F & FLAG_Z ? 'Z' : '.',
			minx->E & EXEC_X0 ? '0' : '.',
			minx->E & EXEC_X1 ? '1' : '.',
			minx->E & EXEC_X2 ? '2' : '.',
			minx->E & EXEC_DZ ? 'z' : '.',
			minx->E & EXEC_EN ? 'E' : '.' );
		break;
	case CPUINFO_STR_REGISTER + MINX_PC:						sprintf( info->s, "PC:%04X", minx->PC ); break;
	case CPUINFO_STR_REGISTER + MINX_SP:						sprintf( info->s, "SP:%04X", minx->SP ); break;
	case CPUINFO_STR_REGISTER + MINX_BA:						sprintf( info->s, "BA:%04X", minx->BA ); break;
	case CPUINFO_STR_REGISTER + MINX_HL:						sprintf( info->s, "HL:%04X", minx->HL ); break;
	case CPUINFO_STR_REGISTER + MINX_X:							sprintf( info->s, "X:%04X", minx->X ); break;
	case CPUINFO_STR_REGISTER + MINX_Y:							sprintf( info->s, "Y:%04X", minx->Y ); break;
	case CPUINFO_STR_REGISTER + MINX_U:							sprintf( info->s, "U:%02X", minx->U ); break;
	case CPUINFO_STR_REGISTER + MINX_V:							sprintf( info->s, "V:%02X", minx->V ); break;
	case CPUINFO_STR_REGISTER + MINX_F:							sprintf( info->s, "F:%02X", minx->F ); break;
	case CPUINFO_STR_REGISTER + MINX_E:							sprintf( info->s, "E:%02X", minx->E ); break;
	case CPUINFO_STR_REGISTER + MINX_N:							sprintf( info->s, "N:%02X", minx->N ); break;
	case CPUINFO_STR_REGISTER + MINX_I:							sprintf( info->s, "I:%02X", minx->I ); break;
	case CPUINFO_STR_REGISTER + MINX_XI:						sprintf( info->s, "XI:%02X", minx->XI ); break;
	case CPUINFO_STR_REGISTER + MINX_YI:						sprintf( info->s, "YI:%02X", minx->YI ); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(MINX, minx);
