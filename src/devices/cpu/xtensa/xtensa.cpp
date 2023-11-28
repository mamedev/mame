// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Tensilica Xtensa

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "xtensa.h"
#include "xtensad.h"

// device type definitions
DEFINE_DEVICE_TYPE(XTENSA, xtensa_device, "xtensa", "Tensilica Xtensa core")

xtensa_device::xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, XTENSA, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_pc(0)
{
}

std::unique_ptr<util::disasm_interface> xtensa_device::create_disassembler()
{
	return std::make_unique<xtensa_disassembler>();
}

device_memory_interface::space_config_vector xtensa_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config),
	};
}

void xtensa_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	std::fill(std::begin(m_a), std::end(m_a), 0);

	set_icountptr(m_icount);

	state_add(XTENSA_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc);
	state_add(STATE_GENPCBASE, "CURPC", m_pc);
	for (int i = 0; i < 16; i++)
		state_add(XTENSA_A0 + i, string_format("a%d", i).c_str(), m_a[i]);

	save_item(NAME(m_pc));
	save_item(NAME(m_a));
}

void xtensa_device::device_reset()
{
	// TODO: Reset state
}

void xtensa_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void xtensa_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
