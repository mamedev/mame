// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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


const device_type SM8500 = &device_creator<sm8500_cpu_device>;


static const UINT8 sm8500_b2w[8] = {
		0, 8, 2, 10, 4, 12, 6, 14
};


sm8500_cpu_device::sm8500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SM8500, "SM8500", tag, owner, clock, "sm8500", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_dma_func(*this)
	, m_timer_func(*this)
	, m_PC(0), m_IE0(0), m_IE1(0), m_IR0(0), m_IR1(0)
		, m_SYS(0), m_CKC(0), m_clock_changed(0)
		, m_SP(0)
	, m_PS0(0)
	, m_PS1(0), m_IFLAGS(0), m_CheckInterrupts(0), m_halted(0), m_icount(0), m_program(nullptr), m_oldpc(0)
{
}


void sm8500_cpu_device::get_sp()
{
	m_SP = m_program->read_byte(0x1d);
	if (m_SYS & 0x40) m_SP |= ( m_program->read_byte(0x1c) << 8 );
}


UINT8 sm8500_cpu_device::mem_readbyte( UINT32 offset ) const
{
	offset &= 0xffff;
	if ( offset < 0x10)
	{
		return m_register_ram[offset + (m_PS0 & 0xF8)];
	}

	return m_program->read_byte( offset );
}


void sm8500_cpu_device::mem_writebyte( UINT32 offset, UINT8 data )
{
	UINT8 i;
	offset &= 0xffff;
	if (offset < 0x10)
	{
		m_register_ram[offset + (m_PS0 & 0xF8)] = data;
	}

	m_program->write_byte( offset, data );

	switch (offset)
	{
		case 0x10: m_IE0 = data; break;
		case 0x11: m_IE1 = data; break;
		case 0x12: m_IR0 = data; break;
		case 0x13: m_IR1 = data; break;
		case 0x19: m_SYS = data; break;
		case 0x1a: m_CKC = data; break;
		case 0x1c:
		case 0x1d: get_sp(); break;
		case 0x1e: m_PS0 = data;
				for (i = 0; i < 16; i++)    // refresh register contents in debugger
				{
					m_program->write_byte(i, mem_readbyte(i));
				}
				break;
		case 0x1f: m_PS1 = data; break;
	}
}


void sm8500_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	m_dma_func.resolve_safe();
	m_timer_func.resolve_safe();

	save_item(NAME(m_PC));
	save_item(NAME(m_IE0));
	save_item(NAME(m_IE1));
	save_item(NAME(m_IR0));
	save_item(NAME(m_IR1));
	save_item(NAME(m_SYS));
	save_item(NAME(m_CKC));
	save_item(NAME(m_clock_changed));
	save_item(NAME(m_SP));
	save_item(NAME(m_PS0));
	save_item(NAME(m_PS1));
	save_item(NAME(m_IFLAGS));
	save_item(NAME(m_CheckInterrupts));
	save_item(NAME(m_halted));
	save_item(NAME(m_oldpc));
	save_pointer(NAME(m_register_ram),0x108);

	// Register state for debugger
	state_add(SM8500_PC, "PC", m_PC ).callimport().callexport().formatstr("%04X");
	state_add(SM8500_SP, "SP", m_SP ).callimport().callexport().formatstr("%04X");
	state_add(SM8500_PS, "PS", m_PS0 ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_SYS, "SYS", m_SYS ).callimport().callexport().formatstr("%04X");
	state_add(SM8500_RR0, "RR0", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR2, "RR2", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR4, "RR4", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR6, "RR6", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR8, "RR8", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR10, "RR10", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR12, "RR12", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(SM8500_RR14, "RR14", m_PC ).callimport().callexport().formatstr("%04s");
	state_add(STATE_GENPC, "curpc", m_PC).callimport().callexport().formatstr("%8s").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_PS1).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}


void sm8500_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case SM8500_PS:
			str = string_format("%04X", ( m_PS0 << 8 ) | m_PS1 );
			break;

		case SM8500_RR0:
			str = string_format("%04X", mem_readword( 0x00 ) );
			break;

		case SM8500_RR2:
			str = string_format("%04X", mem_readword( 0x02 ) );
			break;

		case SM8500_RR4:
			str = string_format("%04X", mem_readword( 0x04 ) );
			break;

		case SM8500_RR6:
			str = string_format("%04X", mem_readword( 0x06 ) );
			break;

		case SM8500_RR8:
			str = string_format("%04X", mem_readword( 0x08 ) );
			break;

		case SM8500_RR10:
			str = string_format("%04X", mem_readword( 0x0a ) );
			break;

		case SM8500_RR12:
			str = string_format("%04X", mem_readword( 0x0c ) );
			break;

		case SM8500_RR14:
			str = string_format("%04X", mem_readword( 0x0e ) );
			break;

		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_PS1 & FLAG_C ? 'C' : '.',
				m_PS1 & FLAG_Z ? 'Z' : '.',
				m_PS1 & FLAG_S ? 'S' : '.',
				m_PS1 & FLAG_V ? 'V' : '.',
				m_PS1 & FLAG_D ? 'D' : '.',
				m_PS1 & FLAG_H ? 'H' : '.',
				m_PS1 & FLAG_B ? 'B' : '.',
				m_PS1 & FLAG_I ? 'I' : '.' );
			break;
	}
}


void sm8500_cpu_device::device_reset()
{
	for (auto & elem : m_register_ram)
	{
		elem = 0;
	}

	m_PC = 0x1020;
	m_clock_changed = 0;
	m_CheckInterrupts = 0;
	m_halted = 0;
	m_IFLAGS = 0;
	mem_writeword(0x10, 0);                 // IE0, IE1
	mem_writeword(0x12, 0);                 // IR0, IR1
	mem_writeword(0x14, 0xffff);            // P0, P1
	mem_writeword(0x16, 0xff00);            // P2, P3
	mem_writebyte(0x19, 0);                 // SYS
	mem_writebyte(0x1a, 0);                 // CKC
	mem_writebyte(0x1f, 0);                 // PS1
	mem_writebyte(0x2b, 0xff);              // URTT
	mem_writebyte(0x2d, 0x42);              // URTS
	mem_writebyte(0x5f, 0x38);              // WDTC
}


#define PUSH_BYTE(X)    m_SP--; \
			if ( ( m_SYS & 0x40 ) == 0 ) m_SP &= 0xFF; \
			mem_writebyte( m_SP, X );


void sm8500_cpu_device::take_interrupt(UINT16 vector)
{
	/* Get regs from ram */
	get_sp();
	m_SYS = m_program->read_byte(0x19);
	m_PS1 = m_program->read_byte(0x1f);
	/* Push PC */
	PUSH_BYTE( m_PC & 0xFF );
	PUSH_BYTE( m_PC >> 8 );
	/* Push PS1 */
	PUSH_BYTE( m_PS1 );
	/* Clear I flag */
	m_PS1 &= ~ 0x01;
	/* save regs to ram */
	m_program->write_byte(0x1f, m_PS1);
	m_program->write_byte(0x1d, m_SP&0xFF);
	if (m_SYS&0x40) m_program->write_byte(0x1c, m_SP>>8);
	/* Change PC to address stored at "vector" */
	m_PC = mem_readword( vector );
}


void sm8500_cpu_device::process_interrupts()
{
	if ( m_CheckInterrupts )
	{
		int irqline = 0;
		while( irqline < 11 )
		{
			if ( m_IFLAGS & ( 1 << irqline ) )
			{
				m_halted = 0;
				m_IE0 = m_program->read_byte(0x10);
				m_IE1 = m_program->read_byte(0x11);
				m_IR0 = m_program->read_byte(0x12);
				m_IR1 = m_program->read_byte(0x13);
				m_PS0 = m_program->read_byte(0x1e);
				m_PS1 = m_program->read_byte(0x1f);
				switch( irqline )
				{
				case WDT_INT:
					take_interrupt( 0x101C );
					break;
				case ILL_INT:
				case NMI_INT:
					take_interrupt( 0x101E );
					break;
				case DMA_INT:
					m_IR0 |= 0x80;
					if ( ( m_IE0 & 0x80 ) && ( ( m_PS0 & 0x07 ) < 8 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x1000 );
					}
					break;
				case TIM0_INT:
					m_IR0 |= 0x40;
					if ( ( m_IE0 & 0x40 ) && ( ( m_PS0 & 0x07 ) < 8 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x1002 );
					}
					break;
				case EXT_INT:
					m_IR0 |= 0x10;
					if ( ( m_IE0 & 0x10 ) && ( ( m_PS0 & 0x07 ) < 7 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x1006 );
					}
					break;
				case UART_INT:
					m_IR0 |= 0x08;
					if ( ( m_IE0 & 0x08 ) && ( ( m_PS0 & 0x07 ) < 6 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x1008 );
					}
					break;
				case LCDC_INT:
					m_IR0 |= 0x01;
					if ( ( m_IE0 & 0x01 ) && ( ( m_PS0 & 0x07 ) < 5 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x100E );
					}
					break;
				case TIM1_INT:
					m_IR1 |= 0x40;
					if ( ( m_IE1 & 0x40 ) && ( ( m_PS0 & 0x07 ) < 4 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x1012 );
					}
					break;
				case CK_INT:
					m_IR1 |= 0x10;
					if ( ( m_IE1 & 0x10 ) && ( ( m_PS0 & 0x07 ) < 3 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x1016 );
					}
					break;
				case PIO_INT:
					m_IR1 |= 0x04;
					if ( ( m_IE1 & 0x04 ) && ( ( m_PS0 & 0x07 ) < 2 ) && ( m_PS1 & 0x01 ) )
					{
						take_interrupt( 0x101A );
					}
					break;
				}
				m_IFLAGS &= ~ ( 1 << irqline );
				m_program->write_byte(0x12, m_IR0);
				m_program->write_byte(0x13, m_IR1);
			}
			irqline++;
		}
	}
}


offs_t sm8500_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( sm8500 );
	return CPU_DISASSEMBLE_NAME(sm8500)(this, buffer, pc, oprom, opram, options);
}


void sm8500_cpu_device::execute_run()
{
	do
	{
		int     mycycles = 0;
		UINT8   r1,r2;
		UINT16  s1,s2;
		UINT32  d1,d2;
		UINT32  res;

		debugger_instruction_hook(this, m_PC);
		m_oldpc = m_PC;
		process_interrupts();
		if ( !m_halted ) {
			UINT8 op = mem_readbyte( m_PC++ );
			m_SYS = m_program->read_byte(0x19);
			m_PS0 = m_program->read_byte(0x1e);
			m_PS1 = m_program->read_byte(0x1f);
			get_sp();
			switch( op )
			{
#include "sm85ops.h"
			}
			if (m_SYS&0x40) m_program->write_byte(0x1c,m_SP>>8);
			m_program->write_byte(0x1d,m_SP&0xFF);
			mem_writebyte(0x1e,m_PS0); // need to update debugger
			m_program->write_byte(0x1f,m_PS1);
		} else {
			mycycles = 4;
			m_dma_func( mycycles );
		}
		m_timer_func( mycycles );
		m_icount -= mycycles;
	} while ( m_icount > 0 );
}


void sm8500_cpu_device::execute_set_input( int inptnum, int state )
{
	m_IR0 = m_program->read_byte(0x12);
	m_IR1 = m_program->read_byte(0x13);
	if ( state == ASSERT_LINE )
	{
		m_IFLAGS |= ( 0x01 << inptnum );
		m_CheckInterrupts = 1;
		switch( inptnum )
		{
			case DMA_INT:   m_IR0 |= 0x80; break;
			case TIM0_INT:  m_IR0 |= 0x40; break;
			case EXT_INT:   m_IR0 |= 0x10; break;
			case UART_INT:  m_IR0 |= 0x08; break;
			case LCDC_INT:  m_IR0 |= 0x01; break;
			case TIM1_INT:  m_IR1 |= 0x40; break;
			case CK_INT:    m_IR1 |= 0x10; break;
			case PIO_INT:   m_IR1 |= 0x04; break;
		}
	}
	else
	{
		m_IFLAGS &= ~( 0x01 << inptnum );
		switch( inptnum )
		{
			case DMA_INT:   m_IR0 &= ~0x80; break;
			case TIM0_INT:  m_IR0 &= ~0x40; break;
			case EXT_INT:   m_IR0 &= ~0x10; break;
			case UART_INT:  m_IR0 &= ~0x08; break;
			case LCDC_INT:  m_IR0 &= ~0x01; break;
			case TIM1_INT:  m_IR1 &= ~0x40; break;
			case CK_INT:    m_IR1 &= ~0x10; break;
			case PIO_INT:   m_IR1 &= ~0x04; break;
		}
		if ( 0 == m_IFLAGS )
		{
			m_CheckInterrupts = 0;
		}
	}
	m_program->write_byte(0x12, m_IR0);
	m_program->write_byte(0x13, m_IR1);
}
