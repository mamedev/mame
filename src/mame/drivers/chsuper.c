#include "driver.h"
#include "cpu/z180/z180.h"


static VIDEO_START(chsuper)
{
}

static VIDEO_UPDATE(chsuper)
{
	return 0;
}


static ADDRESS_MAP_START( chsuper_prg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_REGION("maincpu", 0xf000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( chsuper_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs
ADDRESS_MAP_END



static INPUT_PORTS_START( chsuper )

	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/* WRONG! */

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7},
	{ 2*8,3*8,0*8,1*8, 256+2*8, 256+3*8, 256+0*8, 256+1*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32},
	8*64
};

static GFXDECODE_START( chsuper )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( chsuper )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)	/* HD64180RP8, 8 MHz? */
	MDRV_CPU_PROGRAM_MAP(chsuper_prg_map,0)
	MDRV_CPU_IO_MAP(chsuper_portmap,0)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 32*8-1)

	MDRV_GFXDECODE(chsuper)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(chsuper)
	MDRV_VIDEO_UPDATE(chsuper)

	/* sound hardware */
//	MDRV_SPEAKER_STANDARD_MONO("mono")

MACHINE_DRIVER_END


/*  ROM Regions definition
 */

ROM_START( chsuper3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x80000, CRC(e987ed1f) SHA1(8d1ee01914356714c7d1f8437d98b41a707a174a) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "a.bin",  0x00000, 0x80000, CRC(ace8b591) SHA1(e9ba5efebdc9b655056ed8b2621f062f50e0528f) )
	ROM_LOAD( "b.bin",  0x80000, 0x80000, CRC(5f58c722) SHA1(d339ae27af010b058eae9084fba85fb2fbed3952) )
ROM_END

ROM_START( chsuper2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x80000, CRC(cbf59e69) SHA1(68e4b167fdf9103fd748cff401f4fe7c1d214552) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "a.bin",  0x00000, 0x80000, CRC(7caa8ebe) SHA1(440306a208ec8afd570b15f05b5dc542acc98510) )
	ROM_LOAD( "b.bin",  0x80000, 0x80000, CRC(7bb463d7) SHA1(fb3842ba53e545fa47574c91df7281a9cb417395) )
ROM_END



GAME( 1999, chsuper3, 0,        chsuper, chsuper,  0, ROT0, "unknown",    "Champion Super 3", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1999, chsuper2, chsuper3, chsuper, chsuper,  0, ROT0, "unknown",    "Champion Super 2", GAME_NOT_WORKING|GAME_NO_SOUND )
