/* Queen */

/*

Produttore  STG
N.revisione
CPU main PCB is a standard EPIA
ROMs    epia BIOS + solid state HD



 ------- better component list should be added here!!

 it's a 2002 era PC at least based on the BIOS,
  almost certainly newer than the standard 'PENTIUM' CPU

*/

#include "driver.h"
#include "cpu/i386/i386intf.h"

VIDEO_START(queen)
{

}

VIDEO_UPDATE(queen)
{
	return 0;
}

static ADDRESS_MAP_START( queen_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(queen_io, ADDRESS_SPACE_IO, 32)
ADDRESS_MAP_END


static INPUT_PORTS_START( queen )
INPUT_PORTS_END


static MACHINE_DRIVER_START( queen )
	MDRV_CPU_ADD("main", PENTIUM, 400000000) // no idea
	MDRV_CPU_PROGRAM_MAP(queen_map, 0)
	MDRV_CPU_IO_MAP(queen_io, 0)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(queen)
	MDRV_VIDEO_UPDATE(queen)
MACHINE_DRIVER_END




ROM_START( queen )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "bios-original.bin", 0x00000, 0x40000, CRC(feb542d4) SHA1(3cc5d8aeb0e3b7d9ed33248a4f3dc507d29debd9) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "pqiidediskonmodule", 0,SHA1(9134791a72c2218ddfd0e575e4c06743a02d0edc) MD5(fffff5010746e9e49b2f3d55ac9ffcb8) )
ROM_END


GAME( 2002?, queen,  0,    queen, queen,  0, ROT0, "STG?", "Queen?", GAME_NOT_WORKING|GAME_NO_SOUND )
