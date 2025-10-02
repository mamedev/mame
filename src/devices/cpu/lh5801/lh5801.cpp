// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *
 *    02/2011 (Sandro Ronco)
 *   - Added IO_SPACE and updated all access in ME1 memory for use it.
 *   - Implemented interrupts.
 *   - Fixed the flags in the ROL/ROR/SHL/SHR opcodes.
 *   - Fixed decimal add/sub opcodes.
 *
 * Based on info found in an article for the Tandy TRS-80 PC2
 * and in the PC1500 Technical Reference Manual.
 *
 *****************************************************************************/

#include "emu.h"
#include "lh5801.h"
#include "5801dasm.h"

#define VERBOSE 0

#include "logmacro.h"


enum
{
	LH5801_T=1,
	LH5801_P,
	LH5801_S,
	LH5801_U,
	LH5801_X,
	LH5801_Y,
	LH5801_A,

	LH5801_TM,
	LH5801_IN,
	LH5801_BF,
	LH5801_PU,
	LH5801_PV,
	LH5801_DP,
	LH5801_IRQ_STATE
};


#define P m_p.w.l
#define S m_s.w.l
#define U m_u.w.l
#define UL m_u.b.l
#define UH m_u.b.h
#define X m_x.w.l
#define XL m_x.b.l
#define XH m_x.b.h
#define Y m_y.w.l
#define YL m_y.b.l
#define YH m_y.b.h

#define C 0x01
#define IE 0x02
#define Z 0x04
#define V 0x08
#define H 0x10


DEFINE_DEVICE_TYPE(LH5801, lh5801_cpu_device, "lh5801", "Sharp LH5801")


lh5801_cpu_device::lh5801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, LH5801, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_in_func(*this, 0)
{
}

device_memory_interface::space_config_vector lh5801_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "5801tbl.hxx"

void lh5801_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_IO).specific(m_io);

	m_s.w.l = 0;
	m_p.w.l = 0;
	m_u.w.l = 0;
	m_x.w.l = 0;
	m_y.w.l = 0;
	m_tm = 0;
	m_t = 0;
	m_a = 0;
	m_bf = 0;
	m_dp = 0;
	m_pu = 0;
	m_pv = 0;
	m_oldpc = 0;
	m_irq_state = 0;
	memset(m_ir_flipflop, 0, sizeof(m_ir_flipflop));
	memset(m_lines_status, 0, sizeof(m_lines_status));
	m_idle = 0;

	save_item(NAME(m_s.w.l));
	save_item(NAME(m_p.w.l));
	save_item(NAME(m_u.w.l));
	save_item(NAME(m_x.w.l));
	save_item(NAME(m_y.w.l));
	save_item(NAME(m_tm));
	save_item(NAME(m_t));
	save_item(NAME(m_a));
	save_item(NAME(m_bf));
	save_item(NAME(m_dp));
	save_item(NAME(m_pu));
	save_item(NAME(m_pv));
	save_item(NAME(m_oldpc));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_ir_flipflop));
	save_item(NAME(m_lines_status));
	save_item(NAME(m_idle));

	state_add( LH5801_P,  "P",  m_p.w.l ).formatstr("%04X");
	state_add( LH5801_S,  "S",  m_s.w.l ).formatstr("%04X");
	state_add( LH5801_U,  "U",  m_u.w.l ).formatstr("%04X");
	state_add( LH5801_X,  "X",  m_x.w.l ).formatstr("%04X");
	state_add( LH5801_Y,  "Y",  m_y.w.l ).formatstr("%04X");
	state_add( LH5801_T,  "T",  m_t     ).formatstr("%02X");
	state_add( LH5801_A,  "A",  m_a     ).formatstr("%02X");
	state_add( LH5801_TM, "TM", m_tm    ).formatstr("%03X");
	state_add( LH5801_PV, "PV", m_pv    ).formatstr("%04X");
	state_add( LH5801_PU, "PU", m_pu    ).formatstr("%04X");
	state_add( LH5801_BF, "BF", m_bf    ).formatstr("%04X");
	state_add( LH5801_DP, "DP", m_dp    ).formatstr("%04X");

	state_add(STATE_GENPC,  "GENPC",  m_p.w.l).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_p.w.l).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_t).noshow().formatstr("%8s");

	set_icountptr(m_icount);
}

void lh5801_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_t&0x80?'1':'0',
				m_t&0x40?'1':'0',
				m_t&0x20?'1':'0',
				m_t&0x10?'H':'.',
				m_t&0x08?'V':'.',
				m_t&0x04?'Z':'.',
				m_t&0x02?'I':'.',
				m_t&0x01?'C':'.');
			break;
	}
}

void lh5801_cpu_device::device_reset()
{
	P = (m_program.read_byte(0xfffe) << 8) | m_program.read_byte(0xffff);

	m_idle = 0;

	memset(m_ir_flipflop, 0, sizeof(m_ir_flipflop));
	memset(m_lines_status, 0, sizeof(m_lines_status));
}


void lh5801_cpu_device::check_irq()
{
	if (m_ir_flipflop[0])
	{
		//NMI interrupt
		m_ir_flipflop[0] = 0;
		lh5801_push(m_t);
		m_t &= ~IE;
		lh5801_push_word(P);
		P = (m_program.read_byte(0xfffc) << 8) | m_program.read_byte(0xfffd);
	}
	else if (m_ir_flipflop[1] && (m_t & IE))
	{
		// Counter interrupt (counter not yet implemented)
		m_ir_flipflop[1] = 0;
		lh5801_push(m_t);
		m_t &= ~IE;
		lh5801_push_word(P);
		P = (m_program.read_byte(0xfffa) << 8) | m_program.read_byte(0xfffb);
	}
	else if (m_ir_flipflop[2] && (m_t & IE))
	{
		// MI interrupt
		m_ir_flipflop[2] = 0;
		lh5801_push(m_t);
		m_t &= ~IE;
		lh5801_push_word(P);
		P = (m_program.read_byte(0xfff8) << 8) | m_program.read_byte(0xfff9);
	}
}


void lh5801_cpu_device::execute_run()
{
	do
	{
		check_irq();

		if (m_idle)
		{
			debugger_wait_hook();
			m_icount = 0;
		}
		else
		{
			m_oldpc = P;

			debugger_instruction_hook(P);
			lh5801_instruction();
		}

	} while (m_icount > 0);
}

void lh5801_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
		case LH5801_LINE_MI:
			if (m_lines_status[0] == CLEAR_LINE && state == ASSERT_LINE)
			{
				m_idle = 0;
				m_ir_flipflop[2] = 1;
			}

			m_lines_status[0] = state;
			break;
		case INPUT_LINE_NMI:
			if (m_lines_status[1] == CLEAR_LINE && state == ASSERT_LINE)
			{
				m_idle = 0;
				m_ir_flipflop[0] = 1;
			}

			m_lines_status[1] = state;
			break;
	}
}

std::unique_ptr<util::disasm_interface> lh5801_cpu_device::create_disassembler()
{
	return std::make_unique<lh5801_disassembler>();
}
