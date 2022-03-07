// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor COPS(MM57 MCU family) cores

This is the first "COPS" series (Controller Oriented Processor Systems),
4-bit MCUs with internal RAM and most of them internal ROM too.
It was only briefly on the market and was quickly superceded by the
2nd "COPS": the COP400 series.

Short list of MCU types:
- MM5781+MM5782: 2KB ROM, 160 nibbles RAM
- MM5799: 1.5KB ROM, 96 nibbles RAM
- MM57140: 640 bytes ROM(10 bytes inaccessible?), 55 nibbles RAM

Note that not every "MM57" chip is a generic MCU, there are plenty other chips,
mostly for calculators. For example MM5780 for the Quiz Kid, the decap of that
looks more like a complex state machine.

References:
- 1977 National Semiconductor MOS/LSI databook

TODO:
- documentation says that LB 10 is either 0 or 4, depending on RAM configuration,
  but on qkracerm it's 5 (also confirmed in patent source code), so I assume
  LB 10 is fully configurable as mask option
- MM5799 RAM layout is derived from MCU decap, documentation suggests that the
  secondary option is literally 6x16 but according to the decap it's 4x16 + 4x8

*/

#include "emu.h"
#include "cops1base.h"

#include "debugger.h"


cops1_base_device::cops1_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program),
	m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data),
	m_prgwidth(prgwidth),
	m_datawidth(datawidth),
	m_opla(*this, "opla"),
	m_read_k(*this),
	m_read_inb(*this),
	m_read_f(*this),
	m_write_f(*this),
	m_read_do3(*this),
	m_write_do(*this),
	m_write_s(*this),
	m_write_blk(*this),
	m_read_si(*this),
	m_write_so(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	COPS1_PC=1, COPS1_SA, COPS1_SB,
	COPS1_A, COPS1_C, COPS1_H, COPS1_B, COPS1_F
};

void cops1_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks
	m_read_k.resolve_safe(0);
	m_read_inb.resolve_safe(0);
	m_read_f.resolve();
	m_write_f.resolve_safe();
	m_read_do3.resolve();
	m_write_do.resolve_safe();
	m_write_s.resolve_safe();
	m_write_blk.resolve_safe();
	m_read_si.resolve_safe(0);
	m_write_so.resolve_safe();

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_arg = 0;

	m_a = 0;
	m_h = 0;
	m_b = 0;
	m_c = 0;
	m_skip = false;
	m_sa = 0;
	m_sb = 0;
	m_serial = 0;
	m_f = 0;
	m_do = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_arg));

	save_item(NAME(m_a));
	save_item(NAME(m_h));
	save_item(NAME(m_b));
	save_item(NAME(m_c));
	save_item(NAME(m_skip));
	save_item(NAME(m_sa));
	save_item(NAME(m_sb));
	save_item(NAME(m_serial));
	save_item(NAME(m_f));
	save_item(NAME(m_do));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prev_pc).formatstr("%03X").noshow();

	state_add(COPS1_PC, "PC", m_pc).formatstr("%03X");
	state_add(COPS1_SA, "SA", m_sa).formatstr("%03X");
	state_add(COPS1_SB, "SB", m_sb).formatstr("%03X");

	state_add(COPS1_A, "A", m_a).formatstr("%01X");
	state_add(COPS1_C, "C", m_c).formatstr("%01X");
	state_add(COPS1_H, "H", m_h).formatstr("%01X");
	state_add(COPS1_B, "B", m_b).formatstr("%02X");
	state_add(COPS1_F, "F", m_f).formatstr("%01X");

	set_icountptr(m_icount);
}

device_memory_interface::space_config_vector cops1_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cops1_base_device::device_reset()
{
	m_op = m_prev_op = 0;
	m_pc = m_prev_pc = 0;
	m_skip = false;

	// clear outputs
	m_write_blk(1);
	m_write_f(m_f = 0);
	m_write_do(m_do = 0);
	m_write_s(0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cops1_base_device::device_add_mconfig(machine_config &config)
{
	PLA(config, "opla", 4, 7, 15).set_format(pla_device::FMT::BERKELEY);
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void cops1_base_device::cycle()
{
	m_icount--;

	// shift serial data
	m_write_so(m_serial & 1);
	int feed = m_option_axo_si ? 1 : m_read_si();
	m_serial = (m_serial >> 1 | feed << 3) & 0xf;
}

void cops1_base_device::increment_pc()
{
	// low part is LFSR
	int feed = ((m_pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (m_pc >> 1 ^ m_pc) & 1;
	m_pc = (m_pc & ~0x3f) | (m_pc >> 1 & 0x1f) | (feed << 5);
}

void cops1_base_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// BLK goes low for 1 cycle with BTD
		if (m_prev_op == 0x25)
			m_write_blk(0);

		// fetch next opcode
		if (!m_skip)
			debugger_instruction_hook(m_pc);
		m_op = m_program->read_byte(m_pc);
		increment_pc();
		cycle();

		if (m_op != 0x25)
			m_write_blk(1);

		// fetch opcode argument
		if (op_argument())
		{
			m_arg = m_program->read_byte(m_pc);
			increment_pc();
			cycle();
		}

		// handle opcode if it's not skipped
		if (m_skip)
		{
			m_skip = false;
			m_op = 0; // fake nop
		}
		else
			execute_one();
	}
}
