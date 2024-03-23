// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita (Panasonic) MN1400 family MCU cores

4-bit microcontroller introduced in 1977, possibly Matsushita's first MCU.

Basic MN1400 series:

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

Other models:

MN1450B: internal FLT driver
MN1456A: MN1455 with double amount ROM/RAM
MN148x: DAC for TV/VTR tuner
MN1427: support for FM audio tuner

TODO:
- counter input pin (CSLCT and SNS1)
- are illegal opcodes 0x38/0xe0 and 0x39/0xe1 branch-always and branch-never?
  right now they're implemented as such
- is branch emulation correct when near the end of a page?
- add other MCUs when needed

*/

#include "emu.h"
#include "mn1400base.h"


mn1400_base_device::mn1400_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_opla(*this, "opla"),
	m_stack_levels(stack_levels),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth),
	m_read_a(*this, 0),
	m_read_b(*this, 0),
	m_read_sns(*this, 0),
	m_write_c(*this),
	m_write_d(*this),
	m_write_e(*this),
	m_c_mask(0xfff),
	m_d_mask(0xff),
	m_d_bits(0)
{ }


//-------------------------------------------------
//  disasm
//-------------------------------------------------

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
//  device_start - device-specific startup
//-------------------------------------------------

void mn1400_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;
	m_ram_address = 0;
	memset(m_stack, 0, sizeof(m_stack));
	m_sp = 0;

	m_a = 0;
	m_x = 0;
	m_y = 0;
	m_status = 0;
	m_c = 0;
	m_counter = 0;
	m_ec = false;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_param));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_stack));
	save_item(NAME(m_sp));

	save_item(NAME(m_a));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_status));
	save_item(NAME(m_c));
	save_item(NAME(m_counter));
	save_item(NAME(m_ec));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prev_pc).formatstr("%03X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_status).formatstr("%3s").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%03X"); // 1
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 2
	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 3
	state_add(++m_state_count, "Y", m_y).formatstr("%01X"); // 4
	state_add(++m_state_count, "CNT", m_counter).formatstr("%02X"); // 5

	set_icountptr(m_icount);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mn1400_base_device::device_reset()
{
	m_pc = m_prev_pc = 0;
	m_op = m_prev_op = 0;
	m_status = 0;
	m_ec = false;

	// clear output ports
	write_c(0);
	write_d(0);
	m_write_e(0);
}


//-------------------------------------------------
//  I/O
//-------------------------------------------------

void mn1400_base_device::device_add_mconfig(machine_config &config)
{
	PLA(config, m_opla, 5, 8, 24).set_format(pla_device::FMT::BERKELEY);
}

void mn1400_base_device::write_d(u8 data)
{
	u8 output = m_opla->read(data);
	output = bitswap<8>(output,7,6,5,3,1,2,0,4);

	// DO outputs may be bonded to different DO pins
	if (u32 b = m_d_bits)
		output = bitswap<8>(output, b >> 28 & 7, b >> 24 & 7, b >> 20 & 7, b >> 16 & 7, b >> 12 & 7, b >> 8 & 7, b >> 4 & 7, b >> 0 & 7);

	m_write_d(~output & m_d_mask);
}

void mn1400_base_device::write_c(u16 data)
{
	m_c = data & m_c_mask;
	m_write_c(m_c);
}


//-------------------------------------------------
//  common internal memory maps
//-------------------------------------------------

device_memory_interface::space_config_vector mn1400_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

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

		m_ram_address = (m_x << 4 | m_y) & m_datamask;
		execute_one();
	}
}
