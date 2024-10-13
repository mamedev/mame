// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM76 MCU

*/

#include "emu.h"
#include "mm76.h"

#include "pps41d.h"


DEFINE_DEVICE_TYPE(MM76, mm76_device, "mm76", "Rockwell MM76 A7600") // 640 bytes ROM, 48 nibbles RAM
DEFINE_DEVICE_TYPE(MM76L, mm76l_device, "mm76l", "Rockwell MM76L B7600") // low-power
DEFINE_DEVICE_TYPE(MM76E, mm76e_device, "mm76e", "Rockwell MM76E A8600") // ROM extended to 1KB
DEFINE_DEVICE_TYPE(MM76EL, mm76el_device, "mm76el", "Rockwell MM76EL B8600") // low-power


// constructor
mm76_device::mm76_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76_device(mconfig, MM76, tag, owner, clock, 10, address_map_constructor(FUNC(mm76_device::program_0_6k), this), 6, address_map_constructor(FUNC(mm76_device::data_48x4), this))
{ }

mm76_device::mm76_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	pps41_base_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm76l_device::mm76l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76_device(mconfig, MM76L, tag, owner, clock, 10, address_map_constructor(FUNC(mm76l_device::program_0_6k), this), 6, address_map_constructor(FUNC(mm76l_device::data_48x4), this))
{ }

mm76e_device::mm76e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76e_device(mconfig, MM76E, tag, owner, clock, 10, address_map_constructor(FUNC(mm76e_device::program_1k), this), 6, address_map_constructor(FUNC(mm76e_device::data_48x4), this))
{ }

mm76e_device::mm76e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mm76_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

mm76el_device::mm76el_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76e_device(mconfig, MM76EL, tag, owner, clock, 10, address_map_constructor(FUNC(mm76el_device::program_1k), this), 6, address_map_constructor(FUNC(mm76el_device::data_48x4), this))
{ }


// internal memory maps
void mm76_device::program_0_6k(address_map &map)
{
	map(0x000, 0x17f).mirror(0x200).rom();
	map(0x180, 0x1ff).rom();
	map(0x380, 0x3ff).rom();
}

void mm76e_device::program_1k(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void mm76_device::data_48x4(address_map &map)
{
	map(0x00, 0x2f).ram();
	map(0x30, 0x3f).lr8(NAME([]() { return 0xf; })).nopw();
}


// machine config
void mm76_device::device_add_mconfig(machine_config &config)
{
	PLA(config, "opla", 5, 8, 17).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> mm76_device::create_disassembler()
{
	return std::make_unique<mm76_disassembler>();
}


// initialize
void mm76_device::device_start()
{
	pps41_base_device::device_start();
}

void mm76_device::device_reset()
{
	pps41_base_device::device_reset();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void mm76_device::execute_one()
{
	if (op_is_tr(m_prev_op))
	{
		// 2-byte opcodes
		switch (m_op & 0xf0)
		{
			case 0x20: op_skbei(); break;
			case 0x60: op_skaei(); break;

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
			case 0x20: op_lb(); break;
			case 0x30: op_tr(); break;
			case 0x60: op_aisk(); break;
			case 0x70: op_lai(); break;

			case 0x80: case 0x90: case 0xa0: case 0xb0: op_tm(); break;
			case 0xc0: case 0xd0: case 0xe0: case 0xf0: op_t(); break;

			default:
				switch (m_op & 0xfc)
				{
			case 0x08: op_skbf(); break;
			case 0x10: op_sb(); break;
			case 0x14: op_rb(); break;
			case 0x1c: op_eob(); break;
			case 0x50: op_l(); break;
			case 0x54: op_xnsk(); break;
			case 0x58: op_x(); break;
			case 0x5c: op_xdsk(); break;

			default:
				switch (m_op)
				{
			case 0x00: op_nop(); break;
			case 0x01: op_sknc(); break;
			case 0x02: op_rt(); break;
			case 0x03: op_rtsk(); break;
			case 0x04: op_int0l(); break;
			case 0x05: op_int1h(); break;
			case 0x06: op_din1(); break;
			case 0x07: op_din0(); break;
			case 0x0c: op_sc(); break;
			case 0x0d: op_rc(); break;
			case 0x0e: op_seg1(); break;
			case 0x0f: op_seg2(); break;

			case 0x18: op_oa(); break;
			case 0x19: op_ob(); break;
			case 0x1a: op_iam(); break;
			case 0x1b: op_ibm(); break;

			case 0x40: op_ac(); break;
			case 0x41: op_acsk(); break;
			case 0x42: op_a(); break;
			case 0x43: op_ask(); break;
			case 0x44: op_lba(); break;
			case 0x45: op_com(); break;
			case 0x46: op_xab(); break;
			case 0x47: op_skmea(); break;
			case 0x4a: op_i1(); break;
			case 0x4b: op_i2c(); break;
			case 0x4c: op_lsa(); break;
			case 0x4d: op_ios(); break;
			case 0x4e: op_xas(); break;

			default: op_illegal(); break;
				}
				break; // 0xff

				}
				break; // 0xfc

		} // 0xf0
	}
}
