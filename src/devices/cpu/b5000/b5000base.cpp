// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 family MCU cores

This MCU series sits between A4000 and the more publicly available PPS4/1.
Known part numbers: A/B5000, A5300, A/B5500, A/B5900, B6000, B6100.
The latter two were manufactured for Mattel, with small modifications
useful for making handheld games. In fact, the programmer of the first
Mattel handheld games was a circuit designer at Rockwell.

The main difference between Axxxx and Bxxxx is that B runs on low power,
there's also a small change with the way they output LEDs.

A5300 might not be in this series, the page size is 0x3f instead of 0x40.
A4000 is similar, but too many differences to emulate in this device, probably.

*/

#include "emu.h"
#include "b5000base.h"

#include "debugger.h"


b5000_base_device::b5000_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth),
	m_read_kb(*this),
	m_read_din(*this),
	m_write_str(*this),
	m_write_seg(*this),
	m_write_spk(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void b5000_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks
	m_read_kb.resolve_safe(0);
	m_read_din.resolve_safe(0);
	m_write_str.resolve_safe();
	m_write_seg.resolve_safe();
	m_write_spk.resolve_safe();

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_s = 0;
	m_op = 0;
	m_prev_op = 0;

	m_a = 0;
	m_bl = 0;
	m_bu = 0;
	m_prev_bl = 0;
	m_prev_bu = 0;
	m_bl_delay = false;
	m_bu_delay = false;
	m_ram_addr = 0;
	m_c = 0;
	m_prev_c = 0;
	m_prev2_c = 0;
	m_sr = false;
	m_skip = false;
	m_seg = 0;
	m_suppress0 = false;

	m_atbz_step = 0;
	m_tra_step = 0;
	m_ret_step = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_s));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));

	save_item(NAME(m_a));
	save_item(NAME(m_bl));
	save_item(NAME(m_bu));
	save_item(NAME(m_prev_bl));
	save_item(NAME(m_prev_bu));
	save_item(NAME(m_bl_delay));
	save_item(NAME(m_bu_delay));
	save_item(NAME(m_ram_addr));
	save_item(NAME(m_c));
	save_item(NAME(m_prev_c));
	save_item(NAME(m_prev2_c));
	save_item(NAME(m_sr));
	save_item(NAME(m_skip));
	save_item(NAME(m_seg));
	save_item(NAME(m_suppress0));

	save_item(NAME(m_atbz_step));
	save_item(NAME(m_tra_step));
	save_item(NAME(m_ret_step));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prev_pc).formatstr("%03X").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%03X"); // 1
	state_add(++m_state_count, "S", m_s).formatstr("%03X"); // 2
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 3
	state_add(++m_state_count, "C", m_c).formatstr("%01X"); // 4
	state_add(++m_state_count, "B", m_ram_addr).formatstr("%02X"); // 5
	state_add(++m_state_count, "BU", m_bu).formatstr("%01X").noshow(); // 6
	state_add(++m_state_count, "BL", m_bl).formatstr("%01X").noshow(); // 7

	set_icountptr(m_icount);
}

device_memory_interface::space_config_vector b5000_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void b5000_base_device::device_reset()
{
	reset_pc();
	m_prev_pc = m_pc;
	m_s = m_pc;
	m_op = 0;
	m_prev_op = 0;

	m_bl_delay = false;
	m_bu_delay = false;
	m_sr = false;
	m_skip = false;

	m_atbz_step = 0;
	m_tra_step = 0;
	m_ret_step = 0;
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void b5000_base_device::increment_pc()
{
	// low part is LFSR
	int feed = ((m_pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (m_pc >> 1 ^ m_pc) & 1;
	m_pc = (m_pc & ~0x3f) | (m_pc >> 1 & 0x1f) | (feed << 5);
}

void b5000_base_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		m_prev_bl = m_bl;
		m_prev_bu = m_bu;
		m_prev2_c = m_prev_c;
		m_prev_c = m_c;

		// fetch next opcode
		m_op = m_program->read_byte(m_pc);
		bool skip = m_skip && op_canskip(m_op);
		m_skip = false;

		if (!skip)
			debugger_instruction_hook(m_pc);
		increment_pc();
		m_icount--;

		// handle opcode if it's not skipped
		if (skip)
			m_op = 0; // fake nop
		else
			execute_one();

		// some opcodes have multiple steps and will run in parallel with next ones,
		// eg. it may fetch in order A,B and parts executed in order B,A
		if (m_atbz_step) op_atbz();
		if (m_tra_step) op_tra();
		if (m_ret_step) op_ret();

		// some opcodes delay RAM address adjustment for 1 cycle
		m_ram_addr = (m_bu << 4 & 0x30) | (m_bl & 0xf);

		if (m_bl_delay)
		{
			m_ram_addr = (m_ram_addr & ~0xf) | (m_prev_bl & 0xf);
			m_bl_delay = false;
		}
		if (m_bu_delay)
		{
			m_ram_addr = (m_ram_addr & 0xf) | (m_prev_bu << 4 & 0x30);
			m_bu_delay = false;
		}
	}
}
