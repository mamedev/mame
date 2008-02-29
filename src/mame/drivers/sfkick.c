/* Super Free Kick / Spinkick by HEC

 This driver is no longer being worked on by the original author,
 it would be best suited to somebody with knowledge of the MSX system,
 and possibly a working PCB to investigate the contents of the epoxy
 block which may, or may not be a suicide device.

 ----

 PCB contains a giant epoxy block, it is unknown what is inside, or
 if it is important to the emulation of the game.

 The game is thought to be based on MSX hardware as one of the roms
 is the MSX bios with various strings removed.

 The game appears to be an unofficial/unlicensed 'sequel' to Sega's
 Free Kick

*/

#include "driver.h"
#include "cpu/z80/z80.h"

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

#ifdef UNUSED_FUNCTION
static ADDRESS_MAP_START( readmem2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END
#endif

static INPUT_PORTS_START( sfkick )
    PORT_START
    PORT_DIPNAME(   0x01, 0x01, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x02, 0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x04, 0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x08, 0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x10, 0x10, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x20, 0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x40, 0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME(   0x80, 0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
INPUT_PORTS_END


static VIDEO_START( sfkick )
{
}

static VIDEO_UPDATE( sfkick )
{

	return 0;
}

static MACHINE_DRIVER_START( sfkick )

	MDRV_CPU_ADD(Z80,8000000) // ?
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
//  MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	//MDRV_CPU_ADD(Z80,8000000) // ?
	//MDRV_CPU_PROGRAM_MAP(readmem2,writemem2)
//  MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	//MDRV_GFXDECODE(sfkick) // GFX don't seem to be tile based
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(sfkick)
	MDRV_VIDEO_UPDATE(sfkick)
MACHINE_DRIVER_END

/*

Super Free Kick (c) ??Hec?? (Epoxy block is engraved with HEC)

Board # CBK1029. Not working

CPU: Epoxy block (40 pin, 8 pin connectors), Z80 (x2)
Sound: YM2203
RAM: 6264 (x2), 6116, 41464 (x4)
Other: 8255
X1: 21.47727 MHz

*/

ROM_START( sfkick )
	ROM_REGION( 0x8000*7, REGION_CPU1, 0 ) /* no attempt has been made to map these roms */
	ROM_LOAD( "sfkick1.c5", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
	ROM_LOAD( "sfkick2.a7", 0x08000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "sfkick3.c7", 0x10000, 0x8000, CRC(639d3cf2) SHA1(950fd28058d32e4532eb6e99454dcaef092a955e) )
	ROM_LOAD( "sfkick4.d7", 0x18000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	ROM_LOAD( "sfkick5.h7", 0x20000, 0x8000, CRC(8e8bd9cf) SHA1(f493de40147fdd67a48d4c90b01170fbd6ea074e) )
	ROM_LOAD( "sfkick6.j7", 0x28000, 0x8000, CRC(7a91ac4b) SHA1(afc5e2c2fe0cd108235ac6ae2775cc9a0b1c9f76) )
	ROM_LOAD( "sfkick7.l7", 0x30000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )
ROM_END

/*

Spinkick
Hec

Dumped by Thierry (ShinobiZ)
Board provided by Gerald (Coy)

CPU: Z80A
SND: Z80A + YM2203C

There is also an UM82C55A-PC

*/

ROM_START( spinkick )
	ROM_REGION( 0x8000*7, REGION_CPU1, 0 ) /* no attempt has been made to map these roms */
	ROM_LOAD( "spinkick.r1", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
	ROM_LOAD( "spinkick.r2", 0x08000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "spinkick.r3", 0x10000, 0x8000, CRC(e86a194a) SHA1(19a02375ec463e795770403c3e948d754919458b) ) // only this rom differs
	ROM_LOAD( "spinkick.r4", 0x18000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	ROM_LOAD( "spinkick.r5", 0x20000, 0x8000, CRC(8e8bd9cf) SHA1(f493de40147fdd67a48d4c90b01170fbd6ea074e) )
	ROM_LOAD( "spinkick.r6", 0x28000, 0x8000, CRC(7a91ac4b) SHA1(afc5e2c2fe0cd108235ac6ae2775cc9a0b1c9f76) )
	ROM_LOAD( "spinkick.r7", 0x30000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )
ROM_END

GAME( 199?, sfkick,   0,      sfkick, sfkick, 0, ROT90, "HEC", "Super Free Kick", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 199?, spinkick, sfkick, sfkick, sfkick, 0, ROT90, "HEC", "Spinkick", GAME_NOT_WORKING | GAME_NO_SOUND )
