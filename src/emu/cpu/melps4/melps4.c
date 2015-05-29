// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family cores

  Known types and their features:
  (* means not emulated yet)

 *M58840: 42-pin DIL, 2Kx9 ROM, 128x4 RAM, A/D converter
 *M58841: almost same as M58840
 *M58842: 64-pin DIL, external ROM(11-bit PC), rest is same as M58840
 *M58843: 28-pin DIL, 1Kx9 ROM, 64x4 RAM, A/D converter
 *M58844: almost same as M58843
 *M58845: 42-pin DIL, 2Kx9 ROM, 128x4 RAM, A/D converter, 2 timers
  M58846: 42-pin DIL, 2Kx9 ROM, 128x4 RAM, 2 timers(not same as M58845), extra I/O ports
 *M58847: 40-pin DIL, 2Kx9 ROM, 128x4 RAM, extra I/O ports(not same as M58846)
 *M58848: ? (couldn't find info, just that it exists)

  MELPS 41/42 subfamily:

 *M58494: 72-pin QFP CMOS, 4Kx10 ROM, 32x4 internal + 4Kx4 external RAM, 2 timers
 *M58496: 72-pin QFP CMOS, 2Kx10 ROM, 128x4 internal + 256x4 external RAM, 1 timer, low-power
 *M58497: almost same as M58496

  MELPS 760 subfamily has more differences, document them when needed.


  References:
  - 1982 Mitsubishi LSI Data Book

*/

#include "melps4.h"
#include "debugger.h"

#include "melps4op.inc"


// disasm
void melps4_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		// obviously not from a single flags register, letters are made up
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c",
				m_intp ? 'P':'p',
				m_inte ? 'I':'i',
				m_cps  ? 'D':'d',
				m_cy   ? 'C':'c'
			);
			break;

		default: break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	MELPS4_PC=1, MELPS4_A, MELPS4_B, MELPS4_E,
	MELPS4_Y, MELPS4_X, MELPS4_Z,
	MELPS4_H, MELPS4_L, MELPS4_C, MELPS4_V, MELPS4_W
};

void melps4_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	memset(m_stack, 0, sizeof(m_stack));
	m_op = 0;
	m_prev_op = 0;
	m_bitmask = 0;

	m_cps = 0;
	m_skip = false;
	m_inte = 0;
	m_intp = 1;

	m_a = 0;
	m_b = 0;
	m_e = 0;
	m_y = m_y2 = 0;
	m_x = m_x2 = 0;
	m_z = m_z2 = 0;
	m_cy = m_cy2 = 0;

	m_h = 0;
	m_l = 0;
	m_c = 7;
	m_v = 0;
	m_w = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_stack));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_bitmask));

	save_item(NAME(m_cps));
	save_item(NAME(m_skip));
	save_item(NAME(m_inte));
	save_item(NAME(m_intp));

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_e));
	save_item(NAME(m_y)); save_item(NAME(m_y2));
	save_item(NAME(m_x)); save_item(NAME(m_x2));
	save_item(NAME(m_z)); save_item(NAME(m_z2));
	save_item(NAME(m_cy)); save_item(NAME(m_cy2));

	save_item(NAME(m_h));
	save_item(NAME(m_l));
	save_item(NAME(m_c));
	save_item(NAME(m_v));
	save_item(NAME(m_w));

	// register state for debugger
	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_cy).formatstr("%4s").noshow();

	state_add(MELPS4_PC, "PC", m_pc).formatstr("%04X");
	state_add(MELPS4_A, "A", m_a).formatstr("%2d"); // show in decimal
	state_add(MELPS4_B, "B", m_b).formatstr("%2d"); // "
	state_add(MELPS4_E, "E", m_e).formatstr("%02X");
	state_add(MELPS4_Y, "Y", m_y).formatstr("%1X");
	state_add(MELPS4_X, "X", m_x).formatstr("%1d");
	state_add(MELPS4_Z, "Z", m_z).formatstr("%1d");

	state_add(MELPS4_H, "H", m_h).formatstr("%1X");
	state_add(MELPS4_L, "L", m_l).formatstr("%1X");
	state_add(MELPS4_C, "C", m_c).formatstr("%1X");
	state_add(MELPS4_V, "V", m_v).formatstr("%1X");
	state_add(MELPS4_W, "W", m_w).formatstr("%1X");

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void melps4_cpu_device::device_reset()
{
	m_op = m_prev_op = 0;
	m_pc = m_prev_pc = 0;
	m_inte = 0;
	m_intp = 1;
	op_lcps(); // CPS=0

	m_v = 0;
	m_w = 0;
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void melps4_cpu_device::execute_one()
{
	// B is at $18x and BM is at $10x for all MCU types
	if (m_op >= 0x180)
		op_b();
	else if (m_op >= 0x100)
		op_bm();
	else
		op_illegal();
}

void melps4_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_icount--;
		m_op = m_program->read_word(m_pc << 1) & 0x1ff;
		m_bitmask = 1 << (m_op & 3);
		m_pc = (m_pc & ~0x7f) | ((m_pc + 1) & 0x7f); // stays in the same page

		// handle opcode if it's not skipped
		execute_one();
	}
}
