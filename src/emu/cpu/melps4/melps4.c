// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family cores

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
		case STATE_GENFLAGS:
			break;

		default: break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void melps4_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// zerofill
	m_pc = 0;

	// register for savestates
	save_item(NAME(m_pc));

	// register state for debugger

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void melps4_cpu_device::device_reset()
{
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void melps4_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);

		// handle opcode

	}
}
