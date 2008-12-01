/* bra$il (me am) */

/*

CPU	1x

N80C186XL25 (main)(u1)
1x ispLSI2032-80LJ (u13)(not dumped)
1x ispLSI1032E-70LJ (u18)(not dumped)
1x M6376 (sound)(u17)
1x oscillator 40.000MHz

ROMs
1x MX27C4000 (u16)
2x M27C801 (u7,u8)

Note

1x 28x2 edge connector (cn1)
1x 5 legs connector (cn2)
1x 8 legs connector (cn3)
1x trimmer (volume)
1x pushbutton (k1)
1x battery (b1)


cpu is 80186 based (with extras), see
http://media.digikey.com/pdf/Data%20Sheets/Intel%20PDFs/80C186XL,%2080C188XL.pdf

*/


#include "driver.h"

VIDEO_START(brasil)
{

}

VIDEO_UPDATE(brasil)
{
	return 0;
}


static ADDRESS_MAP_START( brasil_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x7ffff) AM_RAM // probably not
	AM_RANGE(0xe0000, 0xfffff) AM_ROMBANK(1)
ADDRESS_MAP_END

static INPUT_PORTS_START( brasil )
INPUT_PORTS_END

static MACHINE_DRIVER_START( brasil )
	MDRV_CPU_ADD("main", I80186, 25000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(brasil_map,0)
	//MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(brasil)
	MDRV_VIDEO_UPDATE(brasil)
MACHINE_DRIVER_END

ROM_START( brasil )
	ROM_REGION( 0x200000, "user1", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "record_brasil_hrc7_vers.3.u7", 0x000000, 0x100000, CRC(627e0d58) SHA1(6ff8ba7b21e1ea5c88de3f02a057906c9a7cd808) )
	ROM_LOAD16_BYTE( "record_brasil_hrc8_vers.3.u8", 0x000001, 0x100000, CRC(47f7ba2a) SHA1(0add7bbf771fd0bf205a05e910cb388cf052b09f) )

	ROM_REGION( 0x080000, "samples", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound_brasil_hbr_vers.1.u16", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

DRIVER_INIT( brasil )
{
	UINT8 *RAM = memory_region(machine, "user1");
	memory_set_bankptr(machine, 1, &RAM[0x1e0000]);
}

GAME( 2000, brasil,    0,        brasil,    brasil,    brasil, ROT0,  "unknown", "Bra$il", GAME_NOT_WORKING | GAME_NO_SOUND )
