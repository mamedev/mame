/***************************************************************************

Pinkiri 8 skeleton driver

- Missing internal ROM, can't do much at the current time

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"

static VIDEO_START( pinkiri8 )
{

}

static VIDEO_UPDATE( pinkiri8 )
{
	return 0;
}

static ADDRESS_MAP_START( pinkiri8_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( pinkiri8 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5),RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( pinkiri8 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
GFXDECODE_END

static MACHINE_DRIVER_START( pinkiri8 )
	MDRV_CPU_ADD("maincpu",Z180,16000000)
	MDRV_CPU_PROGRAM_MAP(pinkiri8_map)
//  MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,4)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(pinkiri8)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(pinkiri8)
	MDRV_VIDEO_UPDATE(pinkiri8)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
//  MDRV_SOUND_ADD("aysnd", AY8910, 8000000/4 /* guess */)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pinkiri8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pinkiri8-ver.1.02.l1",   0x0000, 0x20000, CRC(f2df5b12) SHA1(e374e184a6a1e932550516011ec09a5accec9b03) )

	ROM_REGION( 0x20000*5, "gfx1", 0 )
	ROM_LOAD( "pinkiri8-chr-01.a1",  0x00000, 0x20000, CRC(8ec73662) SHA1(9098348e519ce753dd7f38f0d855181bfc65aa42) )
	ROM_LOAD( "pinkiri8-chr-02.bc1", 0x20000, 0x20000, CRC(8dc20a65) SHA1(4062510fe06e8844a732754b7915a3b67ba2a3c5) )
	ROM_LOAD( "pinkiri8-chr-03.d1",  0x40000, 0x20000, CRC(bd5f269a) SHA1(7dfd039227551f0f0ed4afaafc76ca64a39a9b83) )
	ROM_LOAD( "pinkiri8-chr-04.ef1", 0x60000, 0x20000, CRC(4d0e5005) SHA1(4b90119c359c4de576131fd0e28d2fe1482ce74f) )
	ROM_LOAD( "pinkiri8-chr-05.h1",  0x80000, 0x20000, CRC(036ca165) SHA1(c4a2d6e394bbabcae1413d8a2916a19c90687edf) )
ROM_END

GAME( 2005?, pinkiri8,  0,   pinkiri8,  pinkiri8,  0, ROT0, "<unknown>", "Pinkiri 8", GAME_NOT_WORKING| GAME_NO_SOUND )
