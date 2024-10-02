// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM530 MCU core implementation

  TODO:
  - is unused RAM unmapped? or mirrored?
  - add BP3,BP4, they connect the 1s or 1/100s counter to some of the LCD segs
  - what do BP1,BP2 do? datasheet block diagram shows BP is 4 bit, but it doesn't
    explain anywhere what they're for
  - add SM531: not just smaller ROM/RAM, melody controller supposedly supports
    envelopes (melody ROM is 128x7 instead of 256x6)

*/

#include "emu.h"
#include "sm530.h"
#include "sm510d.h"


// MCU types
DEFINE_DEVICE_TYPE(SM530, sm530_device, "sm530", "Sharp SM530") // 2Kx8 ROM, 88x4 RAM(24x4 for LCD), melody controller


// internal memory maps
void sm530_device::program_2k(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void sm530_device::data_64_24x4(address_map &map)
{
	map(0x00, 0x3f).ram();
	map(0x40, 0x4b).mirror(0x20).ram().share("lcd_ram_a");
	map(0x50, 0x5b).mirror(0x20).ram().share("lcd_ram_b");
}


// disasm
std::unique_ptr<util::disasm_interface> sm530_device::create_disassembler()
{
	return std::make_unique<sm530_disassembler>();
}


// device definitions
sm530_device::sm530_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	sm511_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data),
	m_write_f(*this)
{ }

sm530_device::sm530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sm530_device(mconfig, SM530, tag, owner, clock, 1 /* stack levels */, 11 /* prg width */, address_map_constructor(FUNC(sm530_device::program_2k), this), 7 /* data width */, address_map_constructor(FUNC(sm530_device::data_64_24x4), this))
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sm530_device::device_start()
{
	// common init
	sm511_device::device_start();

	// zerofill
	m_subdiv = 0;
	m_count_1s = 0;
	m_count_10ms = 0;
	m_ds = false;

	// register for savestates
	save_item(NAME(m_subdiv));
	save_item(NAME(m_count_1s));
	save_item(NAME(m_count_10ms));
	save_item(NAME(m_ds));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm530_device::device_reset()
{
	// common reset
	sm511_device::device_reset();

	// assume LCD is on
	m_bp = 0;
	m_ds = true;
}


//-------------------------------------------------
//  lcd driver
//-------------------------------------------------

void sm530_device::lcd_update()
{
	// 2 rows
	for (int h = 0; h < 2; h++)
	{
		for (int o = 0; o < m_lcd_ram_a.bytes(); o++)
		{
			// 4 segments per group
			u8 seg = h ? m_lcd_ram_b[o] : m_lcd_ram_a[o];
			m_write_segs(o << 1 | h, m_ds ? seg : 0);
		}
	}
}


//-------------------------------------------------
//  divider
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sm530_device::div_timer_cb)
{
	m_div = (m_div + 1) & 0x7fff;

	if (m_div == 0)
	{
		// gamma signal on 1s
		m_gamma |= 2;

		// increment seconds counter
		m_count_1s = (m_count_1s + 1) % 10;

		// gamma signal on 10s
		if (m_count_1s == 0)
			m_gamma |= 1;
	}

	// gamma signal on 0.5s
	if ((m_div & 0x3fff) == 0)
		m_gamma |= 4;

	// secondary timer for 1/100s intervals
	if ((m_div & 0xff) < 250)
	{
		m_subdiv = (m_subdiv + 1) % 32000;

		if ((m_subdiv % 320) == 0)
		{
			// increment 1/100s counter
			m_count_10ms = (m_count_10ms + 1) % 10;

			// gamma signal on 0.1s
			if (m_count_10ms == 0)
				m_gamma |= 8;
		}
	}

	clock_melody();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm530_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x00: op_adx(); break;
		case 0x10: op_lax(); break;
		case 0x30: op_lb(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_t(); break; // TR
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_trs(); break;

		default:
			switch (m_op & 0xfc)
			{
		case 0x20: op_lda(); break;
		case 0x24: op_exc(); break;
		case 0x28: op_exci(); break;
		case 0x2c: op_excd(); break;
		case 0x40: op_rm(); break;
		case 0x44: op_sm(); break;
		case 0x48: op_tmi(); break; // TM
		case 0x60: case 0x64: op_tl(); break;
		case 0x6c: op_tg(); break;

		default:
			switch (m_op)
			{
		case 0x4c: op_incb(); break;
		case 0x4d: op_decb(); break;
		case 0x4e: op_rds(); break;
		case 0x4f: op_sds(); break;

		case 0x50: op_kta(); break;
		case 0x51: op_keta(); break;
		case 0x52: op_dta(); break;
		case 0x53: op_coma(); break;
		case 0x54: op_add(); break;
		case 0x55: op_add11(); break; // ADDC
		case 0x56: op_rc(); break;
		case 0x57: op_sc(); break;
		case 0x58: op_tabl(); break;
		case 0x59: op_tam(); break;
		case 0x5a: op_exbla(); break; // EXBL
		case 0x5b: op_tc(); break;
		case 0x5c: op_ats(); break;
		case 0x5d: op_atf(); break;
		case 0x5e: op_atbp(); break;

		case 0x68: op_rtn0(); break; // RTN
		case 0x69: op_rtn1(); break; // RTNS
		case 0x6a: op_atpl(); break;
		case 0x6b: op_lbl(); break;

		case 0x70: op_idiv(); break;
		case 0x71: op_inis(); break;
		case 0x72: op_sbm(); break; // SABM
		case 0x73: op_sabl(); break;
		case 0x74: op_cend(); break;
		case 0x75: op_tmel(); break;
		case 0x76: op_rme(); break;
		case 0x77: op_sme(); break;
		case 0x78: op_pre(); break;
		case 0x79: op_tal(); break; // TBA

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // 0xf0

	// SABM/SABL is only valid for 1 step
	m_bmask = (m_op == 0x72) ? 0x40 : ((m_op == 0x73) ? 0x08 : 0);
}

bool sm530_device::op_argument()
{
	// LBL, PRE, TL opcodes are 2 bytes
	return m_op == 0x6b || m_op == 0x78 || ((m_op & 0xf8) == 0x60);
}
