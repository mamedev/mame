// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sky Raider driver

***************************************************************************/

#include "emu.h"
#include "includes/skyraid.h"

#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "screen.h"
#include "speaker.h"

void skyraid_state::machine_start()
{
	m_led.resolve();
}

void skyraid_state::skyraid_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));   // terrain
	palette.set_pen_color(1, rgb_t(0x18, 0x18, 0x18));
	palette.set_pen_color(2, rgb_t(0x30, 0x30, 0x30));
	palette.set_pen_color(3, rgb_t(0x48, 0x48, 0x48));
	palette.set_pen_color(4, rgb_t(0x60, 0x60, 0x60));
	palette.set_pen_color(5, rgb_t(0x78, 0x78, 0x78));
	palette.set_pen_color(6, rgb_t(0x90, 0x90, 0x90));
	palette.set_pen_color(7, rgb_t(0xa8, 0xa8, 0xa8));
	palette.set_pen_color(8, rgb_t(0x10, 0x10, 0x10));   // sprites
	palette.set_pen_color(9, rgb_t(0xe0, 0xe0, 0xe0));
	palette.set_pen_color(10, rgb_t(0xa0, 0xa0, 0xa0));
	palette.set_pen_color(11, rgb_t(0x48, 0x48, 0x48));
	palette.set_pen_color(12, rgb_t(0x10, 0x10, 0x10));
	palette.set_pen_color(13, rgb_t(0x48, 0x48, 0x48));
	palette.set_pen_color(14, rgb_t(0xa0, 0xa0, 0xa0));
	palette.set_pen_color(15, rgb_t(0xe0, 0xe0, 0xe0));
	palette.set_pen_color(16, rgb_t(0x00, 0x00, 0x00));   // missiles
	palette.set_pen_color(17, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(18, rgb_t(0x00, 0x00, 0x00));   // text
	palette.set_pen_color(19, rgb_t(0xe0, 0xe0, 0xe0));
}

READ8_MEMBER(skyraid_state::skyraid_port_0_r)
{
	uint8_t val = ioport("LANGUAGE")->read();

	if (ioport("STICKY")->read() > m_analog_range)
		val |= 0x40;
	if (ioport("STICKX")->read() > m_analog_range)
		val |= 0x80;

	return val;
}


WRITE8_MEMBER(skyraid_state::skyraid_range_w)
{
	m_analog_range = data & 0x3f;
}


WRITE8_MEMBER(skyraid_state::skyraid_offset_w)
{
	m_analog_offset = data & 0x3f;
}


WRITE8_MEMBER(skyraid_state::skyraid_scroll_w)
{
	m_scroll = data;
}


void skyraid_state::skyraid_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x300);
	map(0x0400, 0x040f).writeonly().share("pos_ram");
	map(0x0800, 0x087f).ram().mirror(0x480).share("alpha_num_ram");
	map(0x1000, 0x1000).r(FUNC(skyraid_state::skyraid_port_0_r));
	map(0x1001, 0x1001).portr("DSW");
	map(0x1400, 0x1400).portr("COIN");
	map(0x1401, 0x1401).portr("SYSTEM");
	map(0x1c00, 0x1c0f).writeonly().share("obj_ram");
	map(0x4000, 0x4000).w(FUNC(skyraid_state::skyraid_scroll_w));
	map(0x4400, 0x4400).w(FUNC(skyraid_state::skyraid_sound_w));
	map(0x4800, 0x4800).w(FUNC(skyraid_state::skyraid_range_w));
	map(0x5000, 0x5000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5800, 0x5800).w(FUNC(skyraid_state::skyraid_offset_w));
	map(0x7000, 0x7fff).rom();
	map(0xf000, 0xffff).rom();
}

static INPUT_PORTS_START( skyraid )
	PORT_START("LANGUAGE")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( French ) )
	PORT_DIPSETTING(    0x20, DEF_STR( German ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Spanish ) )
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_UNUSED) /* POT1 */
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_UNUSED) /* POT0 */

	PORT_START("DSW")
	PORT_DIPNAME( 0x30, 0x10, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x10, "80 Seconds" )
	PORT_DIPSETTING(    0x20, "100 Seconds" )
	PORT_DIPSETTING(    0x30, "120 Seconds" )
	PORT_DIPNAME( 0x40, 0x40, "DIP #5" )    /* must be OFF */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ))
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))

	/* coinage settings are insane, refer to the manual */

	PORT_START("COIN")
	PORT_DIPNAME( 0x0F, 0x01, DEF_STR( Coinage )) /* dial */
	PORT_DIPSETTING(    0x00, "Mode 0" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x02, "Mode 2" )
	PORT_DIPSETTING(    0x03, "Mode 3" )
	PORT_DIPSETTING(    0x04, "Mode 4" )
	PORT_DIPSETTING(    0x05, "Mode 5" )
	PORT_DIPSETTING(    0x06, "Mode 6" )
	PORT_DIPSETTING(    0x07, "Mode 7" )
	PORT_DIPSETTING(    0x08, "Mode 8" )
	PORT_DIPSETTING(    0x09, "Mode 9" )
	PORT_DIPSETTING(    0x0A, "Mode A" )
	PORT_DIPSETTING(    0x0B, "Mode B" )
	PORT_DIPSETTING(    0x0C, "Mode C" )
	PORT_DIPSETTING(    0x0D, "Mode D" )
	PORT_DIPSETTING(    0x0E, "Mode E" )
	PORT_DIPSETTING(    0x0F, "Mode F" )
	PORT_DIPNAME( 0x10, 0x10, "Score for Extended Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_COIN2)

	PORT_START("SYSTEM")
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_TILT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Hiscore Reset") PORT_CODE(KEYCODE_H)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_START1)
	PORT_SERVICE(0x80, IP_ACTIVE_LOW)

	PORT_START("STICKY")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("STICKX")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10)
INPUT_PORTS_END


static const gfx_layout skyraid_text_layout =
{
	16, 8,  /* width, height */
	64,     /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0, 0, 1, 1, 2, 2, 3, 3,
		4, 4, 5, 5, 6, 6, 7, 7
	},
	{
		0x38, 0x30, 0x28, 0x20, 0x18, 0x10, 0x08, 0x00
	},
	0x40
};


static const gfx_layout skyraid_sprite_layout =
{
	32, 32, /* width, height */
	8,      /* total         */
	2,      /* planes        */
			/* plane offsets */
	{ 0, 1 },
	{
		0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
		0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
		0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
		0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E
	},
	{
		0x000, 0x040, 0x080, 0x0C0, 0x100, 0x140, 0x180, 0x1C0,
		0x200, 0x240, 0x280, 0x2C0, 0x300, 0x340, 0x380, 0x3C0,
		0x400, 0x440, 0x480, 0x4C0, 0x500, 0x540, 0x580, 0x5C0,
		0x600, 0x640, 0x680, 0x6C0, 0x700, 0x740, 0x780, 0x7C0
	},
	0x800
};


static const gfx_layout skyraid_missile_layout =
{
	16, 16, /* width, height */
	8,      /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
	},
	0x100
};


static GFXDECODE_START( gfx_skyraid )
	GFXDECODE_ENTRY( "gfx1", 0, skyraid_text_layout, 18, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, skyraid_sprite_layout, 8, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, skyraid_missile_layout, 16, 1 )
GFXDECODE_END


void skyraid_state::skyraid(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12096000 / 12);
	m_maincpu->set_addrmap(AS_PROGRAM, &skyraid_state::skyraid_map);
	m_maincpu->set_vblank_int("screen", FUNC(skyraid_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 4);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(22 * 1000000 / 15750));
	screen.set_size(512, 240);
	screen.set_visarea(0, 511, 0, 239);
	screen.set_screen_update(FUNC(skyraid_state::screen_update_skyraid));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skyraid);

	PALETTE(config, m_palette, FUNC(skyraid_state::skyraid_palette), 20);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, skyraid_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( skyraid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "030595.e1", 0x7000, 0x800, CRC(c6cb3a2b) SHA1(e4cb8d259446d0614c0c8f097f97dcf21869782e) )
	ROM_RELOAD(            0xF000, 0x800 )
	ROM_LOAD( "030594.d1", 0x7800, 0x800, CRC(27979e96) SHA1(55ffe3094c6764e6b99ee148e3dd730ca263fa3a) )
	ROM_RELOAD(            0xF800, 0x800 )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* alpha numerics */
	ROM_LOAD( "030598.h2", 0x0000, 0x200, CRC(2a7c5fa0) SHA1(93a79e5948dfcd9b6c2ff390e85a43f7a8cac327) )

	ROM_REGION( 0x0800, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "030599.m7", 0x0000, 0x800, CRC(0cd179ea) SHA1(e3c763f76e6103e5909e7b5a979206b262d6e96a) )

	ROM_REGION( 0x0100, "gfx3", 0 ) /* missiles */
	ROM_LOAD_NIB_LOW ( "030597.n5", 0x0000, 0x100, CRC(319ff49c) SHA1(ff4d8b20436179910bf30c720d98df4678f683a9) )
	ROM_LOAD_NIB_HIGH( "030596.m4", 0x0000, 0x100, CRC(30454ed0) SHA1(4216a54c13d9c4803f88f2de35cdee31290bb15e) )

	ROM_REGION( 0x0800, "user1", 0 ) /* terrain */
	ROM_LOAD_NIB_LOW ( "030584.j5", 0x0000, 0x800, CRC(81f6e8a5) SHA1(ad77b469ed0c9d5dfaa221ecf47d0db4a7f7ac91) )
	ROM_LOAD_NIB_HIGH( "030585.k5", 0x0000, 0x800, CRC(b49bec3f) SHA1(b55d25230ec11c52e7b47d2c10194a49adbeb50a) )

	ROM_REGION( 0x0100, "user2", 0 ) /* trapezoid */
	ROM_LOAD_NIB_LOW ( "030582.a6", 0x0000, 0x100, CRC(0eacd595) SHA1(5469e312a1f522ce0a61054b50895a5b1a3f19ba) )
	ROM_LOAD_NIB_HIGH( "030583.b6", 0x0000, 0x100, CRC(3edd6fbc) SHA1(0418ea78cf51e18c51087b43a41cd9e13aac0a16) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "006559.c4", 0x0200, 0x100, CRC(5a8d0e42) SHA1(772220c4c24f18769696ddba26db2bc2e5b0909d) ) /* sync */
ROM_END


GAME( 1978, skyraid, 0, skyraid, skyraid, skyraid_state, empty_init, ORIENTATION_FLIP_Y, "Atari", "Sky Raider", MACHINE_IMPERFECT_COLORS )
