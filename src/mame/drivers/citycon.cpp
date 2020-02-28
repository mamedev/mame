// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

City Connection (c) 1985 Jaleco

2008-07
Dip locations added from dip listing at crazykong.com

***************************************************************************/

#include "emu.h"
#include "includes/citycon.h"

#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "screen.h"
#include "speaker.h"

#define MASTER_CLOCK (20_MHz_XTAL)
#define PIXEL_CLOCK  (MASTER_CLOCK/4) // guess
#define CPU_CLOCK    (8_MHz_XTAL)

/* also a guess, need to extract PAL equations to get further answers */
#define HTOTAL       (320)
#define HBEND        (8)
#define HBSTART      (248)
#define VTOTAL       (262)
#define VBEND        (16)
#define VBSTART      (240)

READ8_MEMBER(citycon_state::citycon_in_r)
{
	return ioport(flip_screen() ? "P2" : "P1")->read();
}

READ8_MEMBER(citycon_state::citycon_irq_ack_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}

void citycon_state::citycon_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(citycon_state::citycon_videoram_w)).share("videoram");
	map(0x2000, 0x20ff).ram().w(FUNC(citycon_state::citycon_linecolor_w)).share("linecolor").mirror(0x0700);
	map(0x2800, 0x28ff).ram().share("spriteram").mirror(0x0700); //0x2900-0x2fff cleared at post but unused
	map(0x3000, 0x3000).r(FUNC(citycon_state::citycon_in_r)).w(FUNC(citycon_state::citycon_background_w));   /* player 1 & 2 inputs multiplexed */
	map(0x3001, 0x3001).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x3002, 0x3002).portr("DSW2").w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0x3004, 0x3005).nopr().writeonly().share("scroll");
	map(0x3007, 0x3007).r(FUNC(citycon_state::citycon_irq_ack_r));
	map(0x3800, 0x3cff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x4000, 0xffff).rom();
}

void citycon_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x4000, 0x4001).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x4002, 0x4002).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x6000, 0x6001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x8000, 0xffff).rom();
}



static INPUT_PORTS_START( citycon )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	/* the coin input must stay low for exactly 2 frames to be consistently recognized. */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW2:7" )
	/* According to manual this is Flip Screen setting */
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip Screen ) ) PORT_DIPLOCATION("SW2:8")
//  PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	5,
	{ 16, 12, 8, 4, 0 },
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*24, 1*24, 2*24, 3*24, 4*24, 5*24, 6*24, 7*24 },
	24*8
};

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	4,  /* 4 bits per pixel */
	{ 4, 0, 0xc000*8+4, 0xc000*8+0 },
	{ 0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,16,   /* 8*16 sprites */
	128,    /* 128 sprites */
	4,  /* 4 bits per pixel */
	{ 4, 0, 0x2000*8+4, 0x2000*8+0 },
	{ 0, 1, 2, 3, 128*16*8+0, 128*16*8+1, 128*16*8+2, 128*16*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    /* every sprite takes 16 consecutive bytes */
};


static GFXDECODE_START( gfx_citycon )
//  GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 512, 32 ) /* colors 512-639 */
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 640, 32 ) /* colors 512-639 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout, 0, 16 ) /* colors 0-255 */
	GFXDECODE_ENTRY( "gfx2", 0x01000, spritelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, tilelayout, 256, 16 ) /* colors 256-511 */
	GFXDECODE_ENTRY( "gfx3", 0x01000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x02000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x03000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x04000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x05000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x06000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x07000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x08000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x09000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x0a000, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x0b000, tilelayout, 256, 16 )
GFXDECODE_END

void citycon_state::machine_start()
{
	save_item(NAME(m_bg_image));
}

void citycon_state::machine_reset()
{
	m_bg_image = 0;
}


void citycon_state::citycon(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, CPU_CLOCK); // HD68B09P
	m_maincpu->set_addrmap(AS_PROGRAM, &citycon_state::citycon_map);
	m_maincpu->set_vblank_int("screen", FUNC(citycon_state::irq0_line_assert));

	mc6809e_device &audiocpu(MC6809E(config, "audiocpu", MASTER_CLOCK / 32)); // schematics allow for either a 6809 or 6809E; HD68A09EP found on one actual PCB
	audiocpu.set_addrmap(AS_PROGRAM, &citycon_state::sound_map);
	audiocpu.set_vblank_int("screen", FUNC(citycon_state::irq0_line_hold)); // actually unused, probably it was during development

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(citycon_state::screen_update_citycon));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_citycon);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::RGBx_444, 640+1024);   // 640 real palette + 1024 virtual palette

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	AY8910(config, "aysnd", MASTER_CLOCK / 16).add_route(ALL_OUTPUTS, "mono", 0.40); // schematics consistently specify AY-3-8910, though YM2149 found on one actual PCB

	ym2203_device &ymsnd(YM2203(config, "ymsnd", MASTER_CLOCK / 16));
	ymsnd.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ymsnd.port_b_read_callback().set("soundlatch2", FUNC(generic_latch_8_device::read));
	ymsnd.add_route(0, "mono", 0.40);
	ymsnd.add_route(1, "mono", 0.40);
	ymsnd.add_route(2, "mono", 0.40);
	ymsnd.add_route(3, "mono", 0.20);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( citycon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c10",          0x4000, 0x4000, CRC(ae88b53c) SHA1(dd12310bd9c9b93462446e8e0a1c853506bf3aa1) )
	ROM_LOAD( "c11",          0x8000, 0x8000, CRC(139eb1aa) SHA1(c570e8ca1499f7ea61938e78c32c1cc3050ca2b7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c1",           0x8000, 0x8000, CRC(1fad7589) SHA1(2e626bbbab8cffe11ee7de3e56aa1871c29d5fa9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "c4",           0x00000, 0x2000, CRC(a6b32fc6) SHA1(d99d5a527440e9a91525c1084b95b213e3b760ec) )   /* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "c12",          0x00000, 0x2000, CRC(08eaaccd) SHA1(a970381e3ba22bcdea6df2d31cd8a10c4b2bc413) )   /* Sprites    */
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(1819aafb) SHA1(8a5ffcd8866e09c5568879257384767d61796111) )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "c9",           0x00000, 0x8000, CRC(8aeb47e6) SHA1(bb09dbe6b37e1bd02abf3024ac4d954c8f0e70f2) )   /* Background tiles */
	ROM_LOAD( "c8",           0x08000, 0x4000, CRC(0d7a1eeb) SHA1(60b8d4124ce857a248d3c41fdb050f11be58549f) )
	ROM_LOAD( "c6",           0x0c000, 0x8000, CRC(2246fe9d) SHA1(f7f8708d499bcbd1a583e1092b54425ad1105f94) )
	ROM_LOAD( "c7",           0x14000, 0x4000, CRC(e8b97de9) SHA1(f4d1b7075f47ab4522c36281b97eaa02fe383814) )

	ROM_REGION( 0xe000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "c2",           0x0000, 0x8000, CRC(f2da4f23) SHA1(5ea1a51c3ac283796f7eafb6719d88356767340d) )    /* background maps */
	ROM_LOAD( "c3",           0x8000, 0x4000, CRC(7ef3ac1b) SHA1(8a0497c4e4733f9c50d576f632210b82497a5e1c) )
	ROM_LOAD( "c5",           0xc000, 0x2000, CRC(c03d8b1b) SHA1(641c1eba334d36ea64b9293a20320b31c7c88858) )    /* color codes for the background */
ROM_END

ROM_START( citycona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c10",          0x4000, 0x4000, CRC(ae88b53c) SHA1(dd12310bd9c9b93462446e8e0a1c853506bf3aa1) )
	ROM_LOAD( "c11b",         0x8000, 0x8000, CRC(d64af468) SHA1(5bb3541af3ce632e8eca313231205713d72fb9dc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c1",           0x8000, 0x8000, CRC(1fad7589) SHA1(2e626bbbab8cffe11ee7de3e56aa1871c29d5fa9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "c4",           0x00000, 0x2000, CRC(a6b32fc6) SHA1(d99d5a527440e9a91525c1084b95b213e3b760ec) )   /* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "c12",          0x00000, 0x2000, CRC(08eaaccd) SHA1(a970381e3ba22bcdea6df2d31cd8a10c4b2bc413) )   /* Sprites    */
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(1819aafb) SHA1(8a5ffcd8866e09c5568879257384767d61796111) )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "c9",           0x00000, 0x8000, CRC(8aeb47e6) SHA1(bb09dbe6b37e1bd02abf3024ac4d954c8f0e70f2) )   /* Background tiles */
	ROM_LOAD( "c8",           0x08000, 0x4000, CRC(0d7a1eeb) SHA1(60b8d4124ce857a248d3c41fdb050f11be58549f) )
	ROM_LOAD( "c6",           0x0c000, 0x8000, CRC(2246fe9d) SHA1(f7f8708d499bcbd1a583e1092b54425ad1105f94) )
	ROM_LOAD( "c7",           0x14000, 0x4000, CRC(e8b97de9) SHA1(f4d1b7075f47ab4522c36281b97eaa02fe383814) )

	ROM_REGION( 0xe000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "c2",           0x0000, 0x8000, CRC(f2da4f23) SHA1(5ea1a51c3ac283796f7eafb6719d88356767340d) )    /* background maps */
	ROM_LOAD( "c3",           0x8000, 0x4000, CRC(7ef3ac1b) SHA1(8a0497c4e4733f9c50d576f632210b82497a5e1c) )
	ROM_LOAD( "c5",           0xc000, 0x2000, CRC(c03d8b1b) SHA1(641c1eba334d36ea64b9293a20320b31c7c88858) )    /* color codes for the background */

	/* stuff below isn't used but loaded because it was on the board .. */
	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "citycon_82s123n.n5",  0x0000, 0x0020, CRC(5ae142a3) SHA1(ba25c9bcbc4936a6b7f402addab50b75dbe519ce) )
	ROM_LOAD( "citycon_82s123n.r4",  0x0100, 0x0020, CRC(29221e13) SHA1(232fd02811f157197c7ce44716dc495ed49a80cc) )
	ROM_LOAD( "citycon_82s129.l6",   0x0200, 0x0100, CRC(91a7b6e3) SHA1(6135b264a69978d17aa8636d24eb1eba41d16c89) )

		// Same PROM content on J10 and L6 sockets
	//ROM_LOAD( "citycon_82s129.j10", 0x0300, 0x0100, CRC(91a7b6e3) SHA1(6135b264a69978d17aa8636d24eb1eba41d16c89) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "citycon_pal16l8a.h7", 0x0000, 0x0104, CRC(24a0f5d4) SHA1(69007ccbe1966b1a1d4378fe06e102598c3cdb09) )
	ROM_LOAD( "citycon_pal16l8a.l5", 0x0200, 0x0104, CRC(dd0cf771) SHA1(4483da095ce7633d8e0c90f181a54ace19a6e87d) )
	ROM_LOAD( "citycon_pal16l8a.u7", 0x0400, 0x0104, CRC(08d4ff84) SHA1(10a2b985e0866661c4f0ce4297f728f07540e9b6) )
ROM_END

ROM_START( cruisin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cr10",         0x4000, 0x4000, CRC(cc7c52f3) SHA1(69d76f146fb1dac62c6def3a4269012b3880f03b) )
	ROM_LOAD( "cr11",         0x8000, 0x8000, CRC(5422f276) SHA1(d384fc4f853fe79b73e939a8fc7b7af780659c5e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c1",           0x8000, 0x8000, CRC(1fad7589) SHA1(2e626bbbab8cffe11ee7de3e56aa1871c29d5fa9) )

	ROM_REGION( 0x03000, "gfx1", 0 )
	ROM_LOAD( "cr4",          0x00000, 0x2000, CRC(8cd0308e) SHA1(7303b9e074bda557d64b39e04cef0f965a756be6) )   /* Characters */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "c12",          0x00000, 0x2000, CRC(08eaaccd) SHA1(a970381e3ba22bcdea6df2d31cd8a10c4b2bc413) )   /* Sprites    */
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(1819aafb) SHA1(8a5ffcd8866e09c5568879257384767d61796111) )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "c9",           0x00000, 0x8000, CRC(8aeb47e6) SHA1(bb09dbe6b37e1bd02abf3024ac4d954c8f0e70f2) )   /* Background tiles */
	ROM_LOAD( "c8",           0x08000, 0x4000, CRC(0d7a1eeb) SHA1(60b8d4124ce857a248d3c41fdb050f11be58549f) )
	ROM_LOAD( "c6",           0x0c000, 0x8000, CRC(2246fe9d) SHA1(f7f8708d499bcbd1a583e1092b54425ad1105f94) )
	ROM_LOAD( "c7",           0x14000, 0x4000, CRC(e8b97de9) SHA1(f4d1b7075f47ab4522c36281b97eaa02fe383814) )

	ROM_REGION( 0xe000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "c2",           0x0000, 0x8000, CRC(f2da4f23) SHA1(5ea1a51c3ac283796f7eafb6719d88356767340d) )    /* background maps */
	ROM_LOAD( "c3",           0x8000, 0x4000, CRC(7ef3ac1b) SHA1(8a0497c4e4733f9c50d576f632210b82497a5e1c) )
	ROM_LOAD( "c5",           0xc000, 0x2000, CRC(c03d8b1b) SHA1(641c1eba334d36ea64b9293a20320b31c7c88858) )    /* color codes for the background */
ROM_END



void citycon_state::init_citycon()
{
	uint8_t *rom = memregion("gfx1")->base();

	/*
	  City Connection controls the text color code for each _scanline_, not
	  for each character as happens in most games. To handle that conveniently,
	  I convert the 2bpp char data into 5bpp, and create a virtual palette so
	  characters can still be drawn in one pass.
	  */
	for (int i = 0x0fff; i >= 0; i--)
	{
		rom[3 * i] = rom[i];
		rom[3 * i + 1] = 0;
		rom[3 * i + 2] = 0;
		int mask = rom[i] | (rom[i] << 4) | (rom[i] >> 4);
		if (i & 0x01) rom[3 * i + 1] |= mask & 0xf0;
		if (i & 0x02) rom[3 * i + 1] |= mask & 0x0f;
		if (i & 0x04) rom[3 * i + 2] |= mask & 0xf0;
	}
}



GAME( 1985, citycon,  0,       citycon, citycon, citycon_state, init_citycon, ROT0, "Jaleco", "City Connection (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, citycona, citycon, citycon, citycon, citycon_state, init_citycon, ROT0, "Jaleco", "City Connection (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, cruisin,  citycon, citycon, citycon, citycon_state, init_citycon, ROT0, "Jaleco (Kitkorp license)", "Cruisin", MACHINE_SUPPORTS_SAVE )
