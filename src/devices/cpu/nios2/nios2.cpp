// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Altera Nios II soft processor

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "nios2.h"
#include "nios2dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(NIOS2, nios2_device, "nios2", "Altera Nios II Processor")

nios2_device::nios2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, NIOS2, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
{
	std::fill(std::begin(m_gpr), std::end(m_gpr), 0);
}

std::unique_ptr<util::disasm_interface> nios2_device::create_disassembler()
{
	return std::make_unique<nios2_disassembler>();
}

device_memory_interface::space_config_vector nios2_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void nios2_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);

	set_icountptr(m_icount);

	state_add(NIOS2_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	for (int i = 1; i < 32; i++)
		state_add(NIOS2_R1 + i - 1, util::string_format("r%d", i).c_str(), m_gpr[i]);

	save_item(NAME(m_gpr));
	save_item(NAME(m_pc));
}

void nios2_device::device_reset()
{
	m_pc = 0;
}

void nios2_device::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}
