/*

Royal Gum

Unknown CPU (either Z80 or Z180)

Big Black Box in the middle of the PCB (for encryption, or containing roms?)

*/

#include "emu.h"
#include "cpu/z80/z80.h"

static ADDRESS_MAP_START( rgum_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( rgum )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( rgum )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

VIDEO_START(royalgum)
{

}

VIDEO_UPDATE(royalgum)
{
	return 0;
}


static MACHINE_DRIVER_START( rgum )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(rgum_map)
//  MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_GFXDECODE(rgum)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(royalgum)
	MDRV_VIDEO_UPDATE(royalgum)
MACHINE_DRIVER_END




ROM_START( rgum )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rgum.u47", 0x00000, 0x20000, CRC(fe410eb9) SHA1(25180ba336269279f251be5483c210a581d27197) ) // encrypted.. 2nd half empty

	ROM_REGION( 0x10000, "data", 0 )
	ROM_LOAD( "rgum.u5", 0x00000, 0x10000, CRC(9d2d1681) SHA1(1c1da0d970ea2cf58f7961417ab6986cc667da5c) ) // plaintext in here, but firt half is empty

	ROM_REGION( 0x10000, "unk", 0 )
	ROM_LOAD( "rgum.u6", 0x00000, 0x2000, CRC(15a34117) SHA1(c7e0aef4007abfaaa533feb026148ba03230b79f) ) // near the data rom, mostly empty

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "rgum.u16", 0x00000, 0x8000, CRC(2a2c8d78) SHA1(2ce335b900dccbc34ad8ae7ae02ec7c75ffcd559) ) // first half empty
	ROM_CONTINUE(0x00000,0x8000)
	ROM_LOAD( "rgum.u17", 0x08000,  0x8000, CRC(fae4e41a) SHA1(421aac2b567040c3a56e01aa70880c94450eaf76) ) // first half empty
	ROM_CONTINUE(0x08000,0x8000)
	ROM_LOAD( "rgum.u18", 0x10000, 0x8000, CRC(79b17da7) SHA1(31e1845261b0152df56135c212e55c4048b7496f) ) // first half empty
	ROM_CONTINUE(0x10000,0x8000)
ROM_END


GAME( 199?, rgum, 0, rgum, rgum, 0, ROT0, "<unknown>",         "Royal Gum (Italy)", GAME_NOT_WORKING | GAME_NO_SOUND )
