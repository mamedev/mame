// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for IKT-5A terminal.

*******************************************************************************/

#include "emu.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "screen.h"


namespace {

class ikt5a_state : public driver_device
{
public:
	ikt5a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_keyboard(*this, "keyboard")
		, m_rs232(*this, "rs232")
		, m_chargen(*this, "chargen")
		, m_keyboard_clk(true)
		, m_keyboard_data(true)
		, m_keyboard_shifter(0)
	{
	}

	void ikt5a(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void keyboard_clk_w(int state);
	void keyboard_data_w(int state);

	void eeprom_w(u8 data);
	void keyboard_ack_w(u8 data);
	void keyboard_shift_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);
	u8 p3_r();

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<pc_kbdc_device> m_keyboard;
	required_device<rs232_port_device> m_rs232;
	required_region_ptr<u8> m_chargen;

	bool m_keyboard_clk;
	bool m_keyboard_data;
	u8 m_keyboard_shifter;
};

void ikt5a_state::machine_start()
{
	save_item(NAME(m_keyboard_clk));
	save_item(NAME(m_keyboard_data));
	save_item(NAME(m_keyboard_shifter));
}

u32 ikt5a_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ikt5a_state::keyboard_clk_w(int state)
{
	if (m_keyboard_clk && !state)
	{
		m_maincpu->set_input_line(MCS51_INT1_LINE, BIT(m_keyboard_shifter, 0) ? ASSERT_LINE : CLEAR_LINE);
		m_keyboard_shifter >>= 1;
		if (m_keyboard_data)
			m_keyboard_shifter |= 0x80;
	}
	m_keyboard_clk = state;
}

void ikt5a_state::keyboard_data_w(int state)
{
	m_keyboard_data = state;
}

void ikt5a_state::eeprom_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 6));
	m_eeprom->di_write(BIT(data, 3));
	m_eeprom->clk_write(BIT(data, 1));

	m_keyboard->clock_write_from_mb(!BIT(data, 0));
}

void ikt5a_state::keyboard_ack_w(u8 data)
{
	m_maincpu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
	m_keyboard_shifter = 0;
}

void ikt5a_state::keyboard_shift_w(u8 data)
{
	m_maincpu->set_input_line(MCS51_INT1_LINE, BIT(m_keyboard_shifter, 0) ? ASSERT_LINE : CLEAR_LINE);
	m_keyboard_shifter >>= 1;
}

u8 ikt5a_state::p1_r()
{
	return 0xff;
}

void ikt5a_state::p1_w(u8 data)
{
	// TODO: P1.1 is keyboard-related
}

u8 ikt5a_state::p3_r()
{
	return 0xde | (m_eeprom->do_read() << 5) | m_rs232->rxd_r();
}

void ikt5a_state::prog_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
}

void ikt5a_state::ext_map(address_map &map)
{
	map(0x6400, 0x6400).mirror(0xff).w(FUNC(ikt5a_state::eeprom_w));
	map(0x7000, 0x7000).mirror(0xff).w(FUNC(ikt5a_state::keyboard_ack_w));
	map(0x7c00, 0x7c00).mirror(0xff).w(FUNC(ikt5a_state::keyboard_shift_w));
	map(0x8000, 0x9fff).ram();
}

static const gfx_layout ikt5a_charlayout =
{
	8, 14,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 15*8, 4*8, 12*8, 2*8, 10*8, 6*8, 14*8, 1*8, 9*8, 5*8, 13*8, 3*8, 11*8, 7*8 },
	16*8
};

static GFXDECODE_START(gfx_ikt5a)
	GFXDECODE_ENTRY("chargen", 0x0000, ikt5a_charlayout, 0, 1)
GFXDECODE_END

void ikt5a_state::ikt5a(machine_config &config)
{
	I80C51(config, m_maincpu, 15_MHz_XTAL / 2); // PCB 80C51BH-2 (clock uncertain)
	m_maincpu->set_addrmap(AS_PROGRAM, &ikt5a_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &ikt5a_state::ext_map);
	m_maincpu->port_in_cb<1>().set(FUNC(ikt5a_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(ikt5a_state::p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(ikt5a_state::p3_r));
	m_maincpu->port_out_cb<3>().set(m_rs232, FUNC(rs232_port_device::write_txd)).bit(1);

	EEPROM_93C06_16BIT(config, m_eeprom); // ST M9306B6

	PC_KBDC(config, m_keyboard, pc_xt_keyboards, STR_KBD_IBM_PC_XT_83);
	m_keyboard->out_clock_cb().set(FUNC(ikt5a_state::keyboard_clk_w));
	m_keyboard->out_data_cb().set(FUNC(ikt5a_state::keyboard_data_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15_MHz_XTAL, 800, 0, 640, 375, 0, 350); // timings guessed
	screen.set_screen_update(FUNC(ikt5a_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, MCS51_INT0_LINE);

	GFXDECODE(config, "gfxdecode", "palette", gfx_ikt5a);
	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	RS232_PORT(config, m_rs232, default_rs232_devices, "loopback");
}

static INPUT_PORTS_START(ikt5a)
INPUT_PORTS_END

ROM_START(ikt5a) // 80C51 (+xtal 15.000) // 8k ram // RGB external, uses XT keyboard
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("ver_ih.bin",   0x0000, 0x4000, CRC(5a15b4e8) SHA1(cc0336892279b730f1596f31e129c5a898ecdc8f))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("g26.bin",      0x0000, 0x2000, CRC(657668be) SHA1(212a9eb1fb9b9c16f3cc606c6befbd913ddfa395))
ROM_END

} // anonymous namespace


COMP(1993, ikt5a, 0, 0, ikt5a, ikt5a, ikt5a_state, empty_init, "Creator / Fura Elektronik", "IKT-5A", MACHINE_IS_SKELETON)
