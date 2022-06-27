// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Funny Bubble ...

It's a Puzzloop (Lup Lup Puzzle) rip-off .. but with two Z80 CPUs

The program roms say omega 1997
  Omega made Super Lup Lup Puzzle/Lup Lup Puzzle/Puzzle Bang Bang see vamphalf.c
  These games copy the game play of Puzzloop but add adult picture backgrounds.

The gfx roms say 1999
Title screen has no date

( a z80 as the main cpu in 1999 ??! )

Did In Chang Electronic Co do the "original" version of Funny Bubble?
 Comad either hacked or licensed it.  As "In Chang Electronic" are
 spelled out in the default high score table ;-)

todo :
   convert to tilemaps


 +-----------------------------------------+
 |    8MHz    M6295    SU12   UG1      UG3 |
 |                     SU13   UG2      UG4 |
++    6116           Z80                   |
|     SU6                                  |
|J    6116                            6116 |
|A    6116                    A1020B  6116 |
|M    6116         UM6264             6116 |
|M                                         |
|A SW2* SW4*                               |
|                                          |
|  SW1  SW3*                 UG13     UG16 |
++       UB16                UH13     UG16 |
 |       UM6264              UG15     UG17 |
 |       Z80  30MHz  12MHZ   UH15     UH17 |
 +-----------------------------------------+

   Z80: ZiLOG Z0840006PSC (6MHz rated) - Both Z80s
A1020B: Actel A1020B PL84C
 M6295: OKI M6295 (badged as AD-65)

Note: SW2, SW3 & SW4 not populated

*/



#include "emu.h"
#include "funybubl.h"

#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"


void funybubl_state::vidram_bank_w(uint8_t data)
{
	m_vrambank->set_bank(data & 1);
}

void funybubl_state::cpurombank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x3f);   // should we add a check that (data&0x3f) < #banks?
}

void funybubl_state::oki_bank_w(uint8_t data)
{
	m_okibank->set_entry(data & 1);
}


void funybubl_state::funybubl_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank"); // banked port 1?
	map(0xc400, 0xcfff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // palette
	map(0xd000, 0xdfff).m(m_vrambank, FUNC(address_map_bank_device::amap8)); // banked port 0?
	map(0xe000, 0xffff).ram();
}

void funybubl_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("SYSTEM").w(FUNC(funybubl_state::vidram_bank_w));    // vidram bank
	map(0x01, 0x01).portr("P1").w(FUNC(funybubl_state::cpurombank_w));     // rom bank?
	map(0x02, 0x02).portr("P2");
	map(0x03, 0x03).portr("DSW").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x06, 0x06).nopr();     /* Nothing is done with the data read */
	map(0x06, 0x06).nopw();        /* Written directly after IO port 0 */
	map(0x07, 0x07).nopw();        /* Reset something on startup - Sound CPU ?? */
}

void funybubl_state::vrambank_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().w(FUNC(funybubl_state::tilemap_w)).share("tilemapram");
	map(0x1000, 0x1fff).ram().share("spriteram");
}

/* Sound CPU */

void funybubl_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(funybubl_state::oki_bank_w));
	map(0x9800, 0x9800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void funybubl_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank");
}


static INPUT_PORTS_START( funybubl )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Maybe unused */

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x28, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x38, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Hard 1" )
	PORT_DIPSETTING(    0x18, "Hard 2" )
	PORT_DIPSETTING(    0x10, "Hard 3" )
	PORT_DIPSETTING(    0x08, "Hard 4" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Nudity" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Semi" )
	PORT_DIPSETTING(    0x00, "Full" )
INPUT_PORTS_END



static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(3,8), RGN_FRAC(2,8), RGN_FRAC(1,8), RGN_FRAC(0,8), RGN_FRAC(7,8), RGN_FRAC(6,8), RGN_FRAC(5,8), RGN_FRAC(4,8) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4)+4,RGN_FRAC(3,4)+0, RGN_FRAC(2,4)+4, RGN_FRAC(2,4)+0, RGN_FRAC(1,4)+4, RGN_FRAC(1,4)+0, RGN_FRAC(0,4)+4, RGN_FRAC(0,4)+0  },
	{ STEP4(0,1), STEP4(4*2,1), STEP4(16*4*2*2,1), STEP4(16*4*2*2+4*2,1) },
	{ STEP16(0,4*2*2) },
	32*16
};


static GFXDECODE_START( gfx_funybubl )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8,   0x100, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8, 0x000, 1 )
GFXDECODE_END



void funybubl_state::machine_start()
{
	m_mainbank->configure_entries(0, 0x10, memregion("maincpu")->base(), 0x4000);
	m_okibank->configure_entries(0, 2, memregion("oki")->base() + 0x20000, 0x20000);

	m_vrambank->set_bank(0);
}


void funybubl_state::funybubl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000/2);      /* 6 MHz?? */
	m_maincpu->set_addrmap(AS_PROGRAM, &funybubl_state::funybubl_map);
	m_maincpu->set_addrmap(AS_IO, &funybubl_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(funybubl_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8000000/2);      /* 4 MHz?? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &funybubl_state::sound_map);

	ADDRESS_MAP_BANK(config, m_vrambank).set_map(&funybubl_state::vrambank_map).set_options(ENDIANNESS_LITTLE, 8, 13, 0x1000);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(12*8, 512-12*8-1, 16, 256-16-1);
//  screen.set_visarea(0*8, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(funybubl_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_funybubl);
	PALETTE(config, m_palette).set_format(4, &funybubl_state::funybubl_R6B6G6, 0xc00/4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	OKIM6295(config, m_oki, 8000000/8, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &funybubl_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( funybubl )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main z80, lots of banked data */
	ROM_LOAD( "a.ub16", 0x00000, 0x40000, CRC(4e799cdd) SHA1(c6474fd2f621c27224e847ecb88a1ae17a0dbaf9)  )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT  ) // bg gfx 8x8x8
	ROM_LOAD( "f.ug13", 0x000000, 0x40000, CRC(64d7163d) SHA1(2619ac96e05779ea23c7f0f71665d284c79ba72f) )
	ROM_LOAD( "g.uh13", 0x040000, 0x40000, CRC(6891e2b8) SHA1(ca711019e5c330759d2a90024dbc0e6731b6227f) )
	ROM_LOAD( "h.ug15", 0x080000, 0x40000, CRC(ca7f7528) SHA1(6becfe8fabd19443a13b948838f41e10e5c9dc87) )
	ROM_LOAD( "i.uh15", 0x0c0000, 0x40000, CRC(23608ec6) SHA1(1c0a5d6e300f9a1abfda73d6a6a31e29a42b30ad) )
	ROM_LOAD( "l.ug16", 0x100000, 0x40000, CRC(0acf8143) SHA1(f49f45496b870d1f51f09a4dda8c5bb7763c40d3) )
	ROM_LOAD( "m.uh16", 0x140000, 0x40000, CRC(55ed8d9c) SHA1(f17bb4d02d4eedc2f297bb008be2fa340bc321d2) )
	ROM_LOAD( "n.ug17", 0x180000, 0x40000, CRC(52398b68) SHA1(522baa8123998e9161fa1ccaf760ac006c5be2dd) )
	ROM_LOAD( "o.uh17", 0x1c0000, 0x40000, CRC(446e31b2) SHA1(7f37a7090c83f2c9b07f1993707540fb32bbed35) )

	ROM_REGION( 0x200000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "d.ug1", 0x000000, 0x80000, CRC(b7ebbc00) SHA1(92520fda2f8f242b8cd49aeaac21b279f48276bf) ) /* Same as below, different labels */
	ROM_LOAD( "e.ug2", 0x080000, 0x80000, CRC(28afc396) SHA1(555d51948ffb237311112dcfd0516a43f603ff03) )
	ROM_LOAD( "j.ug3", 0x100000, 0x80000, CRC(9e8687cd) SHA1(42fcba2532ae5028fcfc1df50750d99ad2586820) )
	ROM_LOAD( "k.ug4", 0x180000, 0x80000, CRC(63f0e810) SHA1(5c7ed32ee8dc1d9aabc8d136ec370471096356c2) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) /* sound z80 (not much code here ..) */
	ROM_LOAD( "p.su6", 0x00000,  0x08000, CRC(33169d4d) SHA1(0ebc932d15b6df022c7e1f44df884e64b25ba745) ) /* Same as below, different label */

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "b.su12", 0x00000,  0x20000, CRC(a2d780f4) SHA1(bebba3db21ab9ddde8c6f19db3b67c869df582eb) ) /* Same as below, different label */
	ROM_LOAD( "c.su13", 0x20000,  0x40000, CRC(1f7e9269) SHA1(5c16b49a4e94aec7606d088c2d45a77842ab565b) ) /* Same as below, different label */
ROM_END

ROM_START( funybublc )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main z80, lots of banked data */
	ROM_LOAD( "2.ub16", 0x00000, 0x40000, CRC(d684c13f) SHA1(6a58b44dd775f374d6fd476a8fd175c28a83a495)  )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT  ) // bg gfx 8x8x8
	ROM_LOAD( "7.ug12",  0x000000, 0x40000, CRC(87603d7b) SHA1(21aec4cd011691f8608c3ddab83697bd89634fc8) )
	ROM_LOAD( "8.uh13",  0x040000, 0x40000, CRC(ab6031bd) SHA1(557793817f98c07c82caab4293aed7dffa4dbf7b) )
	ROM_LOAD( "9.ug15",  0x080000, 0x40000, CRC(0e8352ff) SHA1(29679a7ece2585e1a66296439b68bd56c937e313) )
	ROM_LOAD( "10.uh15", 0x0c0000, 0x40000, CRC(df7dd356) SHA1(13b9f40714dfa7b8cebc0191dcdde88b51f5e78c) )
	ROM_LOAD( "13.ug16", 0x100000, 0x40000, CRC(9f57bdd5) SHA1(6fd60da5f5eee0251e3a08957952ed9f037eeaec) )
	ROM_LOAD( "14.uh16", 0x140000, 0x40000, CRC(2ac15ea3) SHA1(de5be6378b4b6eee6faf532c9ef14bd609041cb3) )
	ROM_LOAD( "15.ug17", 0x180000, 0x40000, CRC(9a5e66a6) SHA1(cbe727e4f1e9a7072520d2e30eb0047cc67bff1b) )
	ROM_LOAD( "16.uh17", 0x1c0000, 0x40000, CRC(218060b3) SHA1(35124afce7f0f998b5c4761bbc888235de4e56ef) )

	ROM_REGION( 0x200000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "5.ug1",  0x000000, 0x80000, CRC(b7ebbc00) SHA1(92520fda2f8f242b8cd49aeaac21b279f48276bf) )
	ROM_LOAD( "6.ug2",  0x080000, 0x80000, CRC(28afc396) SHA1(555d51948ffb237311112dcfd0516a43f603ff03) )
	ROM_LOAD( "11.ug3", 0x100000, 0x80000, CRC(9e8687cd) SHA1(42fcba2532ae5028fcfc1df50750d99ad2586820) )
	ROM_LOAD( "12.ug4", 0x180000, 0x80000, CRC(63f0e810) SHA1(5c7ed32ee8dc1d9aabc8d136ec370471096356c2) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) /* sound z80 (not much code here ..) */
	ROM_LOAD( "1.su6", 0x00000,  0x08000, CRC(33169d4d) SHA1(0ebc932d15b6df022c7e1f44df884e64b25ba745) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3.su12", 0x00000,  0x20000, CRC(a2d780f4) SHA1(bebba3db21ab9ddde8c6f19db3b67c869df582eb) )
	ROM_LOAD( "4.su13", 0x20000,  0x40000, CRC(1f7e9269) SHA1(5c16b49a4e94aec7606d088c2d45a77842ab565b) )
ROM_END


GAME( 1999, funybubl, 0,        funybubl, funybubl, funybubl_state, empty_init, ROT0, "In Chang Electronic Co", "Funny Bubble", MACHINE_SUPPORTS_SAVE )
GAME( 1999, funybublc,funybubl, funybubl, funybubl, funybubl_state, empty_init, ROT0, "Comad", "Funny Bubble (Comad version)", MACHINE_SUPPORTS_SAVE )
