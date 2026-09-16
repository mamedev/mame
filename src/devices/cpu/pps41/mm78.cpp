// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM77/MM78 MCU

*/

#include "emu.h"
#include "mm78.h"

#include "pps41d.h"


DEFINE_DEVICE_TYPE(MM78, mm78_device, "mm78", "Rockwell MM78 A7800") // 2KB ROM, 128 nibbles RAM
DEFINE_DEVICE_TYPE(MM78L, mm78l_device, "mm78l", "Rockwell MM78L B7800") // low-power
DEFINE_DEVICE_TYPE(MM77, mm77_device, "mm77", "Rockwell MM77 A7700") // 1.3KB ROM, 96 nibbles RAM
DEFINE_DEVICE_TYPE(MM77L, mm77l_device, "mm77l", "Rockwell MM77L B7700") // 1.5KB ROM, low-power


// constructor
mm78_device::mm78_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm78_device(mconfig, MM78, tag, owner, clock, 11, address_map_constructor(FUNC(mm78_device::program_2k), this), 7, address_map_constructor(FUNC(mm78_device::data_128x4), this))
{ }

mm78_device::mm78_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm76_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm78l_device::mm78l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm78_device(mconfig, MM78L, tag, owner, clock, 11, address_map_constructor(FUNC(mm78l_device::program_2k), this), 7, address_map_constructor(FUNC(mm78l_device::data_128x4), this))
{ }

mm77_device::mm77_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm77_device(mconfig, MM77, tag, owner, clock, 11, address_map_constructor(FUNC(mm77_device::program_1_3k), this), 7, address_map_constructor(FUNC(mm77_device::data_96x4), this))
{ }

mm77_device::mm77_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm78_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm77l_device::mm77l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm77_device(mconfig, MM77L, tag, owner, clock, 11, address_map_constructor(FUNC(mm77l_device::program_1_5k), this), 7, address_map_constructor(FUNC(mm77l_device::data_96x4), this))
{ }


// internal memory maps
void mm78_device::program_2k(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void mm78_device::program_1_3k(address_map &map)
{
	map(0x040, 0x1ff).rom();
	map(0x240, 0x3ff).rom();
	map(0x640, 0x7ff).rom();
}

void mm78_device::program_1_5k(address_map &map)
{
	map(0x000, 0x3ff).rom();
	map(0x400, 0x5ff).mirror(0x200).rom();
}

void mm78_device::data_128x4(address_map &map)
{
	map(0x00, 0x7f).ram();
}

void mm78_device::data_96x4(address_map &map)
{
	map(0x00, 0x3f).ram();
	map(0x40, 0x47).mirror(0x18).ram(); // not to 0x50
	map(0x50, 0x57).ram();
	map(0x60, 0x67).mirror(0x18).ram(); // not to 0x70
	map(0x70, 0x77).ram();
}


// disasm
std::unique_ptr<util::disasm_interface> mm78_device::create_disassembler()
{
	return std::make_unique<mm78_disassembler>();
}


// initialize
void mm78_device::device_start()
{
	mm76_device::device_start();
	m_stack_levels = 2;

	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 6
}

void mm78_device::device_reset()
{
	mm76_device::device_reset();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void mm78_device::execute_one()
{
	if (op_is_tr(m_prev_op) && op_is_tr(m_prev2_op))
	{
		// 3-byte opcodes
		switch (m_op & 0xf0)
		{
			case 0x80: case 0x90: case 0xa0: case 0xb0: op_tmlb(); break;
			case 0xc0: case 0xd0: case 0xe0: case 0xf0: op_tlb(); break;

			default: op_illegal(); break;
		}
	}
	else if (op_is_tr(m_prev_op))
	{
		// 2-byte opcodes
		switch (m_op & 0xf0)
		{
			case 0x30: op_tr(); break;
			case 0x40: op_skbei(); break;
			case 0x60:
				if (m_op != 0x60)
					op_skaei();
				else
					op_illegal();
				break;

			case 0x80: case 0x90: case 0xa0: case 0xb0: op_tml(); break;
			case 0xc0: case 0xd0: case 0xe0: case 0xf0: op_tl(); break;

			default: op_illegal(); break;
		}
	}
	else
	{
		// standard opcodes
		switch (m_op & 0xf0)
		{
			case 0x10: op_lb(); break;
			case 0x30: op_tr(); break;
			case 0x40: op_lai(); break;
			case 0x60:
				if (m_op != 0x60)
					op_aisk();
				else
					op_i1sk();
				break;

			case 0x80: case 0x90: case 0xa0: case 0xb0: op_tm(); break;
			case 0xc0: case 0xd0: case 0xe0: case 0xf0: op_t(); break;

			default:
				switch (m_op & 0xfc)
				{
			case 0x08: case 0x0c: op_eob(); break;
			case 0x20: op_sb(); break;
			case 0x24: op_rb(); break;
			case 0x28: op_skbf(); break;
			case 0x50: op_l(); break;
			case 0x54: op_xnsk(); break;
			case 0x58: op_xdsk(); break;
			case 0x5c: op_x(); break;

			default:
				switch (m_op)
				{
			case 0x00: op_nop(); break;
			case 0x01: op_skisl(); break;
			case 0x02: op_sknc(); break;
			case 0x03: op_int0h(); break;
			case 0x04: op_int1l(); break;
			case 0x05: op_rc(); break;
			case 0x06: op_sc(); break;
			case 0x07: op_sag(); break;

			case 0x2c: break; // TAB
			case 0x2d: op_ios(); break;
			case 0x2e: op_rtsk(); break;
			case 0x2f: op_rt(); break;

			case 0x70: op_sos(); break;
			case 0x71: op_ros(); break;
			case 0x72: op_ix(); break;
			case 0x73: op_ox(); break;
			case 0x74: op_xas(); break;
			case 0x75: op_lxa(); break;
			case 0x76: op_lba(); break;
			case 0x77: op_com(); break;
			case 0x78: op_i2c(); break;
			case 0x79: op_xax(); break;
			case 0x7a: op_xab(); break;
			case 0x7b: op_ioa(); break;
			case 0x7c: op_ac(); break;
			case 0x7d: op_acsk(); break;
			case 0x7e: op_a(); break;
			case 0x7f: op_skmea(); break;

			default: op_illegal(); break; // won't happen
				}
				break; // 0xff

				}
				break; // 0xfc

		} // 0xf0
	}

	// TAB is delayed by 1 opcode (including a single TR)
	if (m_prev_op == 0x2c)
		op_tab();
}
