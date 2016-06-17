// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  mb86901.cpp - Emulation for the Fujitsu MB86901 / LSI L64801
//                processors. Both chips are identical both
//                electrically and functionally, and implement
//                the integer instructions in a SPARC v7
//                compatible instruction set.
//================================================================

#include "emu.h"
#include "debugger.h"
#include "sparc.h"
#include "mb86901defs.h"

CPU_DISASSEMBLE( sparc );

const device_type MB86901 = &device_creator<mb86901_device>;

const int mb86901_device::WINDOW_COUNT = 7;

//-------------------------------------------------
//  mb86901_device - constructor
//-------------------------------------------------

mb86901_device::mb86901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MB86901, "Fujitsu MB86901", tag, owner, clock, "mb86901", __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 32, 32)
{
}


void mb86901_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	// register our state for the debugger
	state_add(STATE_GENPC,		"GENPC",	m_pc).noshow();
	state_add(STATE_GENFLAGS,	"GENFLAGS",	m_psr).callimport().callexport().formatstr("%6s").noshow();
	state_add(SPARC_PC,			"PC",		m_pc).formatstr("%08X");
	state_add(SPARC_NPC,		"nPC",		m_npc).formatstr("%08X");
	state_add(SPARC_PSR,		"PSR",		m_psr).formatstr("%08X");
	state_add(SPARC_WIM,		"WIM",		m_wim).formatstr("%08X");
	state_add(SPARC_TBR,		"TBR",		m_tbr).formatstr("%08X");
	state_add(SPARC_Y,			"Y",		m_y).formatstr("%08X");
	state_add(SPARC_ICC,		"icc",		m_icc).callexport().formatstr("%4s");
	state_add(SPARC_CWP,		"CWP",		m_cwp).formatstr("%2d");
	char regname[3] = "g0";
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_G0 + i, regname, m_r[i]).formatstr("%08X");
	}
	regname[0] = 'o';
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_O0 + i, regname, m_r[8 + i]).callexport().formatstr("%08X");
	}

	regname[0] = 'l';
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_O0 + i, regname, m_r[16 + i]).callexport().formatstr("%08X");
	}
	regname[0] = 'i';
	for (int i = 0; i < 8; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_O0 + i, regname, m_r[24 + i]).callexport().formatstr("%08X");
	}

	state_add(SPARC_EC,		"EC",		m_ec).formatstr("%1d");
	state_add(SPARC_EF,		"EF",		m_ef).formatstr("%1d");
	state_add(SPARC_ET,		"ET",		m_et).formatstr("%1d");
	state_add(SPARC_PIL,	"PIL",		m_pil).formatstr("%2d");
	state_add(SPARC_S,		"S",		m_s).formatstr("%1d");
	state_add(SPARC_PS,		"PS",		m_ps).formatstr("%1d");


	regname[0] = 'r';
	for (int i = 0; i < 120; i++)
	{
		regname[1] = 0x30 + i;
		state_add(SPARC_R0 + i, regname, m_r[i]).formatstr("%08X");
	}

	// register with the savestate system
	save_item(NAME(m_r));
	save_item(NAME(m_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_psr));
	save_item(NAME(m_wim));
	save_item(NAME(m_tbr));
	save_item(NAME(m_y));
	save_item(NAME(m_impl));
	save_item(NAME(m_ver));
	save_item(NAME(m_icc));
	save_item(NAME(m_ec));
	save_item(NAME(m_ef));
	save_item(NAME(m_pil));
	save_item(NAME(m_s));
	save_item(NAME(m_ps));
	save_item(NAME(m_et));
	save_item(NAME(m_cwp));

	// set our instruction counter
	m_icountptr = &m_icount;
}

void mb86901_device::device_stop()
{
}

void mb86901_device::device_reset()
{
	m_pc = 0;
	m_npc = 0x00000004;
	memset(m_r, 0, sizeof(UINT32) * 120);

	m_wim = 0;
	m_tbr = 0;
	m_y = 0;

	m_impl = 0;
	m_ver = 0;
	m_icc = 0;
	m_ec = false;
	m_ef = false;
	m_pil = 0; // double-check this
	m_s = true;
	m_ps = true; // double-check this
	m_et = true; // double-check this
	m_cwp = 0;

	MAKE_PSR;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *mb86901_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	return nullptr;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void mb86901_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	const int total_window_regs = WINDOW_COUNT * 16;
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c", ICC_N_SET ? 'n' : ' ', ICC_Z_SET ? 'z' : ' ', ICC_V_SET ? 'v' : ' ', ICC_C_SET ? 'c' : ' ');
			break;
		case SPARC_O0:	case SPARC_O1:	case SPARC_O2:	case SPARC_O3:	case SPARC_O4:	case SPARC_O5:	case SPARC_O6:	case SPARC_O7:
			str = string_format("%08X", m_r[8 + m_cwp * 16 + (entry.index() - SPARC_O0)]);
			break;
		case SPARC_L0:	case SPARC_L1:	case SPARC_L2:	case SPARC_L3:	case SPARC_L4:	case SPARC_L5:	case SPARC_L6:	case SPARC_L7:
			str = string_format("%08X", m_r[8 + (8 + (m_cwp * 16 + (entry.index() - SPARC_L0)) % total_window_regs)]);
			break;
		case SPARC_I0:	case SPARC_I1:	case SPARC_I2:	case SPARC_I3:	case SPARC_I4:	case SPARC_I5:	case SPARC_I6:	case SPARC_I7:
			str = string_format("%08X", m_r[8 + (16 + (m_cwp * 16 + (entry.index() - SPARC_I0)) % total_window_regs)]);
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 mb86901_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 mb86901_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t mb86901_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( sparc );
	return CPU_DISASSEMBLE_NAME(sparc)(this, buffer, pc, oprom, opram, options);
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 mb86901_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 mb86901_device::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 mb86901_device::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void mb86901_device::execute_set_input(int inputnum, int state)
{
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void mb86901_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(this, m_pc);

		//UINT32 op = read_word(m_pc);

		// TODO

		m_pc = m_npc;
		m_npc += 4;

		--m_icount;
	}
}
