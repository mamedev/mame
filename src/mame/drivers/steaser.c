/* Strip Teaser
 this has *2* 68705 MCUs..  neither dumped
*/


/*

Strip Teaser (unknown manufacturer)
------------------------------------

lower board (Dk-B)

TS68000CP12 (main cpu)
osc. 11.0592MHz
MC68HC705C8P (MCU)
UM6845RA (CRT controller-Supports alphanumeric and graphics modes.Addresses up to 16 KB of video memory-2 MHz)
Lithium battery 3,6V


upper board (8L74) (soundboard?)

MC68HC705C8P (MCU)
osc. 4.0000MHz
non JAMMA connector
1x dipswitch (4 switch)


ROMs
1x AT27c010 (u31.1)(program)
1x AM27C010 (u32.6)(program)
4x M27C4001 (u46.2 - u51.3 - u61.4 - u66.5)(GFX)
1x M27C4001 (u18.7)(sound)

*/


#include "driver.h"
#include "deprecat.h"

static VIDEO_START(steaser)
{
}

static VIDEO_UPDATE(steaser)
{
	return 0;
}

static READ16_HANDLER( steaser_bd0000_r )
{
	return mame_rand(Machine);
}

static ADDRESS_MAP_START( steaser_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xbd0000, 0xbd0001) AM_READ( steaser_bd0000_r )
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( steaser )
INPUT_PORTS_END


/* The graphics don't seem to be tile based .. */
static const gfx_layout steaser_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static GFXDECODE_START( steaser )
	GFXDECODE_ENTRY( REGION_GFX1, 0, steaser_layout,   0x0, 2  )
GFXDECODE_END



static MACHINE_DRIVER_START( steaser )
	MDRV_CPU_ADD_TAG("main", M68000, 11059200 )
	MDRV_CPU_PROGRAM_MAP(steaser_map,0)
//  MDRV_CPU_VBLANK_INT(irq1_line_hold,1)

	MDRV_GFXDECODE(steaser)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(steaser)
	MDRV_VIDEO_UPDATE(steaser)
MACHINE_DRIVER_END


ROM_START( steaser )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "u31.1", 0x00001, 0x20000, CRC(7963e960) SHA1(2a1c68265e0a3909ccd097ea784e3e179f528844) )
	ROM_LOAD16_BYTE( "u32.6", 0x00000, 0x20000, CRC(c0ab5fb1) SHA1(15b3dbf0242e885b7009c21479544a821d4e5a7d) )

	ROM_REGION( 0x1000, REGION_CPU2, 0 ) /* 68705 */
	ROM_LOAD( "mc68hc705c8p_main.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, REGION_CPU3, 0 ) /* 68705 */
	ROM_LOAD( "mc68hc705c8p_sub.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Sound Samples */
	ROM_LOAD( "u18.7", 0x00000, 0x80000, CRC(ee942232) SHA1(b9c1fc73c6006bcad0dd177e0f30a96f1063a993) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 ) /* GFX (not tiles..) */
	ROM_LOAD( "u46.2", 0x000000, 0x80000, CRC(c4a5e47b) SHA1(9f3d3124c76c0bdf8cdca849e1d921a335e433b6) )
	ROM_LOAD( "u51.3", 0x080000, 0x80000, CRC(4dc57435) SHA1(7dfa6f9e35986dd48869786abbe70103f336bcb1) )
	ROM_LOAD( "u61.4", 0x100000, 0x80000, CRC(d8d8dc6f) SHA1(5a76b1fd1a3a532e5ff2de127286ace7d3567c58) )
	ROM_LOAD( "u66.5", 0x180000, 0x80000, CRC(da309671) SHA1(66baf8a83024547c471da39748ff99a9a9013ea4) )
ROM_END

GAME( 199?, steaser, 0, steaser, steaser, 0, ROT0,  "Unknown", "Strip Teaser (Italy)", GAME_NOT_WORKING|GAME_NO_SOUND )
