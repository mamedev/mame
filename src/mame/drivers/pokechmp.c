// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Poke Champ */

/* This is a Korean hack of Data East's Pocket Gal

   It uses RAM for Palette instead of PROMs
   Samples are played by OKIM6295
   Different Banking
   More Tiles, 8bpp
   Sprites 4bpp instead of 2bpp
   Many code changes

*/

/* README

-The ROMs are labeled as "Unico".
-The CPUs and some other chips are labeled as "SEA HUNTER".
-The chips with the "SEA HUNTER" label all have their
 surfaces scratched out, so I don't know what they are
 (all 40 -pin chips).

ROMs 1 to 4 = GFX?
ROMs 5 to 8 = Program?
ROM 9 = Sound CPU code?
ROM 10 = Sound samples?
ROM 11 = Main CPU code?

-There's a "copyright 1987 data east corp.all rights reserved"
 string inside ROM 11 (because it's a hack of Pocket Gal)

-Sound = Yamaha YM2203C + Y3014B

-Also, there are some GALs on the board (not dumped) a
 8-dips bank and two oscilators (4 MHz and 24 MHz, both near
 the sound parts).

ClawGrip, Jul 2006

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/pokechmp.h"

WRITE8_MEMBER(pokechmp_state::pokechmp_bank_w)
{
	UINT8 *ROM = memregion("maincpu")->base();

	int bank;

	bank  = (data & 0x1) ? 0x04000 : 0x00000;
	bank |= (data & 0x2) ? 0x10000 : 0x00000;

	membank("bank1")->set_base(&ROM[bank]);
}


WRITE8_MEMBER(pokechmp_state::pokechmp_sound_bank_w)
{
	UINT8 *ROM = memregion("oki")->base();
	membank("okibank")->set_base(&ROM[data*0x8000]);
}


WRITE8_MEMBER(pokechmp_state::pokechmp_sound_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}




static ADDRESS_MAP_START( pokechmp_map, AS_PROGRAM, 8, pokechmp_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(pokechmp_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1000, 0x11ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("P1")
	AM_RANGE(0x1801, 0x1801) AM_WRITE(pokechmp_flipscreen_w)
	/* 1800 - 0x181f are unused BAC-06 registers, see video/dec0.c */
	AM_RANGE(0x1802, 0x181f) AM_WRITENOP

	AM_RANGE(0x1a00, 0x1a00) AM_READ_PORT("P2") AM_WRITE(pokechmp_sound_w)
	AM_RANGE(0x1c00, 0x1c00) AM_READ_PORT("DSW") AM_WRITE(pokechmp_bank_w)

	/* Extra on Poke Champ (not on Pocket Gal) */
	AM_RANGE(0x2000, 0x23ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x2400, 0x27ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("fixed")
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( pokechmp_sound_map, AS_PROGRAM, 8, pokechmp_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym2", ym3812_device, write)
	AM_RANGE(0x1800, 0x1800) AM_WRITENOP    /* MSM5205 chip on Pocket Gal, not connected here? */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(pokechmp_sound_bank_w) /* sound rom bank seems to be replaced with OKI bank */
	AM_RANGE(0x2800, 0x2800) AM_DEVREADWRITE("oki", okim6295_device, read, write) // extra
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_byte_r)
//  AM_RANGE(0x3400, 0x3400) AM_READ(pokechmp_adpcm_reset_r) /* not on here */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank3")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( pokechmp_oki_map, AS_0, 8, pokechmp_state )
	AM_RANGE(0x00000, 0x37fff) AM_ROM
	AM_RANGE(0x38000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END


static INPUT_PORTS_START( pokechmp )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW")   /* Dip switch */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow 2 Players Game" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3") /* Affects Time: Normal=120 & Hardest=100 */
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2") /* Listed as "Number of Balls" in the manual */
	PORT_DIPSETTING(    0x00, "3" ) /* Manual shows 2 */
	PORT_DIPSETTING(    0x40, "4" ) /* Manual shows 3 */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") /* Not shown or listed in the manual */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout pokechmp_charlayout =
{
	8,8, /* 8*8 characters */
	RGN_FRAC(1,8),
	8,
	/* bizzare order, but it seems to be correct? */
	{ RGN_FRAC(1,8), RGN_FRAC(3,8),RGN_FRAC(0,8),RGN_FRAC(5,8),RGN_FRAC(2,8),RGN_FRAC(7,8),RGN_FRAC(4,8),RGN_FRAC(6,8) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8  /* every char takes 8 consecutive bytes */
};



static const gfx_layout pokechmp_spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),   /* 1024 sprites */
	4,
	{RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4)},
	{ 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0, 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( pokechmp )
	GFXDECODE_ENTRY( "bgs", 0x00000, pokechmp_charlayout,   0x100, 4 ) /* chars */
	GFXDECODE_ENTRY( "sprites", 0x00000, pokechmp_spritelayout,   0,  32 ) /* sprites */
GFXDECODE_END

/*

OSCs: 24MHz & 4MHz

Clocks - from Billiard List PCB
Main 6502: 1mhz
Sound 6502: 1mhz
YM2203: 1mhz
YM3014: 1.5mhz
OKI M6295 (an AD65 on this board, note pin 7 is low): 1.5mhz

*/

static MACHINE_CONFIG_START( pokechmp, pokechmp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(pokechmp_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pokechmp_state,  nmi_line_pulse)

	MCFG_CPU_ADD("audiocpu", M6502, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(pokechmp_sound_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pokechmp_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pokechmp_state, screen_update_pokechmp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pokechmp)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_4MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_SOUND_ADD("ym2", YM3812, XTAL_24MHz/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/16, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50) /* sound fx */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, pokechmp_oki_map)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(pokechmp_state,pokechmp)
{
	// default sound rom bank
	membank("bank3")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0x4000);

	// default fixed area for main CPU
	membank("fixed")->set_base( memregion("maincpu")->base() + 0x18000 );

	// default OKI sample bank
	membank("okibank")->set_base( memregion("oki")->base() + 0x40000 );
}


ROM_START( pokechmp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pokechamp_11_27010.bin", 0x00000, 0x20000, CRC(9afb6912) SHA1(e45da9524e3bb6f64a68200b70d0f83afe6e4379) )

	ROM_REGION( 0x18000, "audiocpu", 0 )     /* 96k for code + 96k for decrypted opcodes */
	ROM_LOAD( "pokechamp_09_27c512.bin",       0x10000, 0x8000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21) )
	ROM_CONTINUE(              0x08000, 0x8000 )

	ROM_REGION( 0x100000, "bgs", 0 )
	ROM_LOAD( "pokechamp_05_27c020.bin",       0x00000, 0x40000, CRC(554cfa42) SHA1(862d0dd83697da7bd52dc640c34926c62691afea) )
	ROM_LOAD( "pokechamp_06_27c020.bin",       0x40000, 0x40000, CRC(00bb9536) SHA1(1a5584297ebb425d6ce331955e0c6a4f467cd1e6) )
	ROM_LOAD( "pokechamp_07_27c020.bin",       0x80000, 0x40000, CRC(4b15ab5e) SHA1(5523134853b9ea1c81fd5aeb58061376d94e9298) )
	ROM_LOAD( "pokechamp_08_27c020.bin",       0xc0000, 0x40000, CRC(e9db54d6) SHA1(ac3b7c06d0f61847bf9bc6147f2f88d712f2b4b3) )

	ROM_REGION( 0x20000, "sprites", 0 )
	/* the first half of all these roms is identical.  For rom 3 both halves match.  Correct decode is to ignore the first half */
	ROM_LOAD( "pokechamp_02_27c512.bin",       0x00000, 0x08000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87) ) ROM_CONTINUE( 0x00000, 0x8000 )
	ROM_LOAD( "pokechamp_01_27c512.bin",       0x08000, 0x08000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c) ) ROM_CONTINUE( 0x08000, 0x8000 )
	ROM_LOAD( "pokechamp_04_27c512.bin",       0x10000, 0x08000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038) ) ROM_CONTINUE( 0x10000, 0x8000 )
	ROM_LOAD( "pokechamp_03_27c512.bin",       0x18000, 0x08000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5) ) ROM_CONTINUE( 0x18000, 0x8000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pokechamp_10_27c040.bin",       0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

// only the 'maincpu' and 'bgs' regions were dumped for this set, others assumed to be the same
ROM_START( pokechmpa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1", 0x00000, 0x20000, CRC(7d051c36) SHA1(8c2329f863ad677f4398a7dab7476c9492ad4f24) )

	ROM_REGION( 0x18000, "audiocpu", 0 )     /* 96k for code + 96k for decrypted opcodes */
	ROM_LOAD("pokechamp_09_27c512.bin", 0x10000, 0x8000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21))
	ROM_CONTINUE(              0x08000, 0x8000 )

	ROM_REGION( 0x100000, "bgs", 0 )
	ROM_LOAD( "6",       0x00000, 0x40000, CRC(1aec1de2) SHA1(f42db2445dcf1fb0957bf8a4414c3266ae47fae1) )
	ROM_LOAD( "5",       0x40000, 0x40000, CRC(79823f7a) SHA1(1059b4baf4d4d3c49d4de4194f29f8601e75972b) )
	ROM_LOAD( "4",       0x80000, 0x40000, CRC(e76f7596) SHA1(bb4c55bad2693da3f76d33fdf0f7f32c44dfd3e0) )
	ROM_LOAD( "3",       0xc0000, 0x40000, CRC(a22946b8) SHA1(d77fb5bfe00349753a9e6ea9de82c1eefca090f7) )

	ROM_REGION( 0x20000, "sprites", 0 )
	/* the first half of all these roms is identical.  For rom 3 both halves match.  Correct decode is to ignore the first half */
	ROM_LOAD( "pokechamp_02_27c512.bin",       0x00000, 0x08000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87) ) ROM_CONTINUE( 0x00000, 0x8000 )
	ROM_LOAD( "pokechamp_01_27c512.bin",       0x08000, 0x08000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c) ) ROM_CONTINUE( 0x08000, 0x8000 )
	ROM_LOAD( "pokechamp_04_27c512.bin",       0x10000, 0x08000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038) ) ROM_CONTINUE( 0x10000, 0x8000 )
	ROM_LOAD( "pokechamp_03_27c512.bin",       0x18000, 0x08000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5) ) ROM_CONTINUE( 0x18000, 0x8000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pokechamp_10_27c040.bin",       0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

// only the 'maincpu' and 'bgs' and 'oki' regions were dumped for this set, others assumed to be the same
ROM_START(billlist)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("billiard_list.1", 0x00000, 0x20000, CRC(4ef416f7) SHA1(e995410e2c79a3fbd2ac76a80dc6c412eb454e52) )

	ROM_REGION(0x18000, "audiocpu", 0)     /* 96k for code + 96k for decrypted opcodes */
	ROM_LOAD("pokechamp_09_27c512.bin", 0x10000, 0x8000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21))
	ROM_CONTINUE(0x08000, 0x8000)

	ROM_REGION(0x100000, "bgs", 0)
	ROM_LOAD("billiard_list.6", 0x00000, 0x40000, CRC(e674f7c0) SHA1(8a610a92ae141f3004497dc3ce102d07a178683f) )
	ROM_LOAD("billiard_list.5", 0x40000, 0x40000, CRC(f50d1510) SHA1(e09c6b1d0a5bdb44b3b5ae6bd54b169abd978af5) )
	ROM_LOAD("billiard_list.4", 0x80000, 0x40000, CRC(7466c0be) SHA1(6e82d9d8ec5bdeca2f6c7b9ca0f31aabf3faacea) )
	ROM_LOAD("billiard_list.3", 0xc0000, 0x40000, CRC(1ac4fa42) SHA1(2da5c6aa7e6b34ad1a2f052a41a2e607e2f904c2) )

	ROM_REGION(0x20000, "sprites", 0)
	/* the first half of all these roms is identical.  For rom 3 both halves match.  Correct decode is to ignore the first half */
	ROM_LOAD("pokechamp_02_27c512.bin", 0x00000, 0x08000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87)) ROM_CONTINUE(0x00000, 0x8000)
	ROM_LOAD("pokechamp_01_27c512.bin", 0x08000, 0x08000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c)) ROM_CONTINUE(0x08000, 0x8000)
	ROM_LOAD("pokechamp_04_27c512.bin", 0x10000, 0x08000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038)) ROM_CONTINUE(0x10000, 0x8000)
	ROM_LOAD("pokechamp_03_27c512.bin", 0x18000, 0x08000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5)) ROM_CONTINUE(0x18000, 0x8000)

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("billiard_list.x", 0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

GAME( 1995, pokechmp, 0,        pokechmp, pokechmp, pokechmp_state, pokechmp, ROT0, "D.G.R.M.", "Poke Champ (set 1)", 0 )
GAME( 1995, pokechmpa,pokechmp, pokechmp, pokechmp, pokechmp_state, pokechmp, ROT0, "D.G.R.M.", "Poke Champ (set 2)", 0 )
GAME( 1995, billlist, pokechmp, pokechmp, pokechmp, pokechmp_state, pokechmp, ROT0, "D.G.R.M.", "Billard List", 0)
