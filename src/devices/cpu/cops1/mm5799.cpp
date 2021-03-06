// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor MM5799 MCU

*/

#include "emu.h"
#include "mm5799.h"

#include "cops1d.h"


DEFINE_DEVICE_TYPE(MM5799, mm5799_device, "mm5799", "National Semiconductor MM5799")


// constructor
mm5799_device::mm5799_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cops1_base_device(mconfig, MM5799, tag, owner, clock, 11, address_map_constructor(FUNC(mm5799_device::program_map), this), 7, address_map_constructor(FUNC(mm5799_device::data_map), this))
{ }


// internal memory maps
void mm5799_device::program_map(address_map &map)
{
	map(0x0000, 0x01ff).rom();
	map(0x0400, 0x07ff).rom();
}

void mm5799_device::data_map(address_map &map)
{
	if (m_option_ram_d12)
	{
		// 8x12x4
		for (int i = 0; i < 0x80; i += 0x10)
			map(i | 0x04, i | 0x0f).ram();
		map(0x00, 0x03).mirror(0x70).noprw();
	}
	else
	{
		// 6x16x4
		map(0x00, 0x3f).ram();
		for (int i = 0x40; i < 0x80; i += 0x10)
			map(i | 0x00, i | 0x07).ram().mirror(0x08);
	}
}


// disasm
std::unique_ptr<util::disasm_interface> mm5799_device::create_disassembler()
{
	return std::make_unique<mm5799_disassembler>();
}


// initialize
void mm5799_device::device_start()
{
	cops1_base_device::device_start();
}

void mm5799_device::device_reset()
{
	cops1_base_device::device_reset();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void mm5799_device::execute_one()
{
	switch (m_op & 0xc0)
	{
		case 0x00:
			switch (m_op)
			{
		case 0x00: op_nop(); break;
		case 0x10: op_dspa(); break;
		case 0x20: op_comp(); break;
		case 0x30: op_0ta(); break;

		case 0x01: op_hxbr(); break;
		case 0x11: op_dsps(); break;
		case 0x21: op_axo(); break;
		case 0x31: op_hxa(); break;

		case 0x02: op_add(); break;
		case 0x12: op_ad(); break;
		case 0x22: op_sub(); break;
		case 0x32: op_tam(); break;

		case 0x03: op_sc(); break;
		case 0x13: op_lbl(); break;
		case 0x23: op_rsc(); break;
		case 0x33: op_ldf(); break;

		case 0x34: op_read(); break;

		case 0x05: op_tir(); break;
		case 0x15: op_tkb(); break;
		case 0x25: op_btd(); break;
		case 0x35: op_tin(); break;

		case 0x04: case 0x14: case 0x24: op_tf(); break;

		case 0x06: case 0x16: case 0x26: case 0x36: op_mta(); break;
		case 0x07: case 0x17: case 0x27: case 0x37: op_exc(); break;
		case 0x08: case 0x18: case 0x28: case 0x38: op_excm(); break;
		case 0x09: case 0x19: case 0x29: case 0x39: op_excp(); break;

		default: op_lb(); break;
			}
			break; // 0x00

		case 0x40:
			switch (m_op & 0x30)
			{
		case 0x00:
			switch (m_op & 0x0f)
			{
		case 0x00: op_ret(); break;
		case 0x01: op_rets(); break;
		case 0x03: op_bta(); break;
		case 0x0d: op_tc(); break;

		case 0x02: case 0x08: case 0x0b: case 0x0c: op_rsm(); break;
		case 0x04: case 0x05: case 0x06: case 0x07: op_tm(); break;
		case 0x09: case 0x0a: case 0x0e: case 0x0f: op_sm(); break;
			}
			break;

		case 0x10:
			if ((m_op & 0x0f) != 0)
				op_adx();
			else
				op_atb();
			break;

		case 0x20: op_lg(); break;
		case 0x30: op_lm(); break;
			}
			break; // 0x40

		case 0x80: op_call(); break;
		case 0xc0: op_go(); break;
	}
}

bool mm5799_device::op_argument()
{
	// 2-byte instructions: LBL, LDF, LG
	return m_op == 0x13 || m_op == 0x33 || (m_op & 0xf0) == 0x60;
}
