// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "sonix16.h"

#include "sonix16d.h"


DEFINE_DEVICE_TYPE(SONIX16, sonix16_device, "sonix16", "Sonix 16-bit DSP")

sonix16_device::sonix16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SONIX16, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, -1)
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 24, -1)
	, m_pc(0)
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> sonix16_device::create_disassembler()
{
	return std::make_unique<sonix16_disassembler>();
}

device_memory_interface::space_config_vector sonix16_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void sonix16_device::device_start()
{
	state_add(STATE_GENPC,     "PC",    m_pc);
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	set_icountptr(m_icount);
}

void sonix16_device::device_reset()
{
	m_pc = 0x400000;
}

void sonix16_device::execute_set_input(int irqline, int state)
{
}

void sonix16_device::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}
