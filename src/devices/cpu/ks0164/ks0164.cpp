// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// KS0164 core

#include "emu.h"
#include "ks0164.h"
#include "ks0164d.h"
#include "debugger.h"

DEFINE_DEVICE_TYPE(KS0164, ks0164_device, "ks0164", "Samsung KS0164 audio processor")

ks0164_device::ks0164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, KS0164, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 16)
{
}

void ks0164_device::device_start()
{
	state_add(STATE_GENPC,     "GENPC",     m_pc).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).callexport().noshow();

	save_item(NAME(m_pc));
	set_icountptr(m_icount);

	m_pc = 0;
}

void ks0164_device::device_reset()
{
	m_pc = 0x780;
}

uint32_t ks0164_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t ks0164_device::execute_max_cycles() const noexcept
{
	return 1;
}

uint32_t ks0164_device::execute_input_lines() const noexcept
{
	return 0;
}

void ks0164_device::execute_set_input(int inputnum, int state)
{
}

device_memory_interface::space_config_vector ks0164_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> ks0164_device::create_disassembler()
{
	return std::make_unique<ks0164_disassembler>();
}

void ks0164_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

void ks0164_device::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}
