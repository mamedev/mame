// license:BSD-3-Clause
// copyright-holders:David Haywood

// https://apps.dtic.mil/sti/tr/pdf/ADA181619.pdf

#include "emu.h"
#include "mipsx.h"
#include "mipsxdasm.h"

DEFINE_DEVICE_TYPE(MIPSX, mipsx_cpu_device, "mipsx", "MIPS-X")

mipsx_cpu_device::mipsx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, MIPSX, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0)
	, m_pc(0)
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
	m_program = &space(AS_PROGRAM);

	state_add(STATE_GENPC,      "GENPC",     m_pc).formatstr("%08X");
	state_add(STATE_GENPCBASE,  "CURPC",     m_pc).callexport().noshow();

	set_icountptr(m_icount);
}

void mipsx_cpu_device::device_reset()
{
}

/*****************************************************************************/

void mipsx_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		m_pc += 4;
		m_icount--;
	}
}
