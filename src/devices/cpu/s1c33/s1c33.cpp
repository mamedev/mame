// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "s1c33.h"
#include "s1c33dasm.h"

#define LOG_UNHANDLED_OPS       (1U << 1)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(S1C33, s1c33_device, "s1c33", "Seiko Epson S1C33 family CPU core")

s1c33_device::s1c33_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, S1C33, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_pc(0)
{
}

std::unique_ptr<util::disasm_interface> s1c33_device::create_disassembler()
{
	return std::make_unique<s1c33_disassembler>();
}

device_memory_interface::space_config_vector s1c33_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
	};
}

void s1c33_device::device_start()
{
	space(AS_PROGRAM).specific(m_space);

	set_icountptr(m_icount);

	state_add(S1C33_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	save_item(NAME(m_pc));
}

void s1c33_device::device_reset()
{
}

void s1c33_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
	}
}

void s1c33_device::execute_set_input(int inputnum, int state)
{
}
