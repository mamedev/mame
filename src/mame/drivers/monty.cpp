// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    2015-05-08 Skeleton driver for Ritam Monty Plays Scrabble BRAND crossword game

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

    TODO:
    - Need instructions
    - Proper SED1503F emulation (it's simulated in-driver for now)
    - When it wants tiles, put 64 into FD1B (monty), 7D1B (mmonty) and press
      Enter.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "video/sed1520.h"
#include "screen.h"
#include "speaker.h"


class monty_state : public driver_device
{
public:
	monty_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_sed0(*this, "sed1520_0")
		, m_writeUpper(false)
		, m_halt(0)
	{
		for (auto & elem : m_pixels)
			elem = 0xff000000;
	}

	DECLARE_WRITE_LINE_MEMBER(key_pressed);

	void monty(machine_config &config);
	void mmonty(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(ioDisplayWrite_w);
	DECLARE_WRITE8_MEMBER(ioCommandWrite0_w);
	DECLARE_WRITE8_MEMBER(ioCommandWrite1_w);
	DECLARE_WRITE_LINE_MEMBER(halt_changed);

	// screen updates
	uint32_t lcd_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);
	SED1520_UPDATE_CB(screen_update);

	void mmonty_mem(address_map &map);
	void monty_io(address_map &map);
	void monty_mem(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<sed1520_device> m_sed0;     // TODO: This isn't actually a SED1520, it's a SED1503F
	//required_device<sed1520_device> m_sed1;   // TODO: Also, there are 2 SED1503Fs on the board - one is flipped upside down

	// Test
	uint8_t m_writeUpper;
	uint32_t m_pixels[42*32];
	bool m_sound_sw;
	bool m_dirty;
	int m_halt;
};


void monty_state::monty_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	//AM_RANGE(0x4000, 0x4000) // The main rom checks to see if another program is here on startup
	map(0xf800, 0xffff).ram();
}

void monty_state::mmonty_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	//AM_RANGE(0xc000, 0xc000) // The main rom checks to see if another program is here on startup
	map(0x8000, 0xffff).rom();
	map(0x7800, 0x7fff).ram();
}


void monty_state::monty_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(monty_state::ioCommandWrite0_w));
	map(0x01, 0x01).w(FUNC(monty_state::sound_w));
	map(0x02, 0x02).w(FUNC(monty_state::ioCommandWrite1_w));
	map(0x80, 0xff).w(FUNC(monty_state::ioDisplayWrite_w));

	// 7 reads from a bit shifted IO port
	map(0x01, 0x01).portr("X1");
	map(0x02, 0x02).portr("X2");
	map(0x04, 0x04).portr("X3");
	map(0x08, 0x08).portr("X4");
	map(0x10, 0x10).portr("X5");
	map(0x20, 0x20).portr("X6");
	map(0x40, 0x40).portr("X7");
}


// Input ports
static INPUT_PORTS_START( monty )
	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, monty_state, key_pressed)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0xFF, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


WRITE8_MEMBER( monty_state::sound_w )
{
	m_sound_sw ^= 1;
	m_speaker->level_w(m_sound_sw);
}


WRITE8_MEMBER( monty_state::ioCommandWrite0_w )
{
	//printf("(%04x) Command Port 0 write : %02x\n", m_maincpu->pc(), data);
	m_writeUpper = false;
}


WRITE8_MEMBER( monty_state::ioCommandWrite1_w )
{
	//if (data == 0xfe)
	//    printf("---\n");

	//printf("(%04x) Command Port 1 write : %02x\n", m_maincpu->pc(), data);
	m_writeUpper = true;
}


WRITE8_MEMBER( monty_state::ioDisplayWrite_w )
{
	m_dirty = true;
	// Offset directly corresponds to sed1503, DD RAM address (offset 0x7f may be special?)
	//printf("(%04x) %02x %02x\n", m_maincpu->pc(), offset, data);

	uint8_t x = offset & 0x3f;
	uint8_t y = (BIT(offset, 6) + (m_writeUpper ? 2 : 0)) << 3;

	// Skip the controller and write straight to the LCD    (pc=134f)
	for (int i = 0; i < 8; i++)
	{
		// Pixel color
		if (x < 42)
			m_pixels[(y*42) + x] = BIT(data, i) ? 0xffffffff : 0xff000000;

		y++;
	}
}


WRITE_LINE_MEMBER( monty_state::halt_changed )
{
	m_halt = state;
}


WRITE_LINE_MEMBER( monty_state::key_pressed )
{
	if (!state && m_halt)
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


uint32_t monty_state::lcd_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	if (!m_dirty)
		return 1;

	uint8_t x,y,z;
	m_dirty = false;
	for (y = 0; y < 32; y++)
	{
		for (z = 0; z < 8; z++)
		{
			for (x = 0; x < 5; x++)
			{
				bitmap.pix32(y, x+z*6) = m_pixels[y*42 + z*5 + x];
			}
			bitmap.pix32(y, 5+z*6) = 0; // space between letters
		}
		bitmap.pix32(y, 48) = m_pixels[y*42 + 40];
		bitmap.pix32(y, 49) = m_pixels[y*42 + 41];
	}

	return 0;
}


SED1520_UPDATE_CB(monty_state::screen_update)
{
	// TODO: Not really a SED1520 - there are two SED1503s
	return 0x00;
}


// TODO: Additional machine definition - Master Monty has a different memory layout
void monty_state::monty(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 3580000);       // Ceramic resonator labeled 3.58MT
	m_maincpu->set_addrmap(AS_PROGRAM, &monty_state::monty_mem);
	m_maincpu->set_addrmap(AS_IO, &monty_state::monty_io);
	m_maincpu->halt_cb().set(FUNC(monty_state::halt_changed));

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // Not accurate
	screen.set_size(50, 32); // Two SED1503s (42x16 pixels) control the top and bottom halves
	screen.set_visarea(0, 50-1, 0, 32-1);
	screen.set_screen_update(FUNC(monty_state::lcd_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	// LCD controller interfaces
	SED1520(config, m_sed0).set_screen_update_cb(FUNC(monty_state::screen_update));
}

void monty_state::mmonty(machine_config &config)
{
	monty(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &monty_state::mmonty_mem);
}


// ROM definitions
ROM_START( monty )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "monty_main.bin",    0x0000, 0x4000, CRC(720b4f55) SHA1(0106eb88d3fbbf25a745b9b6ee785ba13689d095) )   // 27128
	ROM_LOAD( "monty_module1.bin", 0x4000, 0x4000, CRC(2725d8c3) SHA1(8273b9779c0915f9c7c43ea4fb460f43ce036358) )   // 27128
	ROM_LOAD( "monty_module2.bin", 0x8000, 0x4000, CRC(db672e47) SHA1(bb14fe86df06cfa4b19625ba417d1a5bc8eae155) )   // 27128
ROM_END

ROM_START( mmonty )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "master_monty_main.bin", 0x0000, 0x8000, CRC(bb5ef4d4) SHA1(ba2c759e429f8740df419f9abb60832eddfba8ab) )   // 27C256
	ROM_LOAD( "monty_module1.bin",     0x8000, 0x4000, CRC(2725d8c3) SHA1(8273b9779c0915f9c7c43ea4fb460f43ce036358) )   // 27128
	ROM_LOAD( "monty_module2.bin",     0xc000, 0x4000, CRC(db672e47) SHA1(bb14fe86df06cfa4b19625ba417d1a5bc8eae155) )   // 27128
ROM_END


// Drivers
//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                FLAGS
COMP( 1980, monty,  0,      0,      monty,   monty, monty_state, empty_init, "Ritam", "Monty Plays Scrabble", MACHINE_NOT_WORKING )
COMP( 1982, mmonty, 0,      0,      mmonty,  monty, monty_state, empty_init, "Ritam", "Master Monty",         MACHINE_NOT_WORKING )
