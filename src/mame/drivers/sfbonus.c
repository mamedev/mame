/* Skill Fruit Bonus
 -- (unknown) encrypted CPU
 --  16-bit (program appears interleaved)
*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


static ADDRESS_MAP_START( sfbonus_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
ADDRESS_MAP_END

VIDEO_START(sfbonus)
{

}

VIDEO_UPDATE(sfbonus)
{
	return 0;
}

static INPUT_PORTS_START( sfbonus )
INPUT_PORTS_END

static const gfx_layout sfbonus_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64 },
	8*64
};


static GFXDECODE_START( sfbonus )
	GFXDECODE_ENTRY( "gfx1", 0, sfbonus_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx2", 0, sfbonus_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx3", 0, sfbonus_layout,   0x0, 2  )
GFXDECODE_END


static MACHINE_DRIVER_START( sfbonus )
	MDRV_CPU_ADD("main", M68000, 16000000) // unknown CPU
	MDRV_CPU_PROGRAM_MAP(0,sfbonus_map)

	MDRV_GFXDECODE(sfbonus)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(sfbonus)
	MDRV_VIDEO_UPDATE(sfbonus)

//  MDRV_SPEAKER_STANDARD_STEREO("left", "right")
//  MDRV_SOUND_ADD("oki", OKIM6295, 1000000)
//  MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.47)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.47)
MACHINE_DRIVER_END

ROM_START( sfbonus )
	ROM_REGION( 0x80000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "skfb16.bin", 0x00001, 0x40000, CRC(bfd53646) SHA1(bd58f8c6d5386649a6fc0f4bac46d1b6cd6248b1) )
	ROM_LOAD16_BYTE( "skfb17.bin", 0x00000, 0x40000, CRC(e28ede82) SHA1(f320c4c9c30ec280ee2437d1ad4d2b6270580916) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "skfbrom2.bin", 0x00000, 0x20000, CRC(3823a36e) SHA1(4136e380b63546b9490033ad26d776f326eb9290) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "skfbrom3.bin", 0x00000, 0x80000, CRC(36119517) SHA1(241bb256ab3ba595dcb0c81fd2e60ed35dd7c197) )
	ROM_LOAD16_BYTE( "skfbrom4.bin", 0x00001, 0x80000, CRC(a655bac1) SHA1(0faea01c09409f9182f08370dcc0b466a799f17f) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "skfbrom5.bin", 0x00000, 0x80000, CRC(752e6e3b) SHA1(46c3a1bbbf1a2afe36fa5333b6e74459e17e9bae) )
	ROM_LOAD16_BYTE( "skfbrom6.bin", 0x00001, 0x80000, CRC(30df6b6a) SHA1(7a180fa8ee64b9efb0321baffad72f0a9485d568) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	// these are the same as gfx 2 up to a point, after which they change
	// maybe different tiles for odd / even tilemap columns, like cps1.c
	ROM_LOAD16_BYTE( "skfbrom5a.bin", 0x00000, 0x80000, CRC(ed07a635) SHA1(a31ee06d8cb78c43affa9d899e9cec87a0875934) )
	ROM_LOAD16_BYTE( "skfbrom6a.bin", 0x00001, 0x80000, CRC(b9a39e5a) SHA1(cb875db25d8bccf48542bd6dbecc474b1205563b) )
ROM_END

GAME( 199?, sfbonus,    0,        sfbonus,    sfbonus,    0, ROT0,  "Amcoe", "Skill Fruit Bonus", GAME_NOT_WORKING|GAME_NO_SOUND )
