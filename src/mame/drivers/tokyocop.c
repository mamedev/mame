/* Tokyo Cop

Italian Version

PC based hardware
Custom motherboard with
82815
82801
82562 (LAN)
RTM 560-25R (Audio)
TI4200 128Mb AGP
256 Mb PC133
Pentium 4 (??? XXXXMhz)

I/O Board with Altera Flex EPF15K50EQC240-3

*/

#include "driver.h"
#include "cpu/i386/i386.h"

static VIDEO_START(tokyocop)
{
}

static VIDEO_UPDATE(tokyocop)
{
	return 0;
}

static ADDRESS_MAP_START( tokyocop_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( tokyocop )
INPUT_PORTS_END


static MACHINE_DRIVER_START( tokyocop )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", PENTIUM, 2000000000) /* Pentium4? */
	MDRV_CPU_PROGRAM_MAP(tokyocop_map)

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(tokyocop)
	MDRV_VIDEO_UPDATE(tokyocop)
MACHINE_DRIVER_END


ROM_START(tokyocop)
	ROM_REGION32_LE(0x20000, "maincpu", 0)	/* motherboard bios */
	ROM_LOAD("tokyocop.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "tokyocop", 0, SHA1(a3cf011c8ef8ec80724c28e1534191b40ae8515d) )
ROM_END


GAME( 2003, tokyocop,  0,   tokyocop, tokyocop, 0, ROT0, "Gaelco", "Tokyo Cop (Italy)", GAME_NOT_WORKING|GAME_NO_SOUND )
