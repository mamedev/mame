// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

News

from the program ROM:
PROGRAMED BY KWANG-HO CHO
COPYRIGHT(C) 1993
ALL RIGHTS RESERVED BY POBY
Hi-tel ID:poby:


driver by David Haywood

*/

#include "emu.h"
#include "includes/news.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


void news_state::news_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();     /* 4000-7fff is written to during startup, probably leftover code */
	map(0x8000, 0x87ff).ram().w(FUNC(news_state::news_fgram_w)).share("fgram");
	map(0x8800, 0x8fff).ram().w(FUNC(news_state::news_bgram_w)).share("bgram");
	map(0x9000, 0x91ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xc000, 0xc000).portr("DSW");
	map(0xc001, 0xc001).portr("INPUTS");
	map(0xc002, 0xc002).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc003, 0xc003).w(FUNC(news_state::news_bgpic_w));
	map(0xe000, 0xffff).ram();
}


static INPUT_PORTS_START( news )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Helps" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, "Copyright" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x00, "Poby" )
	PORT_DIPSETTING(    0x20, "Virus" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWA:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:8" )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( newsa )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Helps" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWA:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:8" )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END

static GFXDECODE_START( gfx_news )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb, 0, 16 )
GFXDECODE_END



void news_state::machine_start()
{
	save_item(NAME(m_bgpic));
}

void news_state::machine_reset()
{
	m_bgpic = 0;
}

void news_state::news(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &news_state::news_map);
	m_maincpu->set_vblank_int("screen", FUNC(news_state::irq0_line_hold));


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(news_state::screen_update_news));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_news);
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}



ROM_START( news )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "virus.4", 0x00000, 0x08000, BAD_DUMP CRC(aa005dfb) SHA1(52f4dd399a30568851d43d052b83cfaa6682665d)  ) /* The Original was too short, I padded it with 0xFF */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "virus.2", 0x00000, 0x40000, CRC(b5af58d8) SHA1(5dd8c6ab8b53df695463bd0c3620adf8c08daaec) )
	ROM_LOAD16_BYTE( "virus.3", 0x00001, 0x40000, CRC(a4b1c175) SHA1(b1ac0da4d91bc3a3454ea80aa4cdbbc68bbdf7f1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "virus.1", 0x00000, 0x40000, CRC(41f5935a) SHA1(1566d243f165019660cd4dd69df9f049e0130f15) )
ROM_END

ROM_START( newsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "newsa_virus.4", 0x00000, 0x10000, CRC(74a257da) SHA1(f1f6db521312f152ec3b1e6efa45a514433b2ffc) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "virus.2", 0x00000, 0x40000, CRC(b5af58d8) SHA1(5dd8c6ab8b53df695463bd0c3620adf8c08daaec) )
	ROM_LOAD16_BYTE( "virus.3", 0x00001, 0x40000, CRC(a4b1c175) SHA1(b1ac0da4d91bc3a3454ea80aa4cdbbc68bbdf7f1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "virus.1", 0x00000, 0x40000, CRC(41f5935a) SHA1(1566d243f165019660cd4dd69df9f049e0130f15) )
ROM_END

GAME( 1993, news,  0,    news, news,  news_state, empty_init, ROT0, "Poby / Virus", "News (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, newsa, news, news, newsa, news_state, empty_init, ROT0, "Poby",         "News (set 2)", MACHINE_SUPPORTS_SAVE )
