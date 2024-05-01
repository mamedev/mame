// license:BSD-3-Clause
// copyright-holders:David Haywood

// https://apps.dtic.mil/sti/tr/pdf/ADA181619.pdf

#include "emu.h"
#include "mipsx.h"
#include "mipsxdasm.h"

DEFINE_DEVICE_TYPE(MIPSX, mipsx_cpu_device, "mipsx", "MIPS-X")

mipsx_cpu_device::mipsx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, MIPSX, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 24, 0)
	, m_pc(0)
	, m_debugger_temp(0)
	, m_program(nullptr)
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> mipsx_cpu_device::create_disassembler()
{
	return std::make_unique<mipsx_disassembler>();
}

/*****************************************************************************/

device_memory_interface::space_config_vector mipsx_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


/*****************************************************************************/

void mipsx_cpu_device::device_start()
{
	m_pc = 0;

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);

	state_add(MIPSX_PC,  "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callimport().callexport().noshow();

	set_icountptr(m_icount);
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void mipsx_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case MIPSX_PC:
		case STATE_GENPCBASE:
			m_debugger_temp = m_pc << 2;
			break;
	}
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void mipsx_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case MIPSX_PC:
		case STATE_GENPCBASE:
			m_pc = (m_debugger_temp & 0xfffffffc) >> 2;
			break;
	}
}

void mipsx_cpu_device::device_reset()
{
	m_pc = 0x00000000;
}

/*****************************************************************************/

void mipsx_cpu_device::execute_set_input(int irqline, int state)
{
}


void mipsx_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc<<2);
		m_pc++;
		m_icount--;
	}
}
