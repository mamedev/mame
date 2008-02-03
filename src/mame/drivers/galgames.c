/*
Galaxy Games

----

This is a multi-game coctail cabinet released in 1998.  Namco seems to have
made some cartridges for it (or at least licensed their IP).

Trackball-based.  'BIOS' has 7 built-in games.  There are two LEDs on the PCB.

More information here : http://www.cesgames.com/museum/galaxy/index.html

----

Board silkscreend  237-0211-00
                   REV.-D

Cartridge based mother board
Holds up to 4 cartridges
Chips labeled
    GALAXY U1 V1.90 12/1/98
    GALAXY U2 V1.90 12/1/98

NAMCO 307 Cartridge has surface mount Flash chips in it (not dumped?).

Looks like it requires a medium resolution monitor

Motorola MC68HC000FN12
24 MHz oscillator
Xilinx XC5206
Xilinx XC5202
BT481AKPJ110
NKK N341024SJ-15    x8  (128kb RAM)
OKI M6295 8092352-2

PAL16V8H-15 @ U24   Blue dot on it
PAL16V8H-15 @ U25   Yellow dot on it
PAL16V8H-15 @ U26   Red dot on it
PAL16V8H-15 @ U27   Green dot on it
PAL16V8H-15 @ U45   red dot on it

----

There are a few built-in games on the hardware, so the driver can grow mature without carts dumped.

TODO : This is a skeleton driver.  Just about everything.

*/

#include <stdio.h>
#include "driver.h"
#include "video/generic.h"

static READ16_HANDLER( dummy_read_01 )
{
	return 0x3;				// Pass the check at PC = 0x00000FAE & a later one
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
/*  AM_RANGE(0x01a900, 0x01a9ff) AM_RAM                         Need to install memory handlers
    ...                                                         in the middle of the ROM space?
    AM_RANGE(0x01f700, 0x01f7ff) AM_RAM */
	AM_RANGE(0x200000, 0x23ffff) AM_RAM							/* 2x N341024SJ-15 */
	AM_RANGE(0x400000, 0x400001) AM_RAM
	AM_RANGE(0x600000, 0x600001) AM_READ(dummy_read_01)
	AM_RANGE(0x700000, 0x700001) AM_RAM							/* Writes a 0x0002 here early, reads it later */
	AM_RANGE(0x900000, 0x900001) AM_NOP							/* Writes 0xffff here before each test memory access. */
	AM_RANGE(0xb00000, 0xb7ffff) AM_RAM							/* 4x N341024SJ-15 */
	AM_RANGE(0xd00000, 0xd00001) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( galgames )
INPUT_PORTS_END


static MACHINE_DRIVER_START( galgames )
	MDRV_CPU_ADD_TAG("main", M68000, XTAL_24MHz)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	//MDRV_CPU_VBLANK_INT(main_int, 2)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)
MACHINE_DRIVER_END


ROM_START( galgbios )
	ROM_REGION( 0x200000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "galaxy.u2", 0x000000, 0x100000, CRC(e51ff184) SHA1(aaa795f2c15ec29b3ceeb5c917b643db0dbb7083) )
	ROM_LOAD16_BYTE( "galaxy.u1", 0x000001, 0x100000, CRC(c6d7bc6d) SHA1(93c032f9aa38cbbdda59a8a25ff9f38f7ad9c760) )
ROM_END


/* System board BIOS */
/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT MONITOR COMPANY                FULLNAME             FLAGS */
GAME( 1998, galgbios, 0,      galgames, galgames, 0,   ROT0,   "Creative Electonics", "Galaxy Games BIOS", GAME_NOT_WORKING | GAME_IS_BIOS_ROOT )
