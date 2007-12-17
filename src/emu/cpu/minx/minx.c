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
- Add support for CE D8 (MUL) and CE D9 (DIV)
- Add support for CF xx instructions
- Doublecheck behaviour of CMPN instructions ( CF 60 .. CF 63 )
- Figure out the number of cycles taken by each instruction (this
  information is currently not known/available).

*/

#include "minx.h"
#include "debugger.h"

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
	UINT16 PC;
	UINT16 SP;
	UINT16 BA;
	UINT16 HL;
	UINT16 X;
	UINT16 Y;
	UINT8  U;
	UINT8  V;
	UINT8  F;
	UINT8  E;
	UINT8  N;
	UINT8  I;
	UINT8  XI;
	UINT8  YI;
	UINT8  check_interrupt;
} minx_regs;

static minx_regs regs;
int minx_icount;

#define rd(offset)	program_read_byte_8( offset )
#define wr(offset,data)	program_write_byte_8( offset, data )
#define minx_PC		( ( regs.PC & 0x8000 ) ? ( regs.V << 15 ) | (regs.PC & 0x7FFF ) : regs.PC )

INLINE UINT16 rd16( UINT32 offset ) {
	return rd( offset ) | ( rd( offset + 1 ) << 8 );
}

INLINE void wr16( UINT32 offset, UINT16 data ) {
	wr( offset, ( data & 0x00FF ) );
	wr( offset + 1, ( data >> 8 ) );
}

static void minx_init(int index, int clock, const void *config, int (*irqcallback)(int)) {
	if ( config != NULL ) {
	} else {
	}
}

static void minx_reset( void ) {
	memset( &regs, 0, sizeof(regs) );
	regs.PC = ( rd( 1 ) << 8 ) | rd( 0 );
	change_pc( regs.PC );
}

static void minx_exit( void ) {
}

INLINE UINT8 rdop( void ) {
	UINT8 op = rd( minx_PC );
	regs.PC++;
	return op;
}

INLINE UINT16 rdop16( void ) {
	UINT16 op = rdop();
	op = op | ( rdop() << 8 );
	return op;
}

#include "minxfunc.h"
#include "minxopce.h"
#include "minxopcf.h"
#include "minxops.h"

static int minx_execute( int cycles ) {
	UINT32	oldpc;
	UINT8	op;

	minx_icount = cycles;

	do {
		CALL_MAME_DEBUG;
		oldpc = minx_PC;
		op = rdop();
		insnminx[op]();
		minx_icount -= insnminx_cycles[op];
	} while ( minx_icount > 0 );
	return cycles - minx_icount;
}

static void minx_burn( int cycles ) {
	minx_icount = 0;
}

static void minx_set_context( void *src ) {
}

static void minx_get_context( void *dst ) {
}

static unsigned minx_get_reg( int regnum ) {
	switch( regnum ) {
	case REG_PC:	return (regs.PC & 0x8000) ? ( regs.V << 15 ) | ( regs.PC & 0x7FFF ) : regs.PC;
	case MINX_PC:	return regs.PC;
	case REG_SP:
	case MINX_SP:	return regs.SP;
	case MINX_BA:	return regs.BA;
	case MINX_HL:	return regs.HL;
	case MINX_X:	return regs.X;
	case MINX_Y:	return regs.Y;
	case MINX_U:	return regs.U;
	case MINX_V:	return regs.V;
	case MINX_F:	return regs.F;
	case MINX_E:	return regs.E;
	case MINX_N:	return regs.N;
	case MINX_I:	return regs.I;
	case MINX_XI:	return regs.XI;
	case MINX_YI:	return regs.YI;
	}
	return 0;
}

static void minx_set_reg( int regnum, unsigned val ) {
	switch( regnum ) {
	case REG_PC:
	case MINX_PC:	regs.PC = val; break;
	case REG_SP:
	case MINX_SP:	regs.SP = val; break;
	case MINX_BA:	regs.BA = val; break;
	case MINX_HL:	regs.HL = val; break;
	case MINX_X:	regs.X = val; break;
	case MINX_Y:	regs.Y = val; break;
	case MINX_U:	regs.U = val; break;
	case MINX_V:	regs.V = val; break;
	case MINX_F:	regs.F = val; break;
	case MINX_E:	regs.E = val; break;
	case MINX_N:	regs.N = val; break;
	case MINX_I:	regs.I = val; break;
	case MINX_XI:	regs.XI = val; break;
	case MINX_YI:	regs.YI = val; break;
	}
}

static void minx_set_irq_line( int irqline, int state ) {
	if ( state == ASSERT_LINE ) {
	} else {
	}
}

static void minx_set_info( UINT32 state, cpuinfo *info ) {
	switch( state ) {
	case CPUINFO_INT_INPUT_STATE + 0:
		minx_set_irq_line( state - CPUINFO_INT_INPUT_STATE, info->i ); break;

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
		minx_set_reg( state - CPUINFO_INT_REGISTER, info->i ); break;
	}
}

void minx_get_info( UINT32 state, cpuinfo *info ) {
	switch( state ) {
	case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(minx_regs); break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 1; break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff; break;
	case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_BE; break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1; break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1; break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5; break;
	case CPUINFO_INT_MIN_CYCLES:				info->i = 1; break;
	case CPUINFO_INT_MAX_CYCLES:				info->i = 4; break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8; break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 23; break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0; break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0; break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:	info->i = 0; break;
	case CPUINFO_INT_INPUT_STATE + 0:			info->i = 0; break;
	case CPUINFO_INT_REGISTER + REG_PC:
	case CPUINFO_INT_REGISTER + REG_SP:
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
	case CPUINFO_INT_REGISTER + MINX_YI:			info->i = minx_get_reg( state - CPUINFO_INT_REGISTER ); break;
	case CPUINFO_INT_PREVIOUSPC:				info->i = 0x0000; break;
	case CPUINFO_PTR_SET_INFO:				info->setinfo = minx_set_info; break;
	case CPUINFO_PTR_GET_CONTEXT:				info->getcontext = minx_get_context; break;
	case CPUINFO_PTR_SET_CONTEXT:				info->setcontext = minx_set_context; break;
	case CPUINFO_PTR_INIT:					info->init = minx_init; break;
	case CPUINFO_PTR_RESET:					info->reset = minx_reset; break;
	case CPUINFO_PTR_EXIT:					info->exit = minx_exit; break;
	case CPUINFO_PTR_EXECUTE:				info->execute = minx_execute; break;
	case CPUINFO_PTR_BURN:					info->burn = minx_burn; break;
#ifdef MAME_DEBUG
	case CPUINFO_PTR_DISASSEMBLE:				info->disassemble = minx_dasm; break;
#endif
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &minx_icount; break;
	case CPUINFO_STR_NAME:					strcpy( info->s = cpuintrf_temp_str(), "Minx" ); break;
	case CPUINFO_STR_CORE_FAMILY:				strcpy( info->s = cpuintrf_temp_str(), "Nintendo Minx" ); break;
	case CPUINFO_STR_CORE_VERSION:				strcpy( info->s = cpuintrf_temp_str(), "0.1" ); break;
	case CPUINFO_STR_CORE_FILE:				strcpy( info->s = cpuintrf_temp_str(), __FILE__ ); break;
	case CPUINFO_STR_CORE_CREDITS:				strcpy( info->s = cpuintrf_temp_str(), "Copyright (C) 2006 by The MESS Team." ); break;
	case CPUINFO_STR_FLAGS:
		sprintf( info->s = cpuintrf_temp_str(), "%c%c%c%c%c%c%c%c-%c%c%c%c%c",
			regs.F & FLAG_I ? 'I' : '.',
			regs.F & FLAG_D ? 'D' : '.',
			regs.F & FLAG_L ? 'L' : '.',
			regs.F & FLAG_B ? 'B' : '.',
			regs.F & FLAG_S ? 'S' : '.',
			regs.F & FLAG_O ? 'O' : '.',
			regs.F & FLAG_C ? 'C' : '.',
			regs.F & FLAG_Z ? 'Z' : '.',
			regs.E & EXEC_X0 ? '0' : '.',
			regs.E & EXEC_X1 ? '1' : '.',
			regs.E & EXEC_X2 ? '2' : '.',
			regs.E & EXEC_DZ ? 'z' : '.',
			regs.E & EXEC_EN ? 'E' : '.' );
		break;
	case CPUINFO_STR_REGISTER + MINX_PC:			sprintf( info->s = cpuintrf_temp_str(), "PC:%04X", regs.PC ); break;
	case CPUINFO_STR_REGISTER + MINX_SP:			sprintf( info->s = cpuintrf_temp_str(), "SP:%04X", regs.SP ); break;
	case CPUINFO_STR_REGISTER + MINX_BA:			sprintf( info->s = cpuintrf_temp_str(), "BA:%04X", regs.BA ); break;
	case CPUINFO_STR_REGISTER + MINX_HL:			sprintf( info->s = cpuintrf_temp_str(), "HL:%04X", regs.HL ); break;
	case CPUINFO_STR_REGISTER + MINX_X:			sprintf( info->s = cpuintrf_temp_str(), "X:%04X", regs.X ); break;
	case CPUINFO_STR_REGISTER + MINX_Y:			sprintf( info->s = cpuintrf_temp_str(), "Y:%04X", regs.Y ); break;
	case CPUINFO_STR_REGISTER + MINX_U:			sprintf( info->s = cpuintrf_temp_str(), "U:%02X", regs.U ); break;
	case CPUINFO_STR_REGISTER + MINX_V:			sprintf( info->s = cpuintrf_temp_str(), "V:%02X", regs.V ); break;
	case CPUINFO_STR_REGISTER + MINX_F:			sprintf( info->s = cpuintrf_temp_str(), "F:%02X", regs.F ); break;
	case CPUINFO_STR_REGISTER + MINX_E:			sprintf( info->s = cpuintrf_temp_str(), "E:%02X", regs.E ); break;
	case CPUINFO_STR_REGISTER + MINX_N:			sprintf( info->s = cpuintrf_temp_str(), "N:%02X", regs.N ); break;
	case CPUINFO_STR_REGISTER + MINX_I:			sprintf( info->s = cpuintrf_temp_str(), "I:%02X", regs.I ); break;
	case CPUINFO_STR_REGISTER + MINX_XI:			sprintf( info->s = cpuintrf_temp_str(), "XI:%02X", regs.XI ); break;
	case CPUINFO_STR_REGISTER + MINX_YI:			sprintf( info->s = cpuintrf_temp_str(), "YI:%02X", regs.YI ); break;
	}
}




