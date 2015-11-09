// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Pass (c)1992, Oksan

 Driver by David Haywood
 Inputs by Stephh

 Is that a Korean flag I see?  Are Oksan maybe a Korean Developer?

 Information from ReadMe
 -----------------------

 Pass, Oksan, 1992
 CPU : MC68000P10
 Sound : YM2203C
 Other main chips : Goldstar Z8400B PS, 4 * Hyundai HY6264ALP-10
 Also : PAL20L8ACNS
 The rest all TTl chips

 8 dipswitches (1 bank)
 speed : 14.31818MHz
 Rest see pic
 (included was scans of the board)

 ----------------------

 Working Notes:

 68k interrupts
 lev 1 : 0x64 : 0000 0500 - vblank?
 lev 2 : 0x68 : 0000 0500 - vblank?
 lev 3 : 0x6c : 0000 0500 - vblank?
 lev 4 : 0x70 : 0000 0500 - vblank?
 lev 5 : 0x74 : 0000 0500 - vblank?
 lev 6 : 0x78 : 0000 0500 - vblank?
 lev 7 : 0x7c : 0000 0500 - vblank?
    (all point to the same place ..)

 z80 interrupts
 0x38 looks to have a valid IRQ
 0x66 might be valid NMI

 -- stephh's notes on the inputs --

 reads from 0x230100.w :

   0x001066 : mask = 0xe000 (coinage)

   0x00124e : mask = 0x0300 (player 1 lives)
   0x001292 : mask = 0x0300 (player 2 lives)

   0x0046ea : mask = 0x0001 (unknown effect - flip ? demo sounds ?) ->

   0x004182 : mask = 0x1800 (time, difficulty)


 reads from 0x230200.w :

   0x001000 : mask = 0xffff -> >>  0 in 0x080010
   0x001000 : mask = 0x00f0 -> >>  4 in 0x080016 (player 1 directions)
   0x001000 : mask = 0xf000 -> >> 12 in 0x080018 (player 2 directions)


 0x080010.w : inputs

   bit 00 : COIN1
   bit 01 : START1
   bit 02 : tested at 0x002bca, 0x002f00, 0x004c26, 0x00c25e, 0x001f48, 0x00c474, 0x00c628
   bit 03 : tested at 0x002bca, 0x002f00, 0x004c4c, 0x00c2ce, 0x00c644
   bit 04 : UP
   bit 05 : DOWN
   bit 06 : LEFT
   bit 07 : RIGHT

   bit 08 : COIN2
   bit 09 : START2
   bit 10 : tested at 0x002bca, 0x002f00, 0x004c8a, 0x00c2a8, 0x0021b8
   bit 11 : tested at 0x002bca, 0x002f00, 0x004cb0, 0x00c282
   bit 12 : UP
   bit 13 : DOWN
   bit 14 : LEFT
   bit 15 : RIGHT


 0x080014.w : credits (max = 0x005a)

 0x08007e.w : lives (player 1)
 0x080080.w : lives (player 2)

 0x080002.w : time (0x0000-0x0099, BCD coded)

 --- Game Notes ---

 Graphical Glitches caused when 2 sprites are close together are NOT bugs, the Sprites are
 infact contructed from a tilemap made of 4x4 tiles.

 I imagine flicker on the main character at times is also correct.

 Its rather interesting to see a game this old using 8bpp tiles


 */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "includes/pass.h"


/* todo: check all memory regions actually readable / read from */
static ADDRESS_MAP_START( pass_map, AS_PROGRAM, 16, pass_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_RAM_WRITE(pass_bg_videoram_w) AM_SHARE("bg_videoram") // Background
	AM_RANGE(0x210000, 0x213fff) AM_RAM_WRITE(pass_fg_videoram_w) AM_SHARE("fg_videoram") // Foreground
	AM_RANGE(0x220000, 0x2203ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x230000, 0x230001) AM_WRITE(soundlatch_word_w)
	AM_RANGE(0x230100, 0x230101) AM_READ_PORT("DSW")
	AM_RANGE(0x230200, 0x230201) AM_READ_PORT("INPUTS")
ADDRESS_MAP_END

/* sound cpu */
static ADDRESS_MAP_START( pass_sound_map, AS_PROGRAM, 8, pass_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pass_sound_io_map, AS_IO, 8, pass_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x70, 0x71) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE("oki", okim6295_device, write)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(soundlatch_clear_byte_w)
ADDRESS_MAP_END

/* todo : work out function of unknown but used dsw */
static INPUT_PORTS_START( pass )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown SW 0-0" )    // USED ! Check code at 0x0046ea
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unused SW 0-1" )     // Unused ?
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unused SW 0-2" )     // Unused ?
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Unused SW 0-3" )     // Unused ?
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unused SW 0-4" )     // Unused ?
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unused SW 0-5" )     // Unused ?
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unused SW 0-6" )     // Unused ?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unused SW 0-7" )     // Unused ?
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPNAME( 0x0400, 0x0400, "Unused SW 0-10" )    // Unused ?
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x0000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )         // Time = 99
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )           // Time = 88
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )         // Time = 77
	PORT_DIPSETTING(      0x1000, DEF_STR( Hardest ) )          // Time = 66
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1, 2,3, 4,5,6,7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

/* for something so simple this took a while to see */
static const gfx_layout tiles4x4_fg_layout =
{
	4,4,
	RGN_FRAC(1,1),
	8,
	{ 0,1, 2,3, 4,5,6,7 },
	{ 0, 8, 16, 24 },
	{ 0*32, 1*32, 2*32, 3*32 },
	4*32
};

static GFXDECODE_START( pass )
	GFXDECODE_ENTRY( "gfx1", 0, tiles4x4_fg_layout, 256, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 2 )
GFXDECODE_END

/* todo : is this correct? */
static MACHINE_CONFIG_START( pass, pass_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14318180/2 )
	MCFG_CPU_PROGRAM_MAP(pass_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pass_state,  irq1_line_hold) /* all the same */

	MCFG_CPU_ADD("audiocpu", Z80, 14318180/4 )
	MCFG_CPU_PROGRAM_MAP(pass_sound_map)
	MCFG_CPU_IO_MAP(pass_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(pass_state, irq0_line_hold, 60) /* probably not accurate, unknown timing and generation (ym2203 sound chip?). */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pass_state, screen_update_pass)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pass)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 14318180/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_OKIM6295_ADD("oki", 792000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END


ROM_START( pass )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "33", 0x00001, 0x20000, CRC(0c5f18f6) SHA1(49b60d46e4149ad1d49b044522a6888737c17e7d) )
	ROM_LOAD16_BYTE( "34", 0x00000, 0x20000, CRC(7b54573d) SHA1(251e99fa1f045ae4c90676e1953e49e8191440e4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* z80 clone? */
	ROM_LOAD( "23", 0x00000, 0x10000, CRC(b9a0ccde) SHA1(33e7dda247aa44b1933ae9c033c161c152276ce6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples? */
	ROM_LOAD( "31", 0x00000, 0x20000, CRC(c7315bbd) SHA1(c0bb392793cafc7b3f76da8fb26c6c16948f87e5) )

	ROM_REGION( 0x40000, "gfx1", 0 ) /* fg layer 'sprites' */
	ROM_LOAD16_BYTE( "35", 0x00000, 0x20000, CRC(2ab33f07) SHA1(23f2481450b3f43bbe3856c4cf595af74b1da2e0) )
	ROM_LOAD16_BYTE( "36", 0x00001, 0x20000, CRC(6677709d) SHA1(0d3df11097855294d606e46c0db0cf801c1dc28a) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* bg tiles */
	ROM_LOAD16_BYTE( "37", 0x40000, 0x20000, CRC(296499e7) SHA1(b7727f7942e20a2428df84e99075a572189a0096) )
	ROM_LOAD16_BYTE( "39", 0x40001, 0x20000, CRC(35c0ad5c) SHA1(78e3ca8b2e382a3c7bc53ede2ef5611c520ab095) )
	ROM_LOAD16_BYTE( "38", 0x00000, 0x20000, CRC(7f11b81a) SHA1(50253da7c13f9390fe7afd2faf17b8057f0bee1b) )
	ROM_LOAD16_BYTE( "40", 0x00001, 0x20000, CRC(80e0a71d) SHA1(e62c855f357e7492a59f8719c62a16d418dfa60b) )
ROM_END


GAME( 1992, pass, 0, pass, pass, driver_device, 0, ROT0, "Oksan", "Pass", MACHINE_SUPPORTS_SAVE )
