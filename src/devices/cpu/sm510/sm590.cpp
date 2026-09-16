// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
/*

  Sharp SM590 MCU core implementation

  TODO:
  - if BL >= 4, are port accesses mirrored or ignored? (right now assuming mirrored)
  - R port mask options

*/

#include "emu.h"
#include "sm590.h"
#include "sm510d.h"


// MCU types
DEFINE_DEVICE_TYPE(SM590, sm590_device, "sm590", "Sharp SM590") // 512x8 ROM, 32x4 RAM
DEFINE_DEVICE_TYPE(SM591, sm591_device, "sm591", "Sharp SM591") // 1kx8 ROM, 56x4 RAM
DEFINE_DEVICE_TYPE(SM595, sm595_device, "sm595", "Sharp SM595") // 768x8 ROM, 32x4 RAM


// internal memory maps
void sm590_device::program_512x8(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void sm590_device::program_768x8(address_map &map)
{
	map(0x000, 0x2ff).rom();
}

void sm590_device::program_1kx8(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void sm590_device::data_32x4(address_map &map)
{
	map(0x00, 0x1f).ram();
}

void sm590_device::data_56x4(address_map &map)
{
	map(0x00, 0x2f).ram();
	map(0x30, 0x37).mirror(0x08).ram();
}


// device definitions
sm590_device::sm590_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	sm510_base_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data),
	m_write_rx(*this),
	m_read_rx(*this, 0)
{ }

sm590_device::sm590_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm590_device(mconfig, SM590, tag, owner, clock, 4 /* stack levels */, 9 /* prg width */, address_map_constructor(FUNC(sm590_device::program_512x8), this), 5 /* data width */, address_map_constructor(FUNC(sm590_device::data_32x4), this))
{ }

sm591_device::sm591_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm590_device(mconfig, SM591, tag, owner, clock, 4, 10, address_map_constructor(FUNC(sm591_device::program_1kx8), this), 6, address_map_constructor(FUNC(sm591_device::data_56x4), this))
{ }

sm595_device::sm595_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm590_device(mconfig, SM595, tag, owner, clock, 4, 10, address_map_constructor(FUNC(sm595_device::program_768x8), this), 5, address_map_constructor(FUNC(sm595_device::data_32x4), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> sm590_device::create_disassembler()
{
	return std::make_unique<sm590_disassembler>();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sm590_device::device_start()
{
	// common init
	sm510_base_device::device_start();

	// init/register for savestates
	m_pagemask = 0x7f;
	save_item(NAME(m_rports));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm590_device::device_reset()
{
	// common reset
	sm510_base_device::device_reset();

	// clear outputs
	for (int i = 0; i < 4; i++)
		port_w(i, 0);
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm590_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x00: op_adx(); break;
		case 0x10: op_tax(); break;
		case 0x20: op_lblx(); break;
		case 0x30: op_lax(); break;
		case 0x80: case 0x90: case 0xa0: case 0xb0:
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_t(); break; // TR

		default:
			switch (m_op & 0xfc)
			{
		case 0x60: op_tmi(); break; // TM
		case 0x64: op_tba(); break;
		case 0x68: op_rm(); break;
		case 0x6c: op_sm(); break;
		case 0x74: op_lbmx(); break;
		case 0x78: op_tl(); break;
		case 0x7c: op_tls(); break;

		default:
			switch (m_op)
			{
		case 0x40: op_lda(); break;
		case 0x41: op_exc(); break;
		case 0x42: op_exci(); break;
		case 0x43: op_excd(); break;
		case 0x44: op_coma(); break;
		case 0x45: op_tam(); break;
		case 0x46: op_atr(); break;
		case 0x47: op_mtr(); break;
		case 0x48: op_rc(); break;
		case 0x49: op_sc(); break;
		case 0x4a: op_str(); break;
		case 0x4b: op_cend(); break; // CCTRL
		case 0x4c: op_rtn0(); break; // RTN
		case 0x4d: op_rtn1(); break; // RTNS
		case 0x50: op_inbm(); break;
		case 0x51: op_debm(); break;
		case 0x52: op_incb(); break; // INBL
		case 0x53: op_decb(); break; // DEBL
		case 0x54: op_tc(); break;
		case 0x55: op_rta(); break;
		case 0x56: op_blta(); break;
		case 0x57: op_exbla(); break; // XBLA
		case 0x5c: op_atx(); break;
		case 0x5d: op_exax(); break;
		case 0x70: op_add(); break;
		case 0x71: op_ads(); break;
		case 0x72: op_adc(); break;
		case 0x73: op_add11(); break; // ADCS

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // 0xf0
}

bool sm590_device::op_argument()
{
	// TL, TLS(TML) opcodes are 2 bytes
	return (m_op & 0xf8) == 0x78;
}
