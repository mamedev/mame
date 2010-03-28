/*
  Unknown game, dump was marked 'slot 72 - poker'

  GFX roms contain
  'Extrema Systems International Ltd'
  as well as a logo for the company.

  There are also 'Lucky Boy' graphics in various places, which might be the title.


*/

#include "emu.h"
#include "cpu/z80/z80.h"



static ADDRESS_MAP_START( poker72_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( poker72 )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(0,4), RGN_FRAC(0,4)+4, RGN_FRAC(1,4), RGN_FRAC(1,4)+4 ,RGN_FRAC(2,4),RGN_FRAC(2,4)+4, RGN_FRAC(3,4),RGN_FRAC(3,4)+4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};



static GFXDECODE_START( poker72 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


VIDEO_START(poker72)
{

}

VIDEO_UPDATE(poker72)
{
	return 0;
}

static MACHINE_DRIVER_START( poker72 )


	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(poker72_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(poker72)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(poker72)
	MDRV_VIDEO_UPDATE(poker72)
MACHINE_DRIVER_END



ROM_START( poker72 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27010.bin", 0x00000, 0x20000, CRC(62447341) SHA1(e442c1f834a5dd2ab6ab3bdd316dfa86f2ca6647) )

	ROM_REGION( 0x1000, "89c51", 0 )
	ROM_LOAD( "89c51.bin", 0x00000, 0x1000, CRC(3fdd2148) SHA1(ea39a52482967268c7387aec77cfab1ae5c427fa) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "270135.bin", 0x00000, 0x20000, CRC(188c96ee) SHA1(7e883454cb080cdc82ce47ac92f51c8d45a55085) )
	ROM_LOAD( "270136.bin", 0x20000, 0x20000, CRC(f84c5068) SHA1(49178fe7b12f547a50879002236105a882767ebb) )
	ROM_LOAD( "270137.bin", 0x40000, 0x20000, CRC(310281d1) SHA1(c28f97bb3613c0b481ab6e16e215549c44b83c47) )
	ROM_LOAD( "270138.bin", 0x60000, 0x20000, CRC(d689313d) SHA1(8b9661b3af0e2ced7fe9fa487641e445ce7835b8) )
ROM_END

GAME( 199?, poker72,  0,    poker72, poker72,  0, ROT0, "Extrema Systems International Ltd.", "Lucky Boy / Poker 72", GAME_NOT_WORKING | GAME_NO_SOUND ) // actually unknown, was marked 'slot 72 poker'  Manufacturers logo and 'Lucky Boy' gfx in rom..
