/* Has (c)1983 GTI in the roms, and was called 'Poker.zip'  GFX roms contain 16x16 tiles of cards */
/* Nothing else is known about this set / game */

#include "driver.h"

static VIDEO_START(gtipoker)
{

}

static VIDEO_UPDATE(gtipoker)
{
	return 0;
}

static READ8_HANDLER( gtipoker_unk_r )
{
	return mame_rand(machine);
}

static ADDRESS_MAP_START( gtipoker_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xd000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gtipoker_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xbc, 0xbc) AM_READ(gtipoker_unk_r)
	AM_RANGE(0xbe, 0xbe) AM_READ(gtipoker_unk_r)
	AM_RANGE(0xde, 0xde) AM_READ(gtipoker_unk_r)
	AM_RANGE(0xef, 0xef) AM_READ(gtipoker_unk_r)
	AM_RANGE(0xfd, 0xfd) AM_READ(gtipoker_unk_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( gtipoker )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*16
};

static GFXDECODE_START( gtipoker )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


static MACHINE_DRIVER_START( gtipoker )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(gtipoker_memmap,0)
	MDRV_CPU_IO_MAP(gtipoker_iomap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(gtipoker)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(gtipoker)
	MDRV_VIDEO_UPDATE(gtipoker)
MACHINE_DRIVER_END


ROM_START( gtipoker )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "u12.rom", 0x0000, 0x1000, CRC(abaa257a) SHA1(f830213ae0aaad5a9a44ec77c5a186e9e02fa041) )
	ROM_LOAD( "u18.rom", 0x1000, 0x1000, CRC(1b7e2877) SHA1(717fb70889804baa468203f20b1e7f73b55cc21e) )

	ROM_REGION( 0x1000, REGION_GFX1, 0 )
	ROM_LOAD( "u31.rom", 0x0000, 0x1000, CRC(2028db2c) SHA1(0f81bb71e88c60df3817f58c28715ce2ea01ad4d) )
ROM_END

GAME( 1983, gtipoker,  0,    gtipoker, gtipoker,  0, ROT0, "GTI Inc", "GTI Poker", GAME_NOT_WORKING|GAME_NO_SOUND )
