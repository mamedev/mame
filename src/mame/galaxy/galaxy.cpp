// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Miodrag Milanovic
/***************************************************************************
Galaksija driver by Krzysztof Strzecha and Miodrag Milanovic

22/05/2008 Tape support added (Miodrag Milanovic)
21/05/2008 Galaksija plus initial support (Miodrag Milanovic)
20/05/2008 Added real video implementation (Miodrag Milanovic)
18/04/2005 Possibilty to disable ROM 2. 2k, 22k, 38k and 54k memory
       configurations added.
13/03/2005 Memory mapping improved. Palette corrected. Supprort for newer
           version of snapshots added. Lot of cleanups. Keyboard mapping
           corrected.
19/09/2002 malloc() replaced by image_malloc().
15/09/2002 Snapshot loading fixed. Code cleanup.
31/01/2001 Snapshot loading corrected.
09/01/2001 Fast mode implemented (many thanks to Kevin Thacker).
07/01/2001 Keyboard corrected (still some keys unknown).
           Horizontal screen positioning in video subsystem added.
05/01/2001 Keyboard implemented (some keys unknown).
03/01/2001 Snapshot loading added.
01/01/2001 Preliminary driver.

ToDo:
- pacmanp not showing its hi-res graphics - get black screen
- is the hack in the video still needed? commenting it out made no difference.

***************************************************************************/

#include "emu.h"
#include "galaxy.h"

#include "cpu/z80/z80.h"
#include "formats/gtp_cas.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"


void galaxy_state::galaxyp_io(address_map &map)
{
	map.global_mask(0x01);
	map.unmap_value_high();
	map(0x00, 0x00).w("ay8910", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).w("ay8910", FUNC(ay8910_device::data_w));
}


void galaxy_state::galaxy_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2037).mirror(0x07c0).r(FUNC(galaxy_state::keyboard_r));
	map(0x2038, 0x203f).mirror(0x07c0).w(FUNC(galaxy_state::latch_w));
	// see init_galaxy for ram placement
}

void galaxy_state::galaxyp_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom(); // ROM A
	map(0x1000, 0x1fff).rom(); // ROM B
	map(0x2000, 0x2037).mirror(0x07c0).r(FUNC(galaxy_state::keyboard_r));
	map(0x2038, 0x203f).mirror(0x07c0).w(FUNC(galaxy_state::latch_w));
	map(0x2800, 0xdfff).ram();
	map(0xe000, 0xefff).rom().region("maincpu",0x2000); // ROM C
	map(0xf000, 0xffff).rom().region("maincpu",0x3000); // ROM D
}

/* 2008-05 FP:
Small note about natural keyboard support. Currently:
- "List" is mapped to 'ESC'
- "Break" is mapped to 'F1'
- "Repeat" is mapped to 'F2'                           */

static INPUT_PORTS_START (galaxy)
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)       PORT_CHAR('A')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)       PORT_CHAR('B')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)       PORT_CHAR('C')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)       PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)       PORT_CHAR('E')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)       PORT_CHAR('F')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)       PORT_CHAR('G')

	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)       PORT_CHAR('H')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)       PORT_CHAR('I')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)       PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)       PORT_CHAR('K')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)       PORT_CHAR('L')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)       PORT_CHAR('M')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)       PORT_CHAR('N')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)       PORT_CHAR('O')

	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)       PORT_CHAR('P')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)       PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)       PORT_CHAR('R')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)       PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)       PORT_CHAR('T')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)       PORT_CHAR('U')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)       PORT_CHAR('V')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)       PORT_CHAR('W')

	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)       PORT_CHAR('X')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)       PORT_CHAR('Y')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)       PORT_CHAR('Z')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)   PORT_CHAR(' ')

	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)       PORT_CHAR('0') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)   PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)   PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR('=') PORT_CHAR('-')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("List") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE7")
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8, 10*128*8, 11*128*8, 12*128*8, 13*128*8, 14*128*8, 15*128*8 },
	8                   /* every char takes 1 x 16 bytes */
};

static GFXDECODE_START( gfx_galaxy )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END


void galaxy_state::galaxy(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6'144'000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxy_state::galaxy_mem);
	m_maincpu->set_vblank_int("screen", FUNC(galaxy_state::irq0_line_hold));
	m_maincpu->set_irq_acknowledge_callback(FUNC(galaxy_state::irq_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_palette("palette");
	m_screen->set_size(384, 212);
	m_screen->set_visarea(0, 384-1, 0, 208-1);
	m_screen->set_screen_update(FUNC(galaxy_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_galaxy);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* snapshot */
	SNAPSHOT(config, "snapshot", "gal").set_load_callback(FUNC(galaxy_state::snapshot_cb));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(gtp_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("galaxy_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("galaxy");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("6K").set_extra_options("2K,22K,38K,54K");
}

void galaxy_state::galaxyp(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6'144'000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxy_state::galaxyp_mem);
	m_maincpu->set_addrmap(AS_IO, &galaxy_state::galaxyp_io);
	m_maincpu->set_vblank_int("screen", FUNC(galaxy_state::irq0_line_hold));
	m_maincpu->set_irq_acknowledge_callback(FUNC(galaxy_state::irq_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_palette("palette");
	m_screen->set_size(384, 208);
	m_screen->set_visarea(0, 384-1, 0, 208-1);
	m_screen->set_screen_update(FUNC(galaxy_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_galaxy);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* snapshot */
	SNAPSHOT(config, "snapshot", "gal").set_load_callback(FUNC(galaxy_state::snapshot_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay(AY8910(config, "ay8910", 6'144'000 / 4));
	ay.add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(gtp_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("galaxy_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("galaxy");
}

ROM_START (galaxy)
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "galrom1.dd8", 0x0000, 0x1000, CRC(dc970a32) SHA1(dfc92163654a756b70f5a446daf49d7534f4c739) )
	ROM_LOAD( "galrom2.dd9", 0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "galchr.dd3",  0x0000, 0x0800, CRC(5c3b5bb5) SHA1(19429a61dc5e55ddec3242a8f695e06dd7961f88) )
ROM_END

ROM_START (galaxyp)
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "galrom1.bin", 0x0000, 0x1000, CRC(dc970a32) SHA1(dfc92163654a756b70f5a446daf49d7534f4c739) )
	ROM_LOAD( "galrom2.bin", 0x1000, 0x1000, CRC(5dc5a100) SHA1(5d5ab4313a2d0effe7572bb129193b64cab002c1) )
	ROM_LOAD( "galplus.bin", 0x2000, 0x1000, CRC(d4cfab14) SHA1(b507b9026844eeb757547679907394aa42055eee) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "galchr.dd3",  0x0000, 0x0800, CRC(5c3b5bb5) SHA1(19429a61dc5e55ddec3242a8f695e06dd7961f88) )
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT          COMPANY                                   FULLNAME */
COMP( 1983, galaxy,  0,      0,      galaxy,  galaxy,  galaxy_state, init_galaxy,  "Voja Antonic / Elektronika inzenjering", "Galaksija",      MACHINE_SUPPORTS_SAVE )
COMP( 1985, galaxyp, galaxy, 0,      galaxyp, galaxy,  galaxy_state, init_galaxyp, "Nenad Dunjic",                           "Galaksija plus", MACHINE_SUPPORTS_SAVE )
