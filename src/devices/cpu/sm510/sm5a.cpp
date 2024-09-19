// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  Sharp SM5A MCU core implementation

  TODO:
  - confirm SM5A mnemonics

*/

#include "emu.h"
#include "sm5a.h"
#include "sm510d.h"


// MCU types
DEFINE_DEVICE_TYPE(SM5A, sm5a_device, "sm5a", "Sharp SM5A") // 1.8K ROM, 5x13x4 RAM, shift registers for LCD
DEFINE_DEVICE_TYPE(SM5L, sm5l_device, "sm5l", "Sharp SM5L") // low-power version of SM5A
DEFINE_DEVICE_TYPE(KB1013VK12, kb1013vk12_device, "kb1013vk1_2", "KB1013VK1-2") // Soviet-era clone of SM5A


// internal memory maps
void sm5a_device::program_1_8k(address_map &map)
{
	map(0x000, 0x6ff).rom();
	map(0x700, 0x73f).rom().mirror(0x0c0);
}

void sm5a_device::data_5x13x4(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(sm5a_device::mirror_r), FUNC(sm5a_device::mirror_w)); // d,e,f -> c
	map(0x00, 0x0c).ram();
	map(0x10, 0x1c).ram();
	map(0x20, 0x2c).ram();
	map(0x30, 0x3c).ram();
	map(0x40, 0x4c).ram().mirror(0x30);
}


// device definitions
sm5a_device::sm5a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int o_pins, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	sm500_device(mconfig, type, tag, owner, clock, stack_levels, o_pins, prgwidth, program, datawidth, data)
{ }

sm5a_device::sm5a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm5a_device(mconfig, SM5A, tag, owner, clock, 1 /* stack levels */, 9 /* o group pins */, 11 /* prg width */, address_map_constructor(FUNC(sm5a_device::program_1_8k), this), 7 /* data width */, address_map_constructor(FUNC(sm5a_device::data_5x13x4), this))
{ }

sm5l_device::sm5l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm5a_device(mconfig, SM5L, tag, owner, clock, 1, 9, 11, address_map_constructor(FUNC(sm5l_device::program_1_8k), this), 7, address_map_constructor(FUNC(sm5l_device::data_5x13x4), this))
{ }

kb1013vk12_device::kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm5a_device(mconfig, KB1013VK12, tag, owner, clock, 1, 9, 11, address_map_constructor(FUNC(kb1013vk12_device::program_1_8k), this), 7, address_map_constructor(FUNC(kb1013vk12_device::data_5x13x4), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> sm5a_device::create_disassembler()
{
	return std::make_unique<sm5a_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm5a_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: op_lax(); break;
		case 0x30: op_adx(); break;
		case 0x40: op_lb(); break;
		case 0x70: op_ssr(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_tr(); break;
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_trs(); break;

		default:
			switch (m_op & 0xfc)
			{
		case 0x04: op_rm(); break;
		case 0x0c: op_sm(); break;
		case 0x10: op_exc(); break;
		case 0x14: op_exci(); break;
		case 0x18: op_lda(); break;
		case 0x1c: op_excd(); break;
		case 0x54: op_tmi(); break;

		default:
			switch (m_op)
			{
		case 0x00: op_skip(); break;
		case 0x01: op_atr(); break;
		case 0x02: op_sbm(); break;
		case 0x03: op_atbp(); break;
		case 0x08: op_add(); break;
		case 0x09: op_add11(); break;
		case 0x0a: op_coma(); break;
		case 0x0b: op_exbla(); break;

		case 0x50: op_tal(); break;
		case 0x51: op_tb(); break;
		case 0x52: op_tc(); break;
		case 0x53: op_tam(); break;
		case 0x58: op_tis(); break;
		case 0x59: op_ptw(); break;
		case 0x5a: op_ta0(); break;
		case 0x5b: op_tabl(); break;
		case 0x5c: op_tw(); break;
		case 0x5d: op_dtw(); break;
		case 0x5f: op_lbl(); break;

		case 0x60: op_comcn(); break;
		case 0x61: op_pdtw(); break;
		case 0x62: op_wr(); break;
		case 0x63: op_ws(); break;
		case 0x64: op_incb(); break;
		case 0x65: op_idiv(); break;
		case 0x66: op_rc(); break;
		case 0x67: op_sc(); break;
		case 0x68: op_rmf(); break;
		case 0x69: op_smf(); break;
		case 0x6a: op_kta(); break;
		case 0x6b: op_rbm(); break;
		case 0x6c: op_decb(); break;
		case 0x6d: op_comcb(); break;
		case 0x6e: op_rtn0(); break;
		case 0x6f: op_rtn1(); break;

		// extended opcodes
		case 0x5e:
			m_op = m_op << 8 | m_param;
			switch (m_param)
			{
		case 0x00: op_cend(); break;
		case 0x04: op_dta(); break;

		default: op_illegal(); break;
			}
			break; // 0x5e

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // 0xf0
}
