// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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
- DIV (CE D9) division by zero handling - is supposed to raise a EX4 exception. A real Pokemini unit will freeze. MAME currently will crash.

*/

#include "emu.h"
#include "minx.h"
#include "minxd.h"

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


#define RD(offset)      m_program->read_byte( offset )
#define WR(offset,data) m_program->write_byte( offset, data )
#define GET_MINX_PC     ( ( m_PC & 0x8000 ) ? ( m_V << 15 ) | (m_PC & 0x7FFF ) : m_PC )


DEFINE_DEVICE_TYPE(MINX, minx_cpu_device, "minx", "Nintendo Minx")


minx_cpu_device::minx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, MINX, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 24, 0)
{
}

device_memory_interface::space_config_vector minx_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

uint16_t minx_cpu_device::rd16( uint32_t offset )
{
	return RD( offset ) | ( RD( offset + 1 ) << 8 );
}


void minx_cpu_device::wr16( uint32_t offset, uint16_t data )
{
	WR( offset, ( data & 0x00FF ) );
	WR( offset + 1, ( data >> 8 ) );
}


void minx_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	state_add( MINX_PC, "PC", m_PC ).formatstr("%04X");
	state_add( MINX_SP, "SP", m_SP ).formatstr("%04X");
	state_add( MINX_BA, "BA", m_BA ).formatstr("%04X");
	state_add( MINX_HL, "HL", m_HL ).formatstr("%04X");
	state_add( MINX_X,  "X", m_X ).formatstr("%04X");
	state_add( MINX_Y,  "Y", m_Y ).formatstr("%04X");
	state_add( MINX_U,  "U", m_U ).formatstr("%02X");
	state_add( MINX_V,  "V", m_V ).formatstr("%02X");
	state_add( MINX_F,  "F", m_F ).formatstr("%02X");
	state_add( MINX_E,  "E", m_E ).formatstr("%02X");
	state_add( MINX_N,  "N", m_N ).formatstr("%02X");
	state_add( MINX_I,  "I", m_I ).formatstr("%02X");
	state_add( MINX_XI, "XI", m_XI ).formatstr("%02X");
	state_add( MINX_YI, "YI", m_YI ).formatstr("%02X");

	state_add(STATE_GENPC, "GENPC", m_curpc).formatstr("%06X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_curpc).formatstr("%06X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%14s").noshow();

	set_icountptr(m_icount);
}


void minx_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c-%c%c%c%c%c",
				m_F & FLAG_I ? 'I' : '.',
				m_F & FLAG_D ? 'D' : '.',
				m_F & FLAG_L ? 'L' : '.',
				m_F & FLAG_B ? 'B' : '.',
				m_F & FLAG_S ? 'S' : '.',
				m_F & FLAG_O ? 'O' : '.',
				m_F & FLAG_C ? 'C' : '.',
				m_F & FLAG_Z ? 'Z' : '.',
				m_E & EXEC_X0 ? '0' : '.',
				m_E & EXEC_X1 ? '1' : '.',
				m_E & EXEC_X2 ? '2' : '.',
				m_E & EXEC_DZ ? 'z' : '.',
				m_E & EXEC_EN ? 'E' : '.' );
			break;
	}
}


void minx_cpu_device::device_reset()
{
	m_SP = m_BA = m_HL = m_X = m_Y = 0;
	m_U = m_V = m_F = m_E = m_I = m_XI = m_YI = 0;
	m_halted = m_interrupt_pending = 0;

	m_PC = rd16( 0 );
}


uint8_t minx_cpu_device::rdop()
{
	uint8_t op = RD( GET_MINX_PC );
	m_PC++;
	return op;
}


uint16_t minx_cpu_device::rdop16()
{
	uint16_t op = rdop();
	op = op | ( rdop() << 8 );
	return op;
}


#include "minxfunc.h"
#include "minxopce.h"
#include "minxopcf.h"
#include "minxops.h"


void minx_cpu_device::execute_run()
{
	do
	{
		m_curpc = GET_MINX_PC;
		debugger_instruction_hook(m_curpc);

		if ( m_interrupt_pending )
		{
			m_halted = 0;
			if ( ! ( m_F & 0xc0 ) && m_U == m_V )
			{
				//logerror("minx_execute(): taking IRQ\n");
				PUSH8( m_V );
				PUSH16( m_PC );
				PUSH8( m_F );

				/* Set Interrupt Branch flag */
				m_F |= 0x80;
				m_V = 0;
				m_PC = rd16( standard_irq_callback( 0, m_PC ) << 1 );
				m_icount -= 28;     /* This cycle count is a guess */
			}
		}

		if ( m_halted )
		{
			debugger_wait_hook();
			m_icount -= insnminx_cycles_CE[0xAE];
		}
		else
		{
			execute_one();
		}
	} while ( m_icount > 0 );
}


void minx_cpu_device::execute_set_input(int inputnum, int state)
{
	if ( state == ASSERT_LINE )
	{
		m_interrupt_pending = 1;
	}
	else
	{
		m_interrupt_pending = 0;
	}
}


std::unique_ptr<util::disasm_interface> minx_cpu_device::create_disassembler()
{
	return std::make_unique<minx_disassembler>();
}
