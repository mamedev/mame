// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

Vulgus memory map (preliminary)

driver by Mirko Buffoni

MAIN CPU
0000-9fff ROM
cc00-cc7f Sprites
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff background video RAM
dc00-dfff background color RAM
e000-efff RAM

read:
c000      IN0
c001      IN1
c002      IN2
c003      DSW1
c004      DSW2

write:
c802      background y scroll low 8 bits
c803      background x scroll low 8 bits
c805      background palette bank selector
c902      background y scroll high bit
c903      background x scroll high bit

SOUND CPU
0000-3fff ROM
4000-47ff RAM

write:
8000      AY-3-8910 #1 control
8001      AY-3-8910 #1 write
c000      AY-3-8910 #2 control
c001      AY-3-8910 #2 write

All Clocks and Vsync verified by Corrado Tomaselli (August 2012)

***************************************************************************/

#include "emu.h"
#include "vulgus.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


INTERRUPT_GEN_MEMBER(vulgus_state::vblank_irq)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xd7); /* Z80 - RST 10h - vblank */
}

void vulgus_state::main_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc801, 0xc801).nopw(); // ?
	map(0xc802, 0xc803).ram().share("scroll_low");
	map(0xc804, 0xc804).w(FUNC(vulgus_state::c804_w));
	map(0xc805, 0xc805).w(FUNC(vulgus_state::palette_bank_w));
	map(0xc902, 0xc903).ram().share("scroll_high");
	map(0xcc00, 0xcc7f).ram().share("spriteram");
	map(0xd000, 0xd7ff).ram().w(FUNC(vulgus_state::fgvideoram_w)).share("fgvideoram");
	map(0xd800, 0xdfff).ram().w(FUNC(vulgus_state::bgvideoram_w)).share("bgvideoram");
	map(0xe000, 0xefff).ram();
}

void vulgus_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xc000, 0xc001).w("ay2", FUNC(ay8910_device::address_data_w));
}


static INPUT_PORTS_START( vulgus )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	/* Only the parent set seems to use/see the second coin slot even if set to Cocktail mode */
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	/*  PORT_DIPSETTING(    0x00, "Invalid" ) disables both coins */
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:8" ) /* Shown as "Unused" in the manual, are 7 & 8 undocutmented Difficulty?? */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" ) /* Shown as "Unused" in the manual, Code performs a read then (& 0x03) */
	PORT_DIPNAME( 0x04, 0x04, "Demo Music" )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,3,2")
	PORT_DIPSETTING(    0x30, "10000 50000" )
	PORT_DIPSETTING(    0x50, "10000 60000" )
	PORT_DIPSETTING(    0x10, "10000 70000" )
	PORT_DIPSETTING(    0x70, "20000 60000" )
	PORT_DIPSETTING(    0x60, "20000 70000" )
	PORT_DIPSETTING(    0x20, "20000 80000" )
	PORT_DIPSETTING(    0x40, "30000 70000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};
static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};



static GFXDECODE_START( gfx_vulgus )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,           0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  64*4+16*16, 32*4 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,      64*4, 16 )
GFXDECODE_END




void vulgus_state::vulgus(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/4);  /* 3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &vulgus_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(vulgus_state::vblank_irq));

	Z80(config, m_audiocpu, XTAL(12'000'000)/4); /* 3 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &vulgus_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(vulgus_state::irq0_line_hold), attotime::from_hz(8*60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000)/2, 384, 128, 0, 262, 22, 246);  // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(vulgus_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vulgus);

	PALETTE(config, m_palette, FUNC(vulgus_state::vulgus_palette), 64*4+16*16+4*32*8, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.25); /* 1.5 MHz */

	AY8910(config, "ay2", XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.25); /* 1.5 MHz */
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vulgus ) /* Board ID# 84602-01A-1 */
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "vulgus.002",   0x0000, 0x2000, CRC(e49d6c5d) SHA1(48072aaa1f2603b6301d7542cc3df10ead2847bb) )
	ROM_LOAD( "vulgus.003",   0x2000, 0x2000, CRC(51acef76) SHA1(14dda82b90f9c3a309561a73c300cb54b5fca77d) )
	ROM_LOAD( "vulgus.004",   0x4000, 0x2000, CRC(489e7f60) SHA1(f3f685955fc42f238909dcdb5edc4c117e5543db) )
	ROM_LOAD( "vulgus.005",   0x6000, 0x2000, CRC(de3a24a8) SHA1(6bc9dda7dbbbef82e9f61c9d5cf1555e5290b249) )
	ROM_LOAD( "1-8n.bin",     0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1-11c.bin",    0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "1-3d.bin",     0x00000, 0x2000, CRC(8bc5d7a5) SHA1(c572b4a26f12013f5f6463b79ba9cbee4c474bbe) )   /* characters */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "2-2a.bin",     0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )   /* tiles */
	ROM_LOAD( "2-3a.bin",     0x02000, 0x2000, CRC(8da520da) SHA1(c4c633a909526308de4ad83e8ca449fa71eb3cb5) )
	ROM_LOAD( "2-4a.bin",     0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "2-5a.bin",     0x06000, 0x2000, CRC(b6d81984) SHA1(c935176f8a9bce0f74ff466e10c23ff6557f85ec) )
	ROM_LOAD( "2-6a.bin",     0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "2-7a.bin",     0x0a000, 0x2000, CRC(1e1ca773) SHA1(dbced07d4a886ed9ad3302aaa37bc02c599ee132) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "2-2n.bin",     0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )   /* sprites */
	ROM_LOAD( "2-3n.bin",     0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "2-4n.bin",     0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "2-5n.bin",     0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "e8.bin",       0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    /* red component */
	ROM_LOAD( "e9.bin",       0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    /* green component */
	ROM_LOAD( "e10.bin",      0x0200, 0x0100, CRC(de1fb621) SHA1(c719892f0c6d8c82ee2ff41bfe74b67648f5b4f5) )    /* blue component */
	ROM_LOAD( "d1.bin",       0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    /* char lookup table */
	ROM_LOAD( "j2.bin",       0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    /* sprite lookup table */
	ROM_LOAD( "c9.bin",       0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    /* tile lookup table */
	ROM_LOAD( "82s126.9k",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    /* interrupt timing? (not used) */
	ROM_LOAD( "82s129.8n",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( vulgusa )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "v2",           0x0000, 0x2000, CRC(3e18ff62) SHA1(03f61cc25b4c258effac2172f25641b668a1ae97) )
	ROM_LOAD( "v3",           0x2000, 0x2000, CRC(b4650d82) SHA1(4567dfe2b12c59f8c75f5198a136a9afe4975e09) )
	ROM_LOAD( "v4",           0x4000, 0x2000, CRC(5b26355c) SHA1(4220b70ad2bdfe269d4ac4e957114dbd3cea0975) )
	ROM_LOAD( "v5",           0x6000, 0x2000, CRC(4ca7f10e) SHA1(a3c278aecbb63063b660854ccef6fbaff7e58e32) )
	ROM_LOAD( "1-8n.bin",     0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1-11c.bin",    0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "1-3d.bin",     0x00000, 0x2000, CRC(8bc5d7a5) SHA1(c572b4a26f12013f5f6463b79ba9cbee4c474bbe) )   /* characters */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "2-2a.bin",     0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )   /* tiles */
	ROM_LOAD( "2-3a.bin",     0x02000, 0x2000, CRC(8da520da) SHA1(c4c633a909526308de4ad83e8ca449fa71eb3cb5) )
	ROM_LOAD( "2-4a.bin",     0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "2-5a.bin",     0x06000, 0x2000, CRC(b6d81984) SHA1(c935176f8a9bce0f74ff466e10c23ff6557f85ec) )
	ROM_LOAD( "2-6a.bin",     0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "2-7a.bin",     0x0a000, 0x2000, CRC(1e1ca773) SHA1(dbced07d4a886ed9ad3302aaa37bc02c599ee132) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "2-2n.bin",     0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )   /* sprites */
	ROM_LOAD( "2-3n.bin",     0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "2-4n.bin",     0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "2-5n.bin",     0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "e8.bin",       0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    /* red component */
	ROM_LOAD( "e9.bin",       0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    /* green component */
	ROM_LOAD( "e10.bin",      0x0200, 0x0100, CRC(de1fb621) SHA1(c719892f0c6d8c82ee2ff41bfe74b67648f5b4f5) )    /* blue component */
	ROM_LOAD( "d1.bin",       0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    /* char lookup table */
	ROM_LOAD( "j2.bin",       0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    /* sprite lookup table */
	ROM_LOAD( "c9.bin",       0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    /* tile lookup table */
	ROM_LOAD( "82s126.9k",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    /* interrupt timing? (not used) */
	ROM_LOAD( "82s129.8n",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( vulgusj )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "1-4n.bin",     0x0000, 0x2000, CRC(fe5a5ca5) SHA1(bb1b5ba5ce54f5e329d23fb8ad1357f65f10d6bf) )
	ROM_LOAD( "1-5n.bin",     0x2000, 0x2000, CRC(847e437f) SHA1(1d45ca0b92e7aa3099b8a61c27629d9bec3f25b8) )
	ROM_LOAD( "1-6n.bin",     0x4000, 0x2000, CRC(4666c436) SHA1(a2c921f30f91fead59c4d85d4a5ea8acbcfbf424) )
	ROM_LOAD( "1-7n.bin",     0x6000, 0x2000, CRC(ff2097f9) SHA1(49789c26a2adde043e5dba9d45a89509a211f05c) )
	ROM_LOAD( "1-8n.bin",     0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1-11c.bin",    0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "1-3d.bin",     0x00000, 0x2000, CRC(8bc5d7a5) SHA1(c572b4a26f12013f5f6463b79ba9cbee4c474bbe) )   /* characters */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "2-2a.bin",     0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )   /* tiles */
	ROM_LOAD( "2-3a.bin",     0x02000, 0x2000, CRC(8da520da) SHA1(c4c633a909526308de4ad83e8ca449fa71eb3cb5) )
	ROM_LOAD( "2-4a.bin",     0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "2-5a.bin",     0x06000, 0x2000, CRC(b6d81984) SHA1(c935176f8a9bce0f74ff466e10c23ff6557f85ec) )
	ROM_LOAD( "2-6a.bin",     0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "2-7a.bin",     0x0a000, 0x2000, CRC(1e1ca773) SHA1(dbced07d4a886ed9ad3302aaa37bc02c599ee132) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "2-2n.bin",     0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )   /* sprites */
	ROM_LOAD( "2-3n.bin",     0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "2-4n.bin",     0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "2-5n.bin",     0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "e8.bin",       0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    /* red component */
	ROM_LOAD( "e9.bin",       0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    /* green component */
	ROM_LOAD( "e10.bin",      0x0200, 0x0100, CRC(de1fb621) SHA1(c719892f0c6d8c82ee2ff41bfe74b67648f5b4f5) )    /* blue component */
	ROM_LOAD( "d1.bin",       0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    /* char lookup table */
	ROM_LOAD( "j2.bin",       0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    /* sprite lookup table */
	ROM_LOAD( "c9.bin",       0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    /* tile lookup table */
	ROM_LOAD( "82s126.9k",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    /* interrupt timing? (not used) */
	ROM_LOAD( "82s129.8n",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( mach9 )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "02_4n.bin",   0x0000, 0x2000, CRC(b3310b0c) SHA1(f083d8633da69acaa03e7566f28e87ef0482927d) )
	ROM_LOAD( "03_5n.bin",   0x2000, 0x2000, CRC(51acef76) SHA1(14dda82b90f9c3a309561a73c300cb54b5fca77d) )
	ROM_LOAD( "04_6n.bin",   0x4000, 0x2000, CRC(489e7f60) SHA1(f3f685955fc42f238909dcdb5edc4c117e5543db) )
	ROM_LOAD( "05_7n.bin",   0x6000, 0x2000, CRC(ef3e4278) SHA1(eb3433827a53b2e9c2b09add7376982e84b29558) )
	ROM_LOAD( "06_8n.bin",   0x8000, 0x2000, CRC(6ca5ca41) SHA1(6f28d143e984d3d6af3114702ec27d6e878cc35f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "07_11c.bin",  0x0000, 0x2000, CRC(3bd2acf4) SHA1(b58fb1ea7e30018102ee420d52a1597615412eb1) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "01_3d.bin",   0x00000, 0x2000, CRC(be556775) SHA1(16a4e746ea2462689b7a0e9f01c88d7edf06092d) )   /* characters */

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "08_2a.bin",   0x00000, 0x2000, CRC(e10aaca1) SHA1(f9f0d05475ae4c554552a71bc2f60e02b1442eb1) )   /* tiles */
	ROM_LOAD( "09_3a.bin",   0x02000, 0x2000, CRC(9193f2f1) SHA1(de1ee725627baeabec6823f6ecc4e7f6df152ce3) )
	ROM_LOAD( "10_4a.bin",   0x04000, 0x2000, CRC(206a13f1) SHA1(645666895127aededfa7872b20b7725948a9c462) )
	ROM_LOAD( "11_5a.bin",   0x06000, 0x2000, CRC(d729b5b7) SHA1(e9af9bc7f627e313ec070dc9e41ce6f2cddc5b38) )
	ROM_LOAD( "12_6a.bin",   0x08000, 0x2000, CRC(5a26b38f) SHA1(987a4844c4568a088932f43a3aff847e6d6b4860) )
	ROM_LOAD( "13_7a.bin",   0x0a000, 0x2000, CRC(8033cd4f) SHA1(5eb2e5931e44ca6bf64117dd34e9b6072e6b0ffc) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "14_2n.bin",   0x00000, 0x2000, CRC(6db1b10d) SHA1(85bf67ce4d60b260767ba5fe9b9777f857937fe3) )   /* sprites */
	ROM_LOAD( "15_3n.bin",   0x02000, 0x2000, CRC(5d8c34ec) SHA1(7b7df89398bf83ace1a8c216ca8526beae90972d) )
	ROM_LOAD( "16_4n.bin",   0x04000, 0x2000, CRC(0071a2e3) SHA1(3f7bb4658d2126576a0f8f46f2c947eec1cd231a) )
	ROM_LOAD( "17_5n.bin",   0x06000, 0x2000, CRC(4023a1ec) SHA1(8b69b9cd6db37db94a00da8712413055a631186a) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129_8e.bin",    0x0000, 0x0100, CRC(06a83606) SHA1(218c1b404b4b5b06f06e04143872f6758f83f266) )    /* red component */
	ROM_LOAD( "82s129_9e.bin",    0x0100, 0x0100, CRC(beacf13c) SHA1(d597097afc53fef752b2530d2de04e5aabb664b4) )    /* green component */
	ROM_LOAD( "82s129_10e.bin",   0x0200, 0x0100, CRC(8404067c) SHA1(6e8826f56267007e2adf02dc03dd96bd40e64935) )    /* blue component, only PROM slightly different from original? */
	ROM_LOAD( "82s129_1d.bin",    0x0300, 0x0100, CRC(7179080d) SHA1(6c1e8572a4c7b4825b89fc9549265be7c8f17788) )    /* char lookup table */
	ROM_LOAD( "82s129_2j.bin",    0x0400, 0x0100, CRC(d0842029) SHA1(7d76e1ff75466e190bc2e07ff3ffb45034f838cd) )    /* sprite lookup table */
	ROM_LOAD( "82s129_9c.bin",    0x0500, 0x0100, CRC(7a1f0bd6) SHA1(5a2110e97e82c087999ee4e5adf32d7fa06a3dfb) )    /* tile lookup table */
	ROM_LOAD( "82s129_9k.bin",    0x0600, 0x0100, CRC(32b10521) SHA1(10b258e32813cfa3a853cbd146657b11c08cb770) )    /* interrupt timing? (not used) */
	ROM_LOAD( "82s129_8n.bin",    0x0700, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

GAME( 1984, vulgus,  0,      vulgus, vulgus, vulgus_state, empty_init, ROT270, "Capcom", "Vulgus (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, vulgusa, vulgus, vulgus, vulgus, vulgus_state, empty_init, ROT90,  "Capcom", "Vulgus (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, vulgusj, vulgus, vulgus, vulgus, vulgus_state, empty_init, ROT270, "Capcom", "Vulgus (Japan?)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, mach9,   vulgus, vulgus, vulgus, vulgus_state, empty_init, ROT270,  "bootleg (ITISA)", "Mach-9 (bootleg of Vulgus)", MACHINE_SUPPORTS_SAVE )
