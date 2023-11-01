// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "evo.h"
#include "evod.h"

DEFINE_DEVICE_TYPE(EVOLUTION_CPU, evo_cpu_device, "evo_cpu", "Evolution CPU")

evo_cpu_device::evo_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, EVOLUTION_CPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, -1)
	, m_pc(0), m_program(nullptr), m_icount(0), m_debugger_temp(0)
{
}

std::unique_ptr<util::disasm_interface> evo_cpu_device::create_disassembler()
{
	return std::make_unique<evolution_disassembler>();
}

device_memory_interface::space_config_vector evo_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void evo_cpu_device::device_start()
{
	m_pc = 0;
	m_debugger_temp = 0;
	m_program = &space(AS_PROGRAM);
	state_add(EVO_PC,  "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callimport().callexport().noshow();
	set_icountptr(m_icount);
}

void evo_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case EVO_PC:
		case STATE_GENPCBASE:
			m_debugger_temp = m_pc;
			break;
	}
}

void evo_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case EVO_PC:
		case STATE_GENPCBASE:
			m_pc = (m_debugger_temp & 0xffffffff);
			break;
	}
}

void evo_cpu_device::device_reset()
{
	m_pc = 0x400000;
}

void evo_cpu_device::execute_set_input(int irqline, int state)
{
}

void evo_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		m_pc++;
		m_icount--;
	}
}
