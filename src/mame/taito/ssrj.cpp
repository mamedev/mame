// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***********************************
 Super Speed Race Jr (c) 1985 Taito
 driver by  Tomasz Slanina


 TODO:
 - colors (missing proms)
 - dips
 - when a car sprite goes outside of the screen it gets stuck for a split frame on top of screen

HW info :

    0000-7fff ROM
    c000-dfff VRAM ( 4 tilemaps (4 x $800) )
    e000-e7ff RAM
    e800-efff SCROLL RAM
    f003      ??
  f400-f401 AY 8910
  fc00      ??
  f800      ??

 Scroll RAM contains x and y offsets for each tileline,
 as well as other data (priorities ? additional flags ?)
 All moving objects (cars, etc) are displayed on tilemap 3.

 ------------------------------------
 Cheat :  $e210 - timer

************************************/

#include "emu.h"
#include "ssrj.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


void ssrj_state::machine_start()
{
	save_item(NAME(m_oldport));
}

void ssrj_state::machine_reset()
{
	uint8_t *rom = memregion("maincpu")->base();

	memset(&rom[0xc000], 0 ,0x3fff); /* req for some control types */
	m_oldport = 0x80;
}

uint8_t ssrj_state::wheel_r()
{
	int port = ioport("IN1")->read() - 0x80;
	int retval = port - m_oldport;

	m_oldport = port;
	return retval;
}

void ssrj_state::ssrj_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().w(FUNC(ssrj_state::vram1_w)).share("vram1");
	map(0xc800, 0xcfff).ram().w(FUNC(ssrj_state::vram2_w)).share("vram2");
	map(0xd000, 0xd7ff).ram().share("vram3");
	map(0xd800, 0xdfff).ram().w(FUNC(ssrj_state::vram4_w)).share("vram4");
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share("scrollram");
	map(0xf000, 0xf000).portr("IN0");
	map(0xf001, 0xf001).r(FUNC(ssrj_state::wheel_r));
	map(0xf002, 0xf002).portr("IN2");
	map(0xf003, 0xf003).nopw(); /* unknown */
	map(0xf401, 0xf401).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xf400, 0xf401).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xf800, 0xf800).nopw(); /* wheel ? */
	map(0xfc00, 0xfc00).nopw(); /* unknown */
}

static INPUT_PORTS_START( ssrj )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xe0, 0x00, IPT_PEDAL ) PORT_MINMAX(0,0xe0) PORT_SENSITIVITY(50) PORT_KEYDELTA(0x20)

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("IN2")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED  ) PORT_CONDITION("IN3", 0x30, EQUALS, 0x00) /* code @ $eef, tested when controls != type1 */
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_START1 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Easy)")
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_START2 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Normal)")
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_START3 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Difficult)")
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_START4 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Very difficult)")
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING( 0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "No Hit" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Controls ) ) /* Type 1 has no start button and uses difficulty DSW, type 2-4 have 4 start buttons that determine difficulty. MT07492 for more.  */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x10, "Type 2" )
	PORT_DIPSETTING(    0x20, "Type 3" )
	PORT_DIPSETTING(    0x30, "Type 4" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* sometimes hangs after game over ($69b) */
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,3),  /* 1024 characters */
	3,  /* 3 bits per pixel */
	{ 0, RGN_FRAC(2,3), RGN_FRAC(1,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_ssrj )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 0x10 )
GFXDECODE_END

void ssrj_state::ssrj(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssrj_state::ssrj_map);
	m_maincpu->set_vblank_int("screen", FUNC(ssrj_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 34*8-1, 1*8, 31*8-1); // unknown res
	screen.set_screen_update(FUNC(ssrj_state::screen_update));
	screen.screen_vblank().set(FUNC(ssrj_state::screen_vblank));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ssrj);
	PALETTE(config, m_palette, FUNC(ssrj_state::ssrj_palette), 128);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 8000000/5));
	aysnd.port_b_read_callback().set_ioport("IN3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.30);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ssrj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a40-01.bin",   0x0000, 0x4000, CRC(1ff7dbff) SHA1(a9e676ee087141d62f880cd98e7748db1e6e9461) )
	ROM_LOAD( "a40-02.bin",   0x4000, 0x4000, CRC(bbb36f9f) SHA1(9f85bac639d18ee932273a6c00b36ac969e69bb8) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a40-03.bin",   0x0000, 0x2000, CRC(3753182a) SHA1(3eda34f967563b11416344da87b7be46cbecff2b) )
	ROM_LOAD( "a40-04.bin",   0x2000, 0x2000, CRC(96471816) SHA1(e24b690085602b8bde079e596c2879deab128c83) )
	ROM_LOAD( "a40-05.bin",   0x4000, 0x2000, CRC(dce9169e) SHA1(2cdda1453b2913fad931788e1db0bc01ce923a04) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "proms",  0x0000, 0x0100, NO_DUMP )

ROM_END

GAME( 1985, ssrj,  0,       ssrj,  ssrj, ssrj_state, empty_init, ROT90, "Taito Corporation", "Super Speed Race Junior (Japan)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
