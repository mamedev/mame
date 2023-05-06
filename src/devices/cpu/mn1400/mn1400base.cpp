// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita (Panasonic) MN1400 family MCU cores

4-bit microcontroller introduced in 1977, possibly Matsushita's first MCU.

Basic models:

MN1400: 1KB ROM, 64 nibbles RAM
MN1402: 768 bytes ROM, 32 nibbles RAM
MN1403: 0.5KB ROM, 16 nibbles RAM, 18 pins
MN1404: 0.5KB ROM, 16 nibbles RAM, 16 pins
MN1405: 2KB ROM, 128 nibbles RAM

MN1420: MN1400 with LED driver
MN1421: 28-pin version of MN1420
MN1425: MN1405 with LED driver

MN1430: high-voltage version of MN1400
MN1435: high-voltage version of MN1405

MN1450/MN1460: CMOS version of MN1400
MN1455/MN1465: CMOS version of MN1405

*/

#include "emu.h"
#include "mn1400base.h"


mn1400_base_device::mn1400_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_stack_levels(stack_levels),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mn1400_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;

	m_a = 0;
	m_x = 0;
	m_y = 0;
	m_status = 0;
	m_counter = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_param));

	save_item(NAME(m_a));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_status));
	save_item(NAME(m_counter));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prev_pc).formatstr("%03X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_status).formatstr("%3s").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%03X"); // 1
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 2
	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 3
	state_add(++m_state_count, "Y", m_y).formatstr("%01X"); // 4
	state_add(++m_state_count, "CNT", m_y).formatstr("%02X"); // 5

	set_icountptr(m_icount);
}

device_memory_interface::space_config_vector mn1400_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

void mn1400_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c",
				(m_status & FLAG_P) ? 'P' : 'p',
				(m_status & FLAG_C) ? 'C' : 'c',
				(m_status & FLAG_Z) ? 'Z' : 'z'
			);
			break;
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mn1400_base_device::device_reset()
{
	m_pc = m_prev_pc = 0;
	m_op = m_prev_op = 0;
	m_param = 0;
}


//-------------------------------------------------
//  common internal memory maps
//-------------------------------------------------

void mn1400_base_device::program_1kx8(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void mn1400_base_device::program_2kx8(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void mn1400_base_device::data_64x4(address_map &map)
{
	map(0x00, 0x3f).ram();
}

void mn1400_base_device::data_128x4(address_map &map)
{
	map(0x00, 0x7f).ram();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void mn1400_base_device::cycle()
{
	m_icount--;
}

void mn1400_base_device::increment_pc()
{
	m_pc = (m_pc + 1) & m_prgmask;
}

void mn1400_base_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// fetch next opcode
		m_op = m_program->read_byte(m_pc);
		debugger_instruction_hook(m_pc);
		increment_pc();
		cycle();

		// 2-byte opcodes
		if (op_has_param(m_op))
		{
			m_param = m_program->read_byte(m_pc);
			increment_pc();
			cycle();
		}

		execute_one();
	}
}
