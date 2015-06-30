// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family cores

  References:
  - 1990 Sharp Microcomputers Data Book

*/

#include "sm510.h"
#include "debugger.h"

#include "sm510op.inc"

// MCU types

const device_type SM510 = &device_creator<sm510_device>;


// internal memory maps
static ADDRESS_MAP_START(program_2_7k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x0000, 0x02af) AM_ROM
	AM_RANGE(0x0400, 0x06af) AM_ROM
	AM_RANGE(0x0800, 0x0aaf) AM_ROM
	AM_RANGE(0x0c00, 0x0eaf) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_96_32x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x5f) AM_RAM
	AM_RANGE(0x60, 0x7f) AM_RAM
ADDRESS_MAP_END



// device definitions

sm510_device::sm510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm510_base_device(mconfig, SM510, "SM510", tag, owner, clock, 2 /* stack levels */, 12 /* prg width */, ADDRESS_MAP_NAME(program_2_7k), 7 /* data width */, ADDRESS_MAP_NAME(data_96_32x4), "sm510", __FILE__)
{ }



// disasm
void sm510_base_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	#if 0
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c",
				m_c ? 'C':'c',
				m_s ? 'S':'s'
			);
			break;

		default: break;
	}
	#endif
}

offs_t sm510_base_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(sm510);
	return CPU_DISASSEMBLE_NAME(sm510)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	SM510_PC=1, SM510_ACC, SM510_BL, SM510_BM
};

void sm510_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks
	//..

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;
	m_acc = 0;
	m_bl = 0;
	m_bm = 0;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_param));
	save_item(NAME(m_acc));
	save_item(NAME(m_bl));
	save_item(NAME(m_bm));

	// register state for debugger
	state_add(SM510_PC,  "PC",  m_pc).formatstr("%04X");
	state_add(SM510_ACC, "ACC", m_acc).formatstr("%01X");
	state_add(SM510_BL,  "BL",  m_bl).formatstr("%01X");
	state_add(SM510_BM,  "BM",  m_bm).formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_pc).formatstr("%2s").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm510_base_device::device_reset()
{
}






//-------------------------------------------------
//  execute
//-------------------------------------------------

inline void sm510_base_device::increment_pc()
{
	m_pc = (m_pc + 1) & m_prgmask;
}

void sm510_base_device::get_opcode_param()
{
	// LBL, TL, TML, TM opcodes are 2 bytes
	if (m_op == 0x5f || (m_op & 0xf0) == 0x70 || m_op >= 0xc0)
	{
		m_icount -= 2; // guessed
		m_param = m_program->read_byte(m_pc);
		increment_pc();
	}
}

void sm510_base_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_icount -= 2; // 61us typical
		m_op = m_program->read_byte(m_pc);
		increment_pc();
		get_opcode_param();

		// handle opcode

	}
}
