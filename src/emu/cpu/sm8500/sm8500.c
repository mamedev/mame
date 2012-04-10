/*
  Implementation for Sharp sm8500 cpu. There is hardly any information available
  on this cpu. Currently we've only found documentation on the microcontroller
  parts of the cpu, but nothing on the cpu itself.

  Through looking at binary data we have attempted to figure out the opcodes for
  this cpu, and made educated guesses on the number of cycles for each instruction.

  Code by Wilbert Pol


There is some internal ram for the main cpu registers. They are offset by an index value.
The address is (PS0 & 0xF8) + register number. It is not known what happens when PS0 >= F8.
The assumption is that F8 to 107 is used, but it might wrap around instead.
The registers also mirror out to main RAM, appearing at 0000 to 000F regardless of where
they are internally.

*/

#include "emu.h"
#include "debugger.h"
#include "sm8500.h"

#define FLAG_C	0x80
#define FLAG_Z	0x40
#define FLAG_S	0x20
#define FLAG_V	0x10
#define FLAG_D	0x08
#define FLAG_H	0x04
#define FLAG_B	0x02
#define FLAG_I	0x01

typedef struct _sm8500_state sm8500_state;
struct _sm8500_state
{
	SM8500_CONFIG config;
	UINT16 PC;
	UINT8 IE0;
	UINT8 IE1;
	UINT8 IR0;
	UINT8 IR1;
	UINT8 SYS;
	UINT8 CKC;
	UINT8 clock_changed;
	UINT16 SP;
	UINT8 PS0;
	UINT8 PS1;
	UINT16 IFLAGS;
	UINT8 CheckInterrupts;
	int halted;
	int icount;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	UINT16 oldpc;
	UINT8 register_ram[0x108];
};

INLINE sm8500_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SM8500);
	return (sm8500_state *)downcast<legacy_cpu_device *>(device)->token();
}

static const UINT8 sm8500_b2w[8] = {
        0, 8, 2, 10, 4, 12, 6, 14
};

INLINE void sm8500_get_sp( sm8500_state *cpustate )
{
	UINT16 data = cpustate->program->read_byte(0x1c) << 8;
	cpustate->SP = cpustate->program->read_byte(0x1d);
	if (cpustate->SYS&0x40) cpustate->SP |= data;
}

static UINT8 sm85cpu_mem_readbyte( sm8500_state *cpustate, UINT32 offset )
{
	offset &= 0xffff;
	return (offset < 0x10) ? cpustate->register_ram[offset + (cpustate->PS0 & 0xF8)]
		: cpustate->program->read_byte( offset );
}

static void sm85cpu_mem_writebyte( sm8500_state *cpustate, UINT32 offset, UINT8 data )
{
	UINT8 i;
	offset &= 0xffff;
	if (offset < 0x10)
		cpustate->register_ram[offset + (cpustate->PS0 & 0xF8)] = data;

	cpustate->program->write_byte ( offset, data );

	switch (offset)
	{
		case 0x10: cpustate->IE0 = data; break;
		case 0x11: cpustate->IE1 = data; break;
		case 0x12: cpustate->IR0 = data; break;
		case 0x13: cpustate->IR1 = data; break;
		case 0x19: cpustate->SYS = data; break;
		case 0x1a: cpustate->CKC = data; break;
		case 0x1c:
		case 0x1d: sm8500_get_sp(cpustate); break;
		case 0x1e: cpustate->PS0 = data;
				for (i = 0; i < 16; i++)	// refresh register contents in debugger
					cpustate->program->write_byte(i, sm85cpu_mem_readbyte(cpustate, i)); break;
		case 0x1f: cpustate->PS1 = data; break;
	}
}


INLINE UINT16 sm85cpu_mem_readword( sm8500_state *cpustate, UINT32 address )
{
	return (sm85cpu_mem_readbyte( cpustate, address ) << 8) | (sm85cpu_mem_readbyte( cpustate, address+1 ));
}

INLINE void sm85cpu_mem_writeword( sm8500_state *cpustate, UINT32 address, UINT16 value )
{
	sm85cpu_mem_writebyte( cpustate, address, value >> 8 );
	sm85cpu_mem_writebyte( cpustate, address+1, value );
}

static CPU_INIT( sm8500 )
{
	sm8500_state *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	if ( device->static_config() != NULL ) {
		cpustate->config.handle_dma = ((SM8500_CONFIG *)device->static_config())->handle_dma;
		cpustate->config.handle_timers = ((SM8500_CONFIG *)device->static_config())->handle_timers;
	} else {
		cpustate->config.handle_dma = NULL;
		cpustate->config.handle_timers = NULL;
	}
}

static CPU_RESET( sm8500 )
{
	sm8500_state *cpustate = get_safe_token(device);

	cpustate->PC = 0x1020;
	cpustate->clock_changed = 0;
	cpustate->halted = 0;
	sm85cpu_mem_writeword(cpustate, 0x10, 0);                 // IE0, IE1
	sm85cpu_mem_writeword(cpustate, 0x12, 0);                 // IR0, IR1
	sm85cpu_mem_writeword(cpustate, 0x14, 0xffff);            // P0, P1
	sm85cpu_mem_writeword(cpustate, 0x16, 0xff00);            // P2, P3
	sm85cpu_mem_writebyte(cpustate, 0x19, 0);                 // SYS
	sm85cpu_mem_writebyte(cpustate, 0x1a, 0);                 // CKC
	sm85cpu_mem_writebyte(cpustate, 0x1f, 0);                 // PS1
	sm85cpu_mem_writebyte(cpustate, 0x2b, 0xff);              // URTT
	sm85cpu_mem_writebyte(cpustate, 0x2d, 0x42);              // URTS
	sm85cpu_mem_writebyte(cpustate, 0x5f, 0x38);              // WDTC
}

static CPU_EXIT( sm8500 )
{
}

#define PUSH_BYTE(X)	cpustate->SP--; \
			if ( ( cpustate->SYS & 0x40 ) == 0 ) cpustate->SP &= 0xFF; \
			sm85cpu_mem_writebyte( cpustate, cpustate->SP, X );

INLINE void sm8500_do_interrupt(sm8500_state *cpustate, UINT16 vector) {
	/* Get regs from ram */
	sm8500_get_sp(cpustate);
	cpustate->SYS = cpustate->program->read_byte(0x19);
	cpustate->PS1 = cpustate->program->read_byte(0x1f);
	/* Push PC */
	PUSH_BYTE( cpustate->PC & 0xFF );
	PUSH_BYTE( cpustate->PC >> 8 );
	/* Push PS1 */
	PUSH_BYTE( cpustate->PS1 );
	/* Clear I flag */
	cpustate->PS1 &= ~ 0x01;
	/* save regs to ram */
	cpustate->program->write_byte (0x1f, cpustate->PS1);
	cpustate->program->write_byte (0x1d, cpustate->SP&0xFF);
	if (cpustate->SYS&0x40) cpustate->program->write_byte(0x1c, cpustate->SP>>8);
	/* Change PC to address stored at "vector" */
	cpustate->PC = sm85cpu_mem_readword( cpustate, vector );
}

INLINE void sm8500_process_interrupts(sm8500_state *cpustate) {
	if ( cpustate->CheckInterrupts ) {
		int irqline = 0;
		while( irqline < 11 ) {
			if ( cpustate->IFLAGS & ( 1 << irqline ) ) {
				cpustate->halted = 0;
				cpustate->IE0 = cpustate->program->read_byte(0x10);
				cpustate->IE1 = cpustate->program->read_byte(0x11);
				cpustate->IR0 = cpustate->program->read_byte(0x12);
				cpustate->IR1 = cpustate->program->read_byte(0x13);
				cpustate->PS0 = cpustate->program->read_byte(0x1e);
				cpustate->PS1 = cpustate->program->read_byte(0x1f);
				switch( irqline ) {
				case WDT_INT:
					sm8500_do_interrupt( cpustate, 0x101C );
					break;
				case ILL_INT:
				case NMI_INT:
					sm8500_do_interrupt( cpustate, 0x101E );
					break;
				case DMA_INT:
					cpustate->IR0 |= 0x80;
					if ( ( cpustate->IE0 & 0x80 ) && ( ( cpustate->PS0 & 0x07 ) < 8 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x1000 );
					}
					break;
				case TIM0_INT:
					cpustate->IR0 |= 0x40;
					if ( ( cpustate->IE0 & 0x40 ) && ( ( cpustate->PS0 & 0x07 ) < 8 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x1002 );
					}
					break;
				case EXT_INT:
					cpustate->IR0 |= 0x10;
					if ( ( cpustate->IE0 & 0x10 ) && ( ( cpustate->PS0 & 0x07 ) < 7 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x1006 );
					}
					break;
				case UART_INT:
					cpustate->IR0 |= 0x08;
					if ( ( cpustate->IE0 & 0x08 ) && ( ( cpustate->PS0 & 0x07 ) < 6 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x1008 );
					}
					break;
				case LCDC_INT:
					cpustate->IR0 |= 0x01;
					if ( ( cpustate->IE0 & 0x01 ) && ( ( cpustate->PS0 & 0x07 ) < 5 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x100E );
					}
					break;
				case TIM1_INT:
					cpustate->IR1 |= 0x40;
					if ( ( cpustate->IE1 & 0x40 ) && ( ( cpustate->PS0 & 0x07 ) < 4 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x1012 );
					}
					break;
				case CK_INT:
					cpustate->IR1 |= 0x10;
					if ( ( cpustate->IE1 & 0x10 ) && ( ( cpustate->PS0 & 0x07 ) < 3 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x1016 );
					}
					break;
				case PIO_INT:
					cpustate->IR1 |= 0x04;
					if ( ( cpustate->IE1 & 0x04 ) && ( ( cpustate->PS0 & 0x07 ) < 2 ) && ( cpustate->PS1 & 0x01 ) ) {
						sm8500_do_interrupt( cpustate, 0x101A );
					}
					break;
				}
				cpustate->IFLAGS &= ~ ( 1 << irqline );
				cpustate->program->write_byte(0x12, cpustate->IR0);
				cpustate->program->write_byte(0x13, cpustate->IR1);
			}
			irqline++;
		}
	}
}

static CPU_EXECUTE( sm8500 )
{
	sm8500_state *cpustate = get_safe_token(device);
	UINT8	op;
	int	mycycles;

	do
	{
		UINT8	r1,r2;
		UINT16	s1,s2;
		UINT32	d1,d2;
		UINT32	res;

		debugger_instruction_hook(device, cpustate->PC);
		cpustate->oldpc = cpustate->PC;
		mycycles = 0;
		sm8500_process_interrupts(cpustate);
		if ( !cpustate->halted ) {
			op = sm85cpu_mem_readbyte( cpustate, cpustate->PC++ );
			cpustate->SYS = cpustate->program->read_byte(0x19);
			cpustate->PS0 = cpustate->program->read_byte(0x1e);
			cpustate->PS1 = cpustate->program->read_byte(0x1f);
			sm8500_get_sp(cpustate);
			switch( op )
			{
#include "sm85ops.h"
			}
			if (cpustate->SYS&0x40) cpustate->program->write_byte(0x1c,cpustate->SP>>8);
			cpustate->program->write_byte(0x1d,cpustate->SP&0xFF);
			sm85cpu_mem_writebyte(cpustate,0x1e,cpustate->PS0);	// need to update debugger
			cpustate->program->write_byte(0x1f,cpustate->PS1);
		} else {
			mycycles = 4;
			if ( cpustate->config.handle_dma ) {
				cpustate->config.handle_dma( device, mycycles );
			}
		}
		if ( cpustate->config.handle_timers ) {
			cpustate->config.handle_timers( device, mycycles );
		}
		cpustate->icount -= mycycles;
	} while ( cpustate->icount > 0 );
}

static CPU_BURN( sm8500 )
{
	sm8500_state *cpustate = get_safe_token(device);

	if ( cycles > 0 ) {
		/* burn a number of 4 cycles */
		int n = ( cycles + 3 ) / 4;
		cpustate->icount -= 4 * n;
	}
}

static unsigned sm8500_get_reg( sm8500_state *cpustate, int regnum )
{
	switch( regnum )
	{
	case STATE_GENPC:
	case SM8500_PC:		return cpustate->PC;
	case STATE_GENSP:
	case SM8500_SP:		return cpustate->SP;
	case SM8500_PS:		return sm85cpu_mem_readword( cpustate, 0x1e );
	case SM8500_SYS16:	return cpustate->SYS;
	case SM8500_RR0:	return sm85cpu_mem_readword( cpustate, 0x00 );
	case SM8500_RR2:	return sm85cpu_mem_readword( cpustate, 0x02 );
	case SM8500_RR4:	return sm85cpu_mem_readword( cpustate, 0x04 );
	case SM8500_RR6:	return sm85cpu_mem_readword( cpustate, 0x06 );
	case SM8500_RR8:	return sm85cpu_mem_readword( cpustate, 0x08 );
	case SM8500_RR10:	return sm85cpu_mem_readword( cpustate, 0x0A );
	case SM8500_RR12:	return sm85cpu_mem_readword( cpustate, 0x0C );
	case SM8500_RR14:	return sm85cpu_mem_readword( cpustate, 0x0E );
	case SM8500_IE0:	return sm85cpu_mem_readbyte( cpustate, 0x10 );
	case SM8500_IE1:	return sm85cpu_mem_readbyte( cpustate, 0x11 );
	case SM8500_IR0:	return sm85cpu_mem_readbyte( cpustate, 0x12 );
	case SM8500_IR1:	return sm85cpu_mem_readbyte( cpustate, 0x13 );
	case SM8500_P0:		return sm85cpu_mem_readbyte( cpustate, 0x14 );
	case SM8500_P1:		return sm85cpu_mem_readbyte( cpustate, 0x15 );
	case SM8500_P2:		return sm85cpu_mem_readbyte( cpustate, 0x16 );
	case SM8500_P3:		return sm85cpu_mem_readbyte( cpustate, 0x17 );
	case SM8500_SYS:	return sm85cpu_mem_readbyte( cpustate, 0x19 );
	case SM8500_CKC:	return sm85cpu_mem_readbyte( cpustate, 0x1a );
	case SM8500_SPH:	return sm85cpu_mem_readbyte( cpustate, 0x1c );
	case SM8500_SPL:	return sm85cpu_mem_readbyte( cpustate, 0x1d );
	case SM8500_PS0:	return sm85cpu_mem_readbyte( cpustate, 0x1e );
	case SM8500_PS1:	return sm85cpu_mem_readbyte( cpustate, 0x1f );
	case SM8500_P0C:	return sm85cpu_mem_readbyte( cpustate, 0x20 );
	case SM8500_P1C:	return sm85cpu_mem_readbyte( cpustate, 0x21 );
	case SM8500_P2C:	return sm85cpu_mem_readbyte( cpustate, 0x22 );
	case SM8500_P3C:	return sm85cpu_mem_readbyte( cpustate, 0x23 );
	}
	return 0;
}

static void sm8500_set_reg( sm8500_state *cpustate, int regnum, unsigned val )
{
	switch( regnum )
	{
	case STATE_GENPC:
	case SM8500_PC:		cpustate->PC = val; break;
	case STATE_GENSP:
	case SM8500_SP:		cpustate->SP = val; cpustate->program->write_byte(0x1d, val&0xff); if (cpustate->SYS&0x40) cpustate->program->write_byte(0x1c, val>>8); break;
	case SM8500_PS:		sm85cpu_mem_writeword( cpustate, 0x1e, val); break;
	case SM8500_SYS16:	val&=0xff; sm85cpu_mem_writebyte( cpustate, 0x19, val); break;
	case SM8500_RR0:	sm85cpu_mem_writeword( cpustate, 0x00, val); break;
	case SM8500_RR2:	sm85cpu_mem_writeword( cpustate, 0x02, val); break;
	case SM8500_RR4:	sm85cpu_mem_writeword( cpustate, 0x04, val); break;
	case SM8500_RR6:	sm85cpu_mem_writeword( cpustate, 0x06, val); break;
	case SM8500_RR8:	sm85cpu_mem_writeword( cpustate, 0x08, val); break;
	case SM8500_RR10:	sm85cpu_mem_writeword( cpustate, 0x0A, val); break;
	case SM8500_RR12:	sm85cpu_mem_writeword( cpustate, 0x0C, val); break;
	case SM8500_RR14:	sm85cpu_mem_writeword( cpustate, 0x0E, val); break;
	case SM8500_IE0:	sm85cpu_mem_writebyte( cpustate, 0x10, val); break;
	case SM8500_IE1:	sm85cpu_mem_writebyte( cpustate, 0x11, val); break;
	case SM8500_IR0:	sm85cpu_mem_writebyte( cpustate, 0x12, val); break;
	case SM8500_IR1:	sm85cpu_mem_writebyte( cpustate, 0x13, val); break;
	case SM8500_P0:		sm85cpu_mem_writebyte( cpustate, 0x14, val); break;
	case SM8500_P1:		sm85cpu_mem_writebyte( cpustate, 0x15, val); break;
	case SM8500_P2:		sm85cpu_mem_writebyte( cpustate, 0x16, val); break;
	case SM8500_P3:		sm85cpu_mem_writebyte( cpustate, 0x17, val); break;
	case SM8500_SYS:	sm85cpu_mem_writebyte( cpustate, 0x19, val); break;
	case SM8500_CKC:	sm85cpu_mem_writebyte( cpustate, 0x1a, val); if ( val & 0x80 ) { cpustate->clock_changed = 1; }; break;
	case SM8500_SPH:	sm85cpu_mem_writebyte( cpustate, 0x1c, val); break;
	case SM8500_SPL:	sm85cpu_mem_writebyte( cpustate, 0x1d, val); break;
	case SM8500_PS0:	sm85cpu_mem_writebyte( cpustate, 0x1e, val); break;
	case SM8500_PS1:	sm85cpu_mem_writebyte( cpustate, 0x1f, val); break;
	case SM8500_P0C:	sm85cpu_mem_writebyte( cpustate, 0x20, val); break;
	case SM8500_P1C:	sm85cpu_mem_writebyte( cpustate, 0x21, val); break;
	case SM8500_P2C:	sm85cpu_mem_writebyte( cpustate, 0x22, val); break;
	case SM8500_P3C:	sm85cpu_mem_writebyte( cpustate, 0x23, val); break;
	}
}

static void sm8500_set_irq_line( sm8500_state *cpustate, int irqline, int state )
{
	cpustate->IR0 = cpustate->program->read_byte(0x12);
	cpustate->IR1 = cpustate->program->read_byte(0x13);
	if ( state == ASSERT_LINE ) {
		cpustate->IFLAGS |= ( 0x01 << irqline );
		cpustate->CheckInterrupts = 1;
		switch( irqline ) {
		case DMA_INT:	cpustate->IR0 |= 0x80; break;
		case TIM0_INT:	cpustate->IR0 |= 0x40; break;
		case EXT_INT:	cpustate->IR0 |= 0x10; break;
		case UART_INT:	cpustate->IR0 |= 0x08; break;
		case LCDC_INT:	cpustate->IR0 |= 0x01; break;
		case TIM1_INT:	cpustate->IR1 |= 0x40; break;
		case CK_INT:	cpustate->IR1 |= 0x10; break;
		case PIO_INT:	cpustate->IR1 |= 0x04; break;
		}
	} else {
		cpustate->IFLAGS &= ~( 0x01 << irqline );
		switch( irqline ) {
		case DMA_INT:	cpustate->IR0 &= ~0x80; break;
		case TIM0_INT:	cpustate->IR0 &= ~0x40; break;
		case EXT_INT:	cpustate->IR0 &= ~0x10; break;
		case UART_INT:	cpustate->IR0 &= ~0x08; break;
		case LCDC_INT:	cpustate->IR0 &= ~0x01; break;
		case TIM1_INT:	cpustate->IR1 &= ~0x40; break;
		case CK_INT:	cpustate->IR1 &= ~0x10; break;
		case PIO_INT:	cpustate->IR1 &= ~0x04; break;
		}
		if ( 0 == cpustate->IFLAGS ) {
			cpustate->CheckInterrupts = 0;
		}
	}
	cpustate->program->write_byte(0x12, cpustate->IR0);
	cpustate->program->write_byte(0x13, cpustate->IR1);
}

static CPU_SET_INFO( sm8500 )
{
	sm8500_state *cpustate = get_safe_token(device);

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
		sm8500_set_irq_line( cpustate, state - CPUINFO_INT_INPUT_STATE, info->i ); break;

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
		sm8500_set_reg( cpustate, state - CPUINFO_INT_REGISTER, info->i ); break;

	}
}

CPU_GET_INFO( sm8500 )
{
	sm8500_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
	case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(sm8500_state); break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 8; break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff; break;
	case DEVINFO_INT_ENDIANNESS:				info->i = ENDIANNESS_BIG; break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1; break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1; break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1; break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5; break;
	case CPUINFO_INT_MIN_CYCLES:				info->i = 1; break;
	case CPUINFO_INT_MAX_CYCLES:				info->i = 16; break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 16; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0; break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0; break;
	case DEVINFO_INT_DATABUS_WIDTH + AS_IO:	info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:	info->i = 0; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:	info->i = 0; break;
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
	case CPUINFO_INT_INPUT_STATE + 10:			info->i = cpustate->IFLAGS & ( 1 << (state - CPUINFO_INT_INPUT_STATE)); break;
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
								info->i = sm8500_get_reg( cpustate, state - CPUINFO_INT_REGISTER ); break;
	case CPUINFO_INT_REGISTER + STATE_GENPC:			info->i = sm8500_get_reg( cpustate, SM8500_PC ); break;
	case CPUINFO_INT_REGISTER + STATE_GENSP:			info->i = sm8500_get_reg( cpustate, SM8500_SP ); break;
	case CPUINFO_INT_PREVIOUSPC:				info->i = cpustate->oldpc; break;


	case CPUINFO_FCT_SET_INFO:				info->setinfo = CPU_SET_INFO_NAME(sm8500); break;
	case CPUINFO_FCT_INIT:					info->init = CPU_INIT_NAME(sm8500); break;
	case CPUINFO_FCT_RESET:					info->reset = CPU_RESET_NAME(sm8500); break;
	case CPUINFO_FCT_EXIT:					info->exit = CPU_EXIT_NAME(sm8500); break;
	case CPUINFO_FCT_EXECUTE:				info->execute = CPU_EXECUTE_NAME(sm8500); break;
	case CPUINFO_FCT_BURN:					info->burn = CPU_BURN_NAME(sm8500); break;
	case CPUINFO_FCT_DISASSEMBLE:			info->disassemble = CPU_DISASSEMBLE_NAME(sm8500); break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount; break;

	case DEVINFO_STR_NAME:					strcpy( info->s, "sm8500" ); break;
	case DEVINFO_STR_FAMILY:				strcpy( info->s, "Sharp SM8500" ); break;
	case DEVINFO_STR_VERSION:				strcpy( info->s, "0.1" ); break;
	case DEVINFO_STR_SOURCE_FILE:				strcpy( info->s, __FILE__ ); break;
	case DEVINFO_STR_CREDITS:				strcpy( info->s, "Copyright The MESS Team." ); break;
	case CPUINFO_STR_FLAGS:
		sprintf( info->s, "%c%c%c%c%c%c%c%c",
			cpustate->PS1 & FLAG_C ? 'C' : '.',
			cpustate->PS1 & FLAG_Z ? 'Z' : '.',
			cpustate->PS1 & FLAG_S ? 'S' : '.',
			cpustate->PS1 & FLAG_V ? 'V' : '.',
			cpustate->PS1 & FLAG_D ? 'D' : '.',
			cpustate->PS1 & FLAG_H ? 'H' : '.',
			cpustate->PS1 & FLAG_B ? 'B' : '.',
			cpustate->PS1 & FLAG_I ? 'I' : '.' );
		break;
	case CPUINFO_STR_REGISTER + SM8500_RR0:			sprintf(info->s, "RR0:%04X", sm85cpu_mem_readword( cpustate, 0x00 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR2:			sprintf(info->s, "RR2:%04X", sm85cpu_mem_readword( cpustate, 0x02 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR4:			sprintf(info->s, "RR4:%04X", sm85cpu_mem_readword( cpustate, 0x04 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR6:			sprintf(info->s, "RR6:%04X", sm85cpu_mem_readword( cpustate, 0x06 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR8:			sprintf(info->s, "RR8:%04X", sm85cpu_mem_readword( cpustate, 0x08 ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR10:		sprintf(info->s, "RR10:%04X", sm85cpu_mem_readword( cpustate, 0x0A ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR12:		sprintf(info->s, "RR12:%04X", sm85cpu_mem_readword( cpustate, 0x0C ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_RR14:		sprintf(info->s, "RR14:%04X", sm85cpu_mem_readword( cpustate, 0x0E ) ); break;
	case CPUINFO_STR_REGISTER + SM8500_PC:			sprintf(info->s, "PC:%04X", cpustate->PC); break;
	case CPUINFO_STR_REGISTER + SM8500_SP:			sprintf(info->s, "SP:%04X", cpustate->SP); break;
	case CPUINFO_STR_REGISTER + SM8500_PS:			sprintf(info->s, "PS:%04X", ( cpustate->PS0 << 8 ) | cpustate->PS1 ); break;
	case CPUINFO_STR_REGISTER + SM8500_SYS16:		sprintf(info->s, "SYS:%02X", cpustate->SYS ); break;
	}
}


DEFINE_LEGACY_CPU_DEVICE(SM8500, sm8500);
