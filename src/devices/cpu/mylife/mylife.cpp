// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton CPU device.

***************************************************************************/

#include "emu.h"
#include "mylife.h"
#include "mylifed.h"

DEFINE_DEVICE_TYPE(MYLIFE_CPU, mylife_cpu_device, "mylife_cpu", "My Life CPU")

mylife_cpu_device::mylife_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, MYLIFE_CPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 22, -1)
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 16, -1)
{
}

device_memory_interface::space_config_vector mylife_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> mylife_cpu_device::create_disassembler()
{
	return std::make_unique<mylife_disassembler>();
}

void mylife_cpu_device::device_start()
{
	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_pc).noshow();

	save_item(NAME(m_pc));
}

void mylife_cpu_device::device_reset()
{
	m_pc = 0;
}

void mylife_cpu_device::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}
