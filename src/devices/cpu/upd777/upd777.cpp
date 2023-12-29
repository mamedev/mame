// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "upd777.h"
#include "upd777dasm.h"

#define LOG_UNHANDLED_OPS       (1U << 1)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(UPD777, upd777_device, "upd777", "uPD777")



upd777_device::upd777_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 16, 11, -1, address_map_constructor(FUNC(upd777_device::internal_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 8, 0, data)
{
}

upd777_device::upd777_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd777_device(mconfig, UPD777, tag, owner, clock, address_map_constructor(FUNC(upd777_device::internal_data_map), this))
{
}

std::unique_ptr<util::disasm_interface> upd777_device::create_disassembler()
{
	return std::make_unique<upd777_disassembler>();
}

device_memory_interface::space_config_vector upd777_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void upd777_device::internal_map(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void upd777_device::internal_data_map(address_map &map)
{
}

void upd777_device::device_start()
{
	space(AS_PROGRAM).specific(m_space);
	space(AS_DATA).specific(m_data);

	set_icountptr(m_icount);

	state_add(UPD777_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	upd777_disassembler::populate_addr_table(m_table);

	save_item(NAME(m_pc));
	save_item(NAME(m_icount));
}

void upd777_device::device_reset()
{
	m_pc = 0;
}

u16 upd777_device::fetch()
{
	return m_space.read_word(m_pc++);
}

void upd777_device::do_op()
{
	const u16 inst = fetch();

	switch (inst)
	{
	default:
	{
		LOGMASKED(LOG_UNHANDLED_OPS, "<ill %04x>", inst);
		return;
	}
	}
}

void upd777_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);
		do_op();
		m_icount--;
	}
}

void upd777_device::execute_set_input(int inputnum, int state)
{
}
