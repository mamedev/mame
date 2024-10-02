// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, hap
/*******************************************************************************

Driver for Ritam Monty Plays Scrabble Brand Crossword Game - Portable Computer Console

Scrabble computer that allows you play a game of Scrabble by yourself (or you
can play with up to 3 players). Has a built-in 12,000 vocabulary, expandable
to 44,000 by way of 2 expansion modules each containing 16,000 more obscure words.
You can use the included 'score cards' (which look like little Scrabble boards),
or you can use a real Scrabble board and tiles to play.  Also note, Monty
apparently originally came with a little pen.

This game was later upgraded by Ritam to Master Monty which had 24,000 words
built-in (expandable to a total of 56,000 with the same 2 expansion modules).
Two variations on Master Monty have been seen: one looks exactly the same as the
Monty but the electronics on the inside have been upgraded.  The later version
is blue and says Master Monty at the top.  Both of these versions are hand-upgraded
by adding chips and wires to the inside of the game.

Hardware notes:
- Z80, 3.58MT ceramic resonator
- 2KB SRAM, 16KB ROM(32KB on mmonty)
- 2*16KB ROM sockets for vocabulary expansion
- 2*SED1503F, 40*32 LCD screen, beeper

TODO:
- put expansion roms in softwarelist? (note: slot #1 only accepts rom #1,
  slot #2 only accepts rom #2), they can be disabled in-game with option M

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "video/sed1500.h"

#include "screen.h"
#include "speaker.h"

#include "monty.lh"


namespace {

class monty_state : public driver_device
{
public:
	monty_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcd(*this, "lcd%u", 0)
		, m_dac(*this, "dac")
		, m_inputs(*this, "IN.%u", 0)
	{ }

	void key_pressed(int state);

	void monty(machine_config &config);
	void mmonty(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device_array<sed1503_device, 2> m_lcd;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<6> m_inputs;

	u64 m_lcd_data[32] = { };
	int m_lcd_cs = 0;
	int m_halt = 0;

	void monty_mem(address_map &map) ATTR_COLD;
	void mmonty_mem(address_map &map) ATTR_COLD;
	void monty_io(address_map &map) ATTR_COLD;

	template<int N> void lcd_output_w(offs_t offset, u64 data) { m_lcd_data[N << 4 | offset] = data; } // buffer for screen_update
	u32 screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	void control_w(offs_t offset, u8 data);
	void lcd_w(offs_t offset, u8 data) { m_lcd[m_lcd_cs]->write(offset, data); }
	u8 input_r(offs_t offset);
	void halt_changed(int state) { m_halt = state; }
};

void monty_state::machine_start()
{
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_cs));
	save_item(NAME(m_halt));
}



/*******************************************************************************
    Video
*******************************************************************************/

u32 monty_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	bitmap.fill(0xffffff);

	// letters with width 5 with space in between them
	for (int y = 0; y < 32; y++)
		for (int x = 0; x < 40; x++)
			bitmap.pix(y + 1, x + x/5 + 1) = BIT(m_lcd_data[y], x) ? 0 : 0xffffff;

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

void monty_state::control_w(offs_t offset, u8 data)
{
	// a0: speaker out
	m_dac->write(BIT(offset, 0));

	// a1: lcd chip select
	m_lcd_cs = BIT(offset, 1);
}

u8 monty_state::input_r(offs_t offset)
{
	u8 data = 0;

	// a0-a5, d0-d4: multiplexed inputs
	for (int i = 0; i < 6; i++)
		if (BIT(offset, i))
			data |= m_inputs[i]->read();

	return ~data;
}

void monty_state::key_pressed(int state)
{
	if (state && m_halt)
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void monty_state::monty_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).rom(); // slot 1
	map(0x8000, 0xbfff).rom(); // slot 2
	map(0xf800, 0xffff).ram();
}

void monty_state::mmonty_mem(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xbfff).rom(); // slot 2
	map(0xc000, 0xffff).rom(); // slot 1
}

void monty_state::monty_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).r(FUNC(monty_state::input_r));
	map(0x00, 0x03).w(FUNC(monty_state::control_w));
	map(0x80, 0xff).w(FUNC(monty_state::lcd_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( monty )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Erase") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Option") PORT_CODE(KEYCODE_SPACE) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Blank") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_DOWN) PORT_CHAR('M') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR('I') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_LEFT) PORT_CHAR('G') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_UP) PORT_CHAR('C') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void monty_state::monty(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 3.58_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &monty_state::monty_mem);
	m_maincpu->set_addrmap(AS_IO, &monty_state::monty_io);
	m_maincpu->halt_cb().set(FUNC(monty_state::halt_changed));

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(0);
	screen.set_size(40+8+1, 32+1);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(monty_state::screen_update));

	SED1503(config, m_lcd[0], 32768).write_segs().set(FUNC(monty_state::lcd_output_w<0>));
	SED1503(config, m_lcd[1], 32768).write_segs().set(FUNC(monty_state::lcd_output_w<1>));
	config.set_default_layout(layout_monty);

	// Sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void monty_state::mmonty(machine_config &config)
{
	monty(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &monty_state::mmonty_mem);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( monty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "monty_main.bin",    0x0000, 0x4000, CRC(720b4f55) SHA1(0106eb88d3fbbf25a745b9b6ee785ba13689d095) ) // 27128
	ROM_LOAD( "monty_module1.bin", 0x4000, 0x4000, CRC(2725d8c3) SHA1(8273b9779c0915f9c7c43ea4fb460f43ce036358) ) // 27128
	ROM_LOAD( "monty_module2.bin", 0x8000, 0x4000, CRC(db672e47) SHA1(bb14fe86df06cfa4b19625ba417d1a5bc8eae155) ) // 27128
ROM_END

ROM_START( mmonty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "master_monty_main.bin", 0x0000, 0x8000, CRC(bb5ef4d4) SHA1(ba2c759e429f8740df419f9abb60832eddfba8ab) ) // 27C256
	ROM_LOAD( "monty_module2.bin",     0x8000, 0x4000, CRC(db672e47) SHA1(bb14fe86df06cfa4b19625ba417d1a5bc8eae155) ) // 27128
	ROM_LOAD( "monty_module1.bin",     0xc000, 0x4000, CRC(2725d8c3) SHA1(8273b9779c0915f9c7c43ea4fb460f43ce036358) ) // 27128
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                FLAGS
SYST( 1983, monty,  0,      0,      monty,   monty, monty_state, empty_init, "Ritam", "Monty Plays Scrabble", MACHINE_SUPPORTS_SAVE )
SYST( 1987, mmonty, 0,      0,      mmonty,  monty, monty_state, empty_init, "Ritam", "Master Monty",         MACHINE_SUPPORTS_SAVE )
