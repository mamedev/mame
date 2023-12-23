// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "ht1130.h"
#include "ht1130d.h"

#define LOG_UNHANDLED_OPS       (1U << 1)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(HT1130, ht1130_device, "ht1130", "Holtek HT1130 core")

ht1130_device::ht1130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, HT1130, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 8, 12, 0, address_map_constructor(FUNC(ht1130_device::internal_map), this))
	, m_extregs_config("data", ENDIANNESS_LITTLE, 8, 8, 0)
	, m_pc(0)
{
}

std::unique_ptr<util::disasm_interface> ht1130_device::create_disassembler()
{
	return std::make_unique<ht1130_disassembler>();
}

device_memory_interface::space_config_vector ht1130_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_DATA, &m_extregs_config)
	};
}

void ht1130_device::internal_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void ht1130_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	set_icountptr(m_icount);

	state_add(HT1130_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	save_item(NAME(m_pc));
	save_item(NAME(m_icount));
}

void ht1130_device::device_reset()
{
	m_pc = 0;
}

void ht1130_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		m_icount--;
	}
}

void ht1130_device::execute_set_input(int inputnum, int state)
{
}
