/*
  Implementation for Sharp sm8500 cpu. There is hardly any information available
  on this cpu. Currently we've only found documentation on the microcontroller
  parts of the cpu, but nothing on the cpu itself.

  Through looking at binary data we have attempted to figure out the opcodes for
  this cpu, and made educated guesses on the number of cycles for each instruction.

  Code by Wilbert Pol
*/

#include "sm8500.h"
#include "debugger.h"

#define FLAG_C	0x80
#define FLAG_Z	0x40
#define FLAG_S	0x20
#define FLAG_V	0x10
#define FLAG_D	0x08
#define FLAG_H	0x04
#define FLAG_B	0x02
#define FLAG_I	0x01

typedef struct {
	SM8500_CONFIG config;
	UINT16 PC;
	UINT8 *register_base;
	UINT8 IE0;
	UINT8 IE1;
	UINT8 IR0;
	UINT8 IR1;
	UINT8 P0;
	UINT8 P1;
	UINT8 P2;
	UINT8 P3;
	UINT8 SYS;
	UINT8 CKC;
	UINT8 clock_changed;
	UINT16 SP;
	UINT8 PS0;
	UINT8 PS1;
	UINT8 P0C;
	UINT8 P1C;
	UINT8 P2C;
	UINT8 P3C;
	UINT8 IFLAGS;
	UINT8 CheckInterrupts;
	int halted;
	int (*irq_callback)(int irqline);
	UINT8 internal_ram[0x500];
} sm8500_regs;

static sm8500_regs regs;

/* nr of cycles to run */
int sm8500_icount;

static UINT8 sm8500_b2w[8] = {
        0, 8, 2, 10, 4, 12, 6, 14
};

UINT8 sm85cpu_mem_readbyte( UINT32 offset ) {
	return ( offset < 0x10 ) ? regs.register_base[offset] : program_read_byte_8( offset );
}

void sm85cpu_mem_writebyte( UINT32 offset, UINT8 data ) {
	if ( offset < 0x10 ) {
		regs.register_base[offset] = data;
	} else {
		program_write_byte_8( offset, data );
	}
}

UINT8* sm8500_internal_ram( void )
{
	return regs.internal_ram;
}

static void sm8500_init(int index, int clock, const void *config, int (*irqcallback)(int)) {
	regs.irq_callback = irqcallback;
	if ( config != NULL ) {
		regs.config.handle_dma = ((SM8500_CONFIG *)config)->handle_dma;
		regs.config.handle_timers = ((SM8500_CONFIG *)config)->handle_timers;
	} else {
		regs.config.handle_dma = NULL;
		regs.config.handle_timers = NULL;
	}
	regs.register_base = regs.internal_ram;
}

static void sm8500_reset( void )
{
	regs.PC = 0x1020;
	regs.IE0 = 0;
	regs.IE1 = 0;
	regs.IR0 = 0;
	regs.IR1 = 0;
	regs.P0 = 0xFF;
	regs.P1 = 0xFF;
	regs.P2 = 0xFF;
	regs.P3 = 0;
	regs.SYS = 0;
	regs.CKC = 0; regs.clock_changed = 0;
	regs.PS1 = 0;
	regs.register_base = regs.internal_ram;
	regs.halted = 0;
}

static void sm8500_exit( void )
{
}

#define PUSH_BYTE(X)	regs.SP = regs.SP - 1; \
			if ( ( regs.SYS & 0x40 ) == 0 ) { \
				regs.SP = regs.SP & 0xFF; \
			} \
			sm85cpu_mem_writebyte( regs.SP, X );

INLINE void sm8500_do_interrupt(UINT16 vector) {
	/* Push PC */
	PUSH_BYTE( regs.PC & 0xFF );
	PUSH_BYTE( regs.PC >> 8 );
	/* Push PS1 */
	PUSH_BYTE( regs.PS1 );
	/* Clear I flag */
	regs.PS1 &= ~ 0x01;
	/* Change PC to address stored at "vector" */
	regs.PC = sm85cpu_mem_readword( vector );
}

INLINE void sm8500_process_interrupts(void) {
	if ( regs.CheckInterrupts ) {
		int irqline = 0;
		while( irqline < 11 ) {
			if ( regs.IFLAGS & ( 1 << irqline ) ) {
				regs.halted = 0;
				switch( irqline ) {
				case ILL_INT:
					sm8500_do_interrupt( 0x101C );
					break;
				case WDT_INT:
					sm8500_do_interrupt( 0x101E );
					break;
				case NMI_INT:
					sm8500_do_interrupt( 0x101E );
					break;
				case DMA_INT:
					regs.IR0 |= 0x80;
					if ( ( regs.IE0 & 0x80 ) && ( ( regs.PS0 & 0x07 ) < 8 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x1000 );
					}
					break;
				case TIM0_INT:
					regs.IR0 |= 0x40;
					if ( ( regs.IE0 & 0x40 ) && ( ( regs.PS0 & 0x07 ) < 8 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x1002 );
					}
					break;
				case EXT_INT:
					regs.IR0 |= 0x10;
					if ( ( regs.IE0 & 0x10 ) && ( ( regs.PS0 & 0x07 ) < 7 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x1006 );
					}
					break;
				case UART_INT:
					regs.IR0 |= 0x08;
					if ( ( regs.IE0 & 0x08 ) && ( ( regs.PS0 & 0x07 ) < 6 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x1008 );
					}
					break;
				case LCDC_INT:
					regs.IR0 |= 0x01;
					if ( ( regs.IE0 & 0x01 ) && ( ( regs.PS0 & 0x07 ) < 5 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x100E );
					}
					break;
				case TIM1_INT:
					regs.IR1 |= 0x40;
					if ( ( regs.IE1 & 0x40 ) && ( ( regs.PS0 & 0x07 ) < 4 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x1012 );
					}
					break;
				case CK_INT:
					regs.IR1 |= 0x10;
					if ( ( regs.IE1 & 0x10 ) && ( ( regs.PS0 & 0x07 ) < 3 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x1016 );
					}
					break;
				case PIO_INT:
					regs.IR1 |= 0x04;
					if ( ( regs.IE1 & 0x04 ) && ( ( regs.PS0 & 0x07 ) < 2 ) && ( regs.PS1 & 0x01 ) ) {
						sm8500_do_interrupt( 0x101A );
					}
					break;
				}
				regs.IFLAGS &= ~ ( 1 << irqline );
			}
			irqline++;
		}
	}
}

static int sm8500_execute( int cycles )
{
	UINT8	op;
	UINT16 oldpc;
	int	mycycles;

	sm8500_icount = cycles;

	do
	{
		UINT8	r1,r2;
		UINT16	s1,s2;
		UINT32	d1,d2;
		UINT32	res;

		CALL_MAME_DEBUG;
		oldpc = regs.PC;
		mycycles = 0;
		sm8500_process_interrupts();
		if ( !regs.halted ) {
			op = sm85cpu_mem_readbyte( regs.PC++ );
			switch( op )
			{
#include "sm85ops.h"
			}
		} else {
			mycycles = 4;
			if ( regs.config.handle_dma ) {
				regs.config.handle_dma( mycycles );
			}
		}
		if ( regs.config.handle_timers ) {
			regs.config.handle_timers( mycycles );
		}
		sm8500_icount -= mycycles;
	} while ( sm8500_icount > 0 );

	return cycles - sm8500_icount;
}

static void sm8500_burn( int cycles )
{
	if ( cycles > 0 ) {
		/* burn a number of 4 cycles */
		int n = ( cycles + 3 ) / 4;
		sm8500_icount -= 4 * n;
	}
}

static void sm8500_set_context( void *src )
{
}

static void sm8500_get_context( void *dst )
{
}

unsigned sm8500_get_reg( int regnum )
{
	switch( regnum )
	{
	case REG_PC:
	case SM8500_PC:		return regs.PC;
	case REG_SP:
	case SM8500_SP:		return ( regs.SYS & 0x40 ) ? regs.SP : regs.SP & 0xFF ;
	case SM8500_PS:		return ( regs.PS0 << 8 ) | regs.PS1;
	case SM8500_SYS16:	return regs.SYS;
	case SM8500_RR0:	return sm85cpu_mem_readword( 0x00 );
	case SM8500_RR2:	return sm85cpu_mem_readword( 0x02 );
	case SM8500_RR4:	return sm85cpu_mem_readword( 0x04 );
	case SM8500_RR6:	return sm85cpu_mem_readword( 0x06 );
	case SM8500_RR8:	return sm85cpu_mem_readword( 0x08 );
	case SM8500_RR10:	return sm85cpu_mem_readword( 0x0A );
	case SM8500_RR12:	return sm85cpu_mem_readword( 0x0C );
	case SM8500_RR14:	return sm85cpu_mem_readword( 0x0E );
	case SM8500_IE0:	return regs.IE0;
	case SM8500_IE1:	return regs.IE1;
	case SM8500_IR0:	return regs.IR0;
	case SM8500_IR1:	return regs.IR1;
	case SM8500_P0:		return regs.P0;
	case SM8500_P1:		return regs.P1;
	case SM8500_P2:		return regs.P2;
	case SM8500_P3:		return regs.P3;
	case SM8500_SYS:	return regs.SYS;
	case SM8500_CKC:	return regs.CKC;
	case SM8500_SPH:	return (regs.SP >> 8);
	case SM8500_SPL:	return regs.SP & 0xFF;
	case SM8500_PS0:	return regs.PS0;
	case SM8500_PS1:	return regs.PS1;
	case SM8500_P0C:	return regs.P0C;
	case SM8500_P1C:	return regs.P1C;
	case SM8500_P2C:	return regs.P2C;
	case SM8500_P3C:	return regs.P3C;
	}
	return 0;
}

static void sm8500_set_reg( int regnum, unsigned val )
{
	switch( regnum )
	{
	case REG_PC:
	case SM8500_PC:		regs.PC = val; break;
	case REG_SP:
	case SM8500_SP:		regs.SP = val; break;
	case SM8500_PS:		sm8500_set_reg( SM8500_PS0, ( val >> 8 ) & 0xFF ); sm8500_set_reg( SM8500_PS1, val & 0xFF ); break;
	case SM8500_SYS16:	regs.SYS = val; break;
	case SM8500_RR0:	sm85cpu_mem_writeword( 0x00, val); break;
	case SM8500_RR2:	sm85cpu_mem_writeword( 0x02, val); break;
	case SM8500_RR4:	sm85cpu_mem_writeword( 0x04, val); break;
	case SM8500_RR6:	sm85cpu_mem_writeword( 0x06, val); break;
	case SM8500_RR8:	sm85cpu_mem_writeword( 0x08, val); break;
	case SM8500_RR10:	sm85cpu_mem_writeword( 0x0A, val); break;
	case SM8500_RR12:	sm85cpu_mem_writeword( 0x0C, val); break;
	case SM8500_RR14:	sm85cpu_mem_writeword( 0x0E, val); break;
	case SM8500_IE0:	regs.IE0 = val; break;
	case SM8500_IE1:	regs.IE1 = val; break;
	case SM8500_IR0:	regs.IR0 = val; break;
	case SM8500_IR1:	regs.IR1 = val; break;
	case SM8500_P0:		regs.P0 = val; break;
	case SM8500_P1:		regs.P1 = val; break;
	case SM8500_P2:		regs.P2 = val; break;
	case SM8500_P3:		regs.P3 = val; break;
	case SM8500_SYS:	regs.SYS = val; break;
	case SM8500_CKC:	regs.CKC = val; if ( val & 0x80 ) { regs.clock_changed = 1; }; break;
	case SM8500_SPH:	regs.SP = ( ( val & 0xFF ) << 8 ) | ( regs.SP & 0xFF ); break;
	case SM8500_SPL:	regs.SP = ( regs.SP & 0xFF00 ) | ( val & 0xFF ); break;
	case SM8500_PS0:	regs.PS0 = val; regs.register_base = regs.internal_ram + ( val & 0xF8 ); break;
	case SM8500_PS1:	regs.PS1 = val; break;
	case SM8500_P0C:	regs.P0C = val; break;
	case SM8500_P1C:	regs.P1C = val; break;
	case SM8500_P2C:	regs.P2C = val; break;
	case SM8500_P3C:	regs.P3C = val; break;
	}
}

static void sm8500_set_irq_line( int irqline, int state )
{
	if ( state == ASSERT_LINE ) {
		regs.IFLAGS |= ( 0x01 << irqline );
		regs.CheckInterrupts = 1;
		switch( irqline ) {
		case DMA_INT:	regs.IR0 |= 0x80; break;
		case TIM0_INT:	regs.IR0 |= 0x40; break;
		case EXT_INT:	regs.IR0 |= 0x10; break;
		case UART_INT:	regs.IR0 |= 0x08; break;
		case LCDC_INT:	regs.IR0 |= 0x01; break;
		case TIM1_INT:	regs.IR1 |= 0x40; break;
		case CK_INT:	regs.IR1 |= 0x10; break;
		case PIO_INT:	regs.IR1 |= 0x04; break;
		}
	} else {
		regs.IFLAGS &= ~( 0x01 << irqline );
		switch( irqline ) {
		case DMA_INT:	regs.IR0 &= ~0x80; break;
		case TIM0_INT:	regs.IR0 &= ~0x40; break;
		case EXT_INT:	regs.IR0 &= ~0x10; break;
		case UART_INT:	regs.IR0 &= ~0x08; break;
		case LCDC_INT:	regs.IR0 &= ~0x01; break;
		case TIM1_INT:	regs.IR1 &= ~0x40; break;
		case CK_INT:	regs.IR1 &= ~0x10; break;
		case PIO_INT:	regs.IR1 &= ~0x04; break;
		}
		if ( 0 == regs.IFLAGS ) {
			regs.CheckInterrupts = 0;
		}
	}
}

static void sm8500_set_info( UINT32 state, cpuinfo *info )
{
	switch(state)
	{
	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:
	case CPUINFO_INT_INPUT_STATE + 5:
	case CPUINFO_INT_INPUT_STATE + 6:
	case CPUINFO_INT_INPUT_STATE + 7:
	case CPUINFO_INT_INPUT_STATE + 8:
	case CPUINFO_INT_INPUT_STATE + 9:
	case CPUINFO_INT_INPUT_STATE + 10:
		sm8500_set_irq_line( state - CPUINFO_INT_INPUT_STATE, info->i ); break;

	case CPUINFO_INT_REGISTER + SM8500_RR0:
	case CPUINFO_INT_REGISTER + SM8500_RR2:
	case CPUINFO_INT_REGISTER + SM8500_RR4:
	case CPUINFO_INT_REGISTER + SM8500_RR6:
	case CPUINFO_INT_REGISTER + SM8500_RR8:
	case CPUINFO_INT_REGISTER + SM8500_RR10:
	case CPUINFO_INT_REGISTER + SM8500_RR12:
	case CPUINFO_INT_REGISTER + SM8500_RR14:
	case CPUINFO_INT_REGISTER + SM8500_PC:
	case CPUINFO_INT_REGISTER + SM8500_SP:
	case CPUINFO_INT_REGISTER + SM8500_PS:
	case CPUINFO_INT_REGISTER + SM8500_SYS16:
	case CPUINFO_INT_REGISTER + SM8500_SYS:
	case CPUINFO_INT_REGISTER + SM8500_IE0:
	case CPUINFO_INT_REGISTER + SM8500_IE1:
	case CPUINFO_INT_REGISTER + SM8500_IR0:
	case CPUINFO_INT_REGISTER + SM8500_IR1:
	case CPUINFO_INT_REGISTER + SM8500_P0:
	case CPUINFO_INT_REGISTER + SM8500_P1:
	case CPUINFO_INT_REGISTER + SM8500_P2:
	case CPUINFO_INT_REGISTER + SM8500_P3:
	case CPUINFO_INT_REGISTER + SM8500_CKC:
	case CPUINFO_INT_REGISTER + SM8500_SPH:
	case CPUINFO_INT_REGISTER + SM8500_SPL:
	case CPUINFO_INT_REGISTER + SM8500_PS0:
	case CPUINFO_INT_REGISTER + SM8500_PS1:
	case CPUINFO_INT_REGISTER + SM8500_P0C:
	case CPUINFO_INT_REGISTER + SM8500_P1C:
	case CPUINFO_INT_REGISTER + SM8500_P2C:
	case CPUINFO_INT_REGISTER + SM8500_P3C:
							sm8500_set_reg( state - CPUINFO_INT_REGISTER, info->i ); break;

	}
}

void sm8500_get_info( UINT32 state, cpuinfo *info )
{
	switch(state)
	{
	case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(sm8500_regs); break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 8; break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff; break;
	case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_BE; break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1; break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1; break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5; break;
	case CPUINFO_INT_MIN_CYCLES:				info->i = 1; break;
	case CPUINFO_INT_MAX_CYCLES:				info->i = 16; break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8; break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16; break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0; break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0; break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:	info->i = 0; break;
	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:
	case CPUINFO_INT_INPUT_STATE + 5:
	case CPUINFO_INT_INPUT_STATE + 6:
	case CPUINFO_INT_INPUT_STATE + 7:			info->i = regs.IFLAGS & ( 1 << (state - CPUINFO_INT_INPUT_STATE)); break;
	case CPUINFO_INT_REGISTER + SM8500_RR0:
	case CPUINFO_INT_REGISTER + SM8500_RR2:
	case CPUINFO_INT_REGISTER + SM8500_RR4:
	case CPUINFO_INT_REGISTER + SM8500_RR6:
	case CPUINFO_INT_REGISTER + SM8500_RR8:
	case CPUINFO_INT_REGISTER + SM8500_RR10:
	case CPUINFO_INT_REGISTER + SM8500_RR12:
	case CPUINFO_INT_REGISTER + SM8500_RR14:
	case CPUINFO_INT_REGISTER + SM8500_PC:
	case CPUINFO_INT_REGISTER + SM8500_SP:
	case CPUINFO_INT_REGISTER + SM8500_PS:
	case CPUINFO_INT_REGISTER + SM8500_SYS16:
	case CPUINFO_INT_REGISTER + SM8500_SYS:
	case CPUINFO_INT_REGISTER + SM8500_IE0:
	case CPUINFO_INT_REGISTER + SM8500_IE1:
	case CPUINFO_INT_REGISTER + SM8500_IR0:
	case CPUINFO_INT_REGISTER + SM8500_IR1:
	case CPUINFO_INT_REGISTER + SM8500_P0:
	case CPUINFO_INT_REGISTER + SM8500_P1:
	case CPUINFO_INT_REGISTER + SM8500_P2:
	case CPUINFO_INT_REGISTER + SM8500_P3:
	case CPUINFO_INT_REGISTER + SM8500_CKC:
	case CPUINFO_INT_REGISTER + SM8500_SPH:
	case CPUINFO_INT_REGISTER + SM8500_SPL:
	case CPUINFO_INT_REGISTER + SM8500_PS0:
	case CPUINFO_INT_REGISTER + SM8500_PS1:
	case CPUINFO_INT_REGISTER + SM8500_P0C:
	case CPUINFO_INT_REGISTER + SM8500_P1C:
	case CPUINFO_INT_REGISTER + SM8500_P2C:
	case CPUINFO_INT_REGISTER + SM8500_P3C:
								info->i = sm8500_get_reg( state - CPUINFO_INT_REGISTER ); break;
	case CPUINFO_INT_REGISTER + REG_PC:			info->i = sm8500_get_reg( SM8500_PC ); break;
	case CPUINFO_INT_REGISTER + REG_SP:			info->i = sm8500_get_reg( SM8500_SP ); break;
	case CPUINFO_INT_PREVIOUSPC:				info->i = 0x0000; break;


	case CPUINFO_PTR_SET_INFO:				info->setinfo = sm8500_set_info; break;
	case CPUINFO_PTR_GET_CONTEXT:				info->getcontext = sm8500_get_context; break;
	case CPUINFO_PTR_SET_CONTEXT:				info->setcontext = sm8500_set_context; break;
	case CPUINFO_PTR_INIT:					info->init = sm8500_init; break;
	case CPUINFO_PTR_RESET:					info->reset = sm8500_reset; break;
	case CPUINFO_PTR_EXIT:					info->exit = sm8500_exit; break;
	case CPUINFO_PTR_EXECUTE:				info->execute = sm8500_execute; break;
	case CPUINFO_PTR_BURN:					info->burn = sm8500_burn; break;
#ifdef MAME_DEBUG
	case CPUINFO_PTR_DISASSEMBLE:			info->disassemble = sm8500_dasm; break;
#endif
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &sm8500_icount; break;

	case CPUINFO_STR_NAME:					strcpy( info->s = cpuintrf_temp_str(), "sm8500" ); break;
	case CPUINFO_STR_CORE_FAMILY:				strcpy( info->s = cpuintrf_temp_str(), "Sharp SM8500" ); break;
	case CPUINFO_STR_CORE_VERSION:				strcpy( info->s = cpuintrf_temp_str(), "0.1" ); break;
	case CPUINFO_STR_CORE_FILE:				strcpy( info->s = cpuintrf_temp_str(), __FILE__ ); break;
	case CPUINFO_STR_CORE_CREDITS:				strcpy( info->s = cpuintrf_temp_str(), "Copyright (C) 2005 by The MESS Team." ); break;
	case CPUINFO_STR_FLAGS:
		sprintf( info->s = cpuintrf_temp_str(), "%c%c%c%c%c%c%c%c",
			regs.PS1 & FLAG_C ? 'C' : '.',
			regs.PS1 & FLAG_Z ? 'Z' : '.',
			regs.PS1 & FLAG_S ? 'S' : '.',
			regs.PS1 & FLAG_V ? 'V' : '.',
			regs.PS1 & FLAG_D ? 'D' : '.',
			regs.PS1 & FLAG_H ? 'H' : '.',
			regs.PS1 & FLAG_B ? 'B' : '.',
			regs.PS1 & FLAG_I ? 'I' : '.' );
		break;
	case CPUINFO_STR_REGISTER + SM8500_RR0:			sprintf(info->s = cpuintrf_temp_str(), "RR0:%04X", sm85cpu_mem_readword( 0x00 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR2:			sprintf(info->s = cpuintrf_temp_str(), "RR2:%04X", sm85cpu_mem_readword( 0x02 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR4:			sprintf(info->s = cpuintrf_temp_str(), "RR4:%04X", sm85cpu_mem_readword( 0x04 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR6:			sprintf(info->s = cpuintrf_temp_str(), "RR6:%04X", sm85cpu_mem_readword( 0x06 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR8:			sprintf(info->s = cpuintrf_temp_str(), "RR8:%04X", sm85cpu_mem_readword( 0x08 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR10:		sprintf(info->s = cpuintrf_temp_str(), "RR10:%04X", sm85cpu_mem_readword( 0x0A ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR12:		sprintf(info->s = cpuintrf_temp_str(), "RR12:%04X", sm85cpu_mem_readword( 0x0C ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR14:		sprintf(info->s = cpuintrf_temp_str(), "RR14:%04X", sm85cpu_mem_readword( 0x0E ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_PC:			sprintf(info->s = cpuintrf_temp_str(), "PC:%04X", regs.PC); break;
	case CPUINFO_STR_REGISTER + SM8500_SP:			sprintf(info->s = cpuintrf_temp_str(), "SP:%04X", regs.SP); break;
	case CPUINFO_STR_REGISTER + SM8500_PS:			sprintf(info->s = cpuintrf_temp_str(), "PS:%04X", ( regs.PS0 << 8 ) | regs.PS1 ); break;
	case CPUINFO_STR_REGISTER + SM8500_SYS16:		sprintf(info->s = cpuintrf_temp_str(), "SYS:%04X", regs.SYS ); break;
	}
}

