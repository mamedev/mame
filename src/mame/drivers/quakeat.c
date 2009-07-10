/*

 Quake Arcade Tournament

 This is unknown PC hardware, only the HDD is dumped.  The HDD is stickered 'Release Beta 2'

 I've also seen CDs of this for sale, so maybe there should be a CD too, for the music?



 -- set info

Quake Arcade Tournament by Lazer-Tron

PC running Windows 95 with a Dongle on the parallel port

Created .chd with version 0.125

It found the following disk paramaters...

Input offset    511
Cyclinders  263
Heads       255
Sectors     63
Byte/Sector 512
Sectors/Hunk    8
Logical size    2,1163,248,864


The "backup" directory on hard disk was created by me.


*/

#include "driver.h"
#include "cpu/i386/i386.h"

static VIDEO_START(quake)
{
}

static VIDEO_UPDATE(quake)
{
	return 0;
}

static ADDRESS_MAP_START( quake_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( quake )
INPUT_PORTS_END


static MACHINE_DRIVER_START( quake )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", PENTIUM, 2000000000) /* Probably a Pentium .. ?? Mhz*/
	MDRV_CPU_PROGRAM_MAP(quake_map)

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(quake)
	MDRV_VIDEO_UPDATE(quake)
MACHINE_DRIVER_END


ROM_START(quake)
	ROM_REGION32_LE(0x20000, "maincpu", 0)	/* motherboard bios */
	ROM_LOAD("quakearcadetournament.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "quakeat", 0, SHA1(c44695b9d521273c9d3c0e18c88f0dca0185bd7b) )
ROM_END


GAME( 19??, quake,  0,   quake, quake, 0, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
