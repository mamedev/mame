// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 family MCU cores

This MCU series sits between A4000 and the more publicly available PPS4/1.
Known part numbers: A/B5000, A/B5300, A/B5500, A/B5900, B6000, B6100.
The latter two were manufactured for Mattel, with small modifications
useful for making handheld games.

The main difference between Axxxx and Bxxxx is that B runs on low power,
there's also a small change with the way they output LEDs.

A4000 is similar, but too many differences to emulate in this device.

*/

#include "emu.h"
#include "b5000base.h"

#include "debugger.h"


b5000_base_device::b5000_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	B5000_PC=1
};

void b5000_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks
	//..

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_skip = false;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_skip));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prev_pc).formatstr("%03X").noshow();

	state_add(B5000_PC, "PC", m_pc).formatstr("%03X");

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
	m_pc = m_prev_pc = 0;
	m_skip = false;
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void b5000_base_device::cycle()
{
	m_icount--;
}

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
		debugger_instruction_hook(m_pc);
		increment_pc();
		cycle();

		execute_one();
	}
}
