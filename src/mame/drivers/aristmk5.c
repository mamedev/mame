/*

   Aristocrat MK5 / MKV hardware
   possibly 'Acorn Archimedes on a chip' hardware

   -- is it missing a bios rom?
   
*/

#include "driver.h"
#include "cpu/arm/arm.h"


static ADDRESS_MAP_START( aristmk5_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x01600000, 0x017fffff) AM_ROM AM_REGION("main", 0) // rom here, or a missing bios here?	
ADDRESS_MAP_END


static INPUT_PORTS_START( aristmk5 )
INPUT_PORTS_END

VIDEO_START(aristmk5)
{

}

VIDEO_UPDATE(aristmk5)
{
	return 0;
}

static MACHINE_DRIVER_START( aristmk5 )
	MDRV_CPU_ADD("main", ARM, 10000000) // ?
	MDRV_CPU_PROGRAM_MAP(aristmk5_map,0)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(aristmk5)
	MDRV_VIDEO_UPDATE(aristmk5)
MACHINE_DRIVER_END

ROM_START( reelrock )
	ROM_REGION( 0x200000, "main", 0 ) /* ARM Code */
	ROM_LOAD16_BYTE( "reelrock.u7",  0x000000, 0x80000, CRC(b60af34f) SHA1(1143380b765db234b3871c0fe04736472fde7de4) )
	ROM_LOAD16_BYTE( "reelrock.u11", 0x000001, 0x80000, CRC(57e341d0) SHA1(9b0d50763bb74ca5fe404c9cd526633721cf6677) )
	ROM_LOAD16_BYTE( "reelrock.u8",  0x100000, 0x80000, CRC(57eec667) SHA1(5f3888d75f48b6148f451d7ebb7f99e1a0939f3c) )
	ROM_LOAD16_BYTE( "reelrock.u12", 0x100001, 0x80000, CRC(4ac20679) SHA1(0ac732ffe6a33806e4a06e87ec875a3e1314e06b) )
ROM_END

ROM_START( indiandr )
	ROM_REGION( 0x200000, "main", 0 ) /* ARM Code */
	ROM_LOAD16_BYTE( "indiandr.u7",  0x000000, 0x80000, CRC(0c924a3e) SHA1(499b4ae601e53173e3ba5f400a40e5ae7bbaa043) )
	ROM_LOAD16_BYTE( "indiandr.u11", 0x000001, 0x80000, CRC(e371dc0f) SHA1(a01ab7fb63a19c144f2c465ecdfc042695124bdf) )
	ROM_LOAD16_BYTE( "indiandr.u8",  0x100000, 0x80000, CRC(1c6bfb47) SHA1(7f751cb499a6185a0ab64eeec511583ceeee6ee8) )
	ROM_LOAD16_BYTE( "indiandr.u12", 0x100001, 0x80000, CRC(4bbe67f6) SHA1(928f88387da66697f1de54f086531f600f80a15e) )
ROM_END

GAME( 1998, reelrock, 0, aristmk5, aristmk5, 0, ROT0,  "Aristocrat", "Reelin-n-Rockin (A - 13/07/98)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, indiandr, 0, aristmk5, aristmk5, 0, ROT0,  "Aristocrat", "Indian Dreaming (B - 15/12/98)", GAME_NOT_WORKING|GAME_NO_SOUND )
