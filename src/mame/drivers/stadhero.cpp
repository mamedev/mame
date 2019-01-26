// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Stadium Hero (Japan)            (c) 1988 Data East Corporation

    Emulation by Bryan McPhail, mish@tendril.co.uk

=== PCB Info ===

  The OSC on the CPU board(DE-0303-3) is 20MHz
  The OSC on the video board(DE-0304-3) is 24MHz
  68000 =  20 / 2  (10)
  6502 =   24 / 16 (1.5)
  YM3812 = 24 / 8  (3)
  YM2203 = 24 / 16 (1.5)
  M6295 is driven by a 1.056MHz resonator, pin 7 is high
  HSync = 15.6246kHz
  VSync = 57.4434Hz

TODO : RNG issue? Some behavior isn't correct (ex: BGM randomizer).
    reference: https://youtu.be/6azneK6uUnA

***************************************************************************/

#include "emu.h"
#include "includes/stadhero.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


/******************************************************************************/

WRITE16_MEMBER(stadhero_state::int_ack_w)
{
	m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
}


/******************************************************************************/

void stadhero_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x2007ff).ram().w(FUNC(stadhero_state::pf1_data_w)).share("pf1_data");
	map(0x240000, 0x240007).w(m_tilegen, FUNC(deco_bac06_device::pf_control_0_w));                          /* text layer */
	map(0x240010, 0x240017).w(m_tilegen, FUNC(deco_bac06_device::pf_control_1_w));
	map(0x260000, 0x261fff).rw(m_tilegen, FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x30c000, 0x30c001).portr("INPUTS");
	map(0x30c002, 0x30c003).lr8("30c002", [this]() { return uint8_t(m_coin->read()); });
	map(0x30c004, 0x30c005).portr("DSW").w(FUNC(stadhero_state::int_ack_w));
	map(0x30c007, 0x30c007).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x310000, 0x3107ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xff8000, 0xffbfff).ram(); /* Main ram */
	map(0xffc000, 0xffc7ff).mirror(0x000800).ram().share("spriteram");
}

/******************************************************************************/

void stadhero_state::audio_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3812_device::write));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x3800, 0x3800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x8000, 0xffff).rom();
}

/******************************************************************************/

static INPUT_PORTS_START( stadhero )
	PORT_START("INPUTS")    /* 0x30c000 - 0x30c001 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")   /* 0x30c004 - 0x30c005 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x0010, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0300, 0x0300, "Time (1P Vs CPU)" )          /* Table at 0x0014f6 */
	PORT_DIPSETTING(      0x0200, "600" )
	PORT_DIPSETTING(      0x0300, "500" )
	PORT_DIPSETTING(      0x0100, "450" )
	PORT_DIPSETTING(      0x0000, "400" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time (1P Vs 2P)" )           /* Table at 0x0014fe */
	PORT_DIPSETTING(      0x0800, "270" )
	PORT_DIPSETTING(      0x0c00, "210" )
	PORT_DIPSETTING(      0x0400, "180" )
	PORT_DIPSETTING(      0x0000, "120" )
	PORT_DIPNAME( 0x3000, 0x3000, "Final Set" )                 /* Table at 0x00078c */
	PORT_DIPSETTING(      0x2000, "3 Credits" )
	PORT_DIPSETTING(      0x3000, "4 Credits" )
	PORT_DIPSETTING(      0x1000, "5 Credits" )
	PORT_DIPSETTING(      0x0000, "6 Credits" )
	PORT_DIPUNUSED( 0x4000, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x8000, IP_ACTIVE_LOW )

	PORT_START("COIN")  /* 0x30c002 & 0x30c003 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* related to music/sound */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* related to music/sound */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* related to music/sound */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* related to music/sound */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	RGN_FRAC(1,3),
	3,      /* 4 bits per pixel  */
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tile_3bpp =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static GFXDECODE_START( gfx_stadhero )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tile_3bpp,    512, 16 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 256, 16 ) /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

void stadhero_state::stadhero(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &stadhero_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(stadhero_state::irq5_line_assert));

	M6502(config, m_audiocpu, 24_MHz_XTAL/16);
	m_audiocpu->set_addrmap(AS_PROGRAM, &stadhero_state::audio_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh(HZ_TO_ATTOSECONDS(58));
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(stadhero_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_stadhero);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 1024);

	DECO_BAC06(config, m_tilegen, 0);
	m_tilegen->set_gfx_region_wide(1, 1, 2);
	m_tilegen->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, 0);
	m_spritegen->set_gfx_region(2);
	m_spritegen->set_gfxdecode_tag(m_gfxdecode);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch, 0);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 24_MHz_XTAL/16));
	ym1.add_route(0, "mono", 0.95);
	ym1.add_route(1, "mono", 0.95);
	ym1.add_route(2, "mono", 0.95);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 24_MHz_XTAL/8));
	ym2.irq_handler().set_inputline(m_audiocpu, M6502_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.80);
}

/******************************************************************************/

ROM_START( stadhero )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ef15.9a",  0x00000, 0x10000, CRC(bbba364e) SHA1(552096102f402085596635f02096462c6b8e13a7) )
	ROM_LOAD16_BYTE( "ef13.4e",  0x00001, 0x10000, CRC(97c6717a) SHA1(6c81260f49a59f70c71f520e51330a6833828684) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound */
	ROM_LOAD( "ef18.7f",  0x08000, 0x08000, CRC(20fd9668) SHA1(058e34a0ebfc372aaa9230c2bc9164ee2e85e217) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "ef08.2j",  0x00000, 0x10000, CRC(e84752fe) SHA1(9af2140ddbb44be793ab5b39787bac27f5b1c1f2) )  /* chars */
	ROM_LOAD( "ef09.4j",  0x10000, 0x08000, CRC(2ade874d) SHA1(5c884535214438a4ea79fd262700a346bc12ad81) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "ef11.13j", 0x00000, 0x10000, CRC(af563e96) SHA1(c88eaff4a1ea133d708f4511bb1dbc99ef066eed) )  /* tiles */
	ROM_LOAD( "ef10.11j", 0x10000, 0x10000, CRC(dca3d599) SHA1(2b97a70065f3065e7fbb54fb53cb120d9e5013b3) )
	ROM_LOAD( "ef12.14j", 0x20000, 0x10000, CRC(9a1bf51c) SHA1(e733c193b305496878551fc6eefc21587ba75c82) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ef00.2a",  0x00000, 0x10000, CRC(94ed257c) SHA1(caa4a4c8bf3b34d2288e117cfc704cca4c6f913b) )  /* sprites */
	ROM_LOAD( "ef01.4a",  0x10000, 0x10000, CRC(6eb9a721) SHA1(0f9dce614e67e57612e3a4ce187f0f9c12b78281) )
	ROM_LOAD( "ef02.5a",  0x20000, 0x10000, CRC(850cb771) SHA1(ccb54036191674d76965270a5831fba3e62f47c0) )
	ROM_LOAD( "ef03.7a",  0x30000, 0x10000, CRC(24338b96) SHA1(7730486bd0b84ba0a69b5547e348ee0058d4e7f1) )
	ROM_LOAD( "ef04.8a",  0x40000, 0x10000, CRC(9e3d97a7) SHA1(d02722376721caa5d8498f15f16959f42b75e7c1) )
	ROM_LOAD( "ef05.9a",  0x50000, 0x10000, CRC(88631005) SHA1(3c1787fb3aabdd9fecf679b2f4a9f833bf660885) )
	ROM_LOAD( "ef06.11a", 0x60000, 0x10000, CRC(9f47848f) SHA1(e23337684c8999483cbd11d3d953b06c34f13069) )
	ROM_LOAD( "ef07.12a", 0x70000, 0x10000, CRC(8859f655) SHA1(b3d69c5808b3ba7347ddb7f9693499903e9bfe6b) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ef17.1e",  0x00000, 0x10000, CRC(07c78358) SHA1(ce82b429eec0193fd9665b717336756a514db144) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "ef19.3d",  0x00000, 0x00200, CRC(852ff668) SHA1(d3053b68f86dcc81c3c3be280f75a4acd0b05be2) )  // ?
ROM_END

/******************************************************************************/

GAME( 1988, stadhero, 0, stadhero, stadhero, stadhero_state, empty_init, ROT0, "Data East Corporation", "Stadium Hero (Japan)", MACHINE_SUPPORTS_SAVE )
