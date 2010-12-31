/*  Jaleco 'Stepping Stage'

*************************************************************************
 Naibo added:

 A PC computer(Harddisk not dumped yet) + Two 68000 based board set.

 One 68000 drives 3 screens, another handles players input.
*************************************************************************

 dump is incomplete, these are leftovers from an upgrade
 music roms are missing at least

 is it a 3 screen game (1 horizontal, 2 vertical) ?

 */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "rendlay.h"
#include "stepstag.lh"

static READ16_HANDLER( unknown_read_0xc00000 )
{
	return space->machine->rand();
}

static READ16_HANDLER( unknown_read_0xd00000 )
{
	return space->machine->rand();
}

static READ16_HANDLER( unknown_read_0xffff00 )
{
	return space->machine->rand();
}

static ADDRESS_MAP_START( stepstag_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x300000, 0x3fffff) AM_RAM

	AM_RANGE(0x400000, 0x43ffff) AM_RAM

	AM_RANGE(0x500000, 0x53ffff) AM_RAM

	AM_RANGE(0x700000, 0x700001) AM_WRITENOP //??
	AM_RANGE(0x700002, 0x700003) AM_WRITENOP //??
	AM_RANGE(0x700004, 0x700005) AM_WRITENOP //??
	AM_RANGE(0x700006, 0x700007) AM_WRITENOP //??

	AM_RANGE(0x800000, 0x87ffff) AM_RAM
	AM_RANGE(0x880000, 0x880001) AM_WRITENOP //??
	AM_RANGE(0x900000, 0x97ffff) AM_RAM
	AM_RANGE(0x980000, 0x980001) AM_WRITENOP //??
	AM_RANGE(0xa00000, 0xa7ffff) AM_RAM
	AM_RANGE(0xa80000, 0xa80001) AM_WRITENOP //??
	AM_RANGE(0xc00000, 0xc00001) AM_READ(unknown_read_0xc00000) AM_WRITENOP //??
	AM_RANGE(0xd00000, 0xd00001) AM_READ(unknown_read_0xd00000)
	AM_RANGE(0xf00000, 0xf00001) AM_WRITENOP //??
	AM_RANGE(0xffff00, 0xffff01) AM_READ(unknown_read_0xffff00)
ADDRESS_MAP_END

static READ16_HANDLER( unknown_sub_read_0xbe0004 )
{
	return space->machine->rand();
}


static ADDRESS_MAP_START( stepstag_sub_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ(unknown_sub_read_0xbe0004)
ADDRESS_MAP_END

static INPUT_PORTS_START( stepstag )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1, 2,3, 4,5,6,7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( stepstag )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx5", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx6", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx7", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx8", 0, tiles8x8_layout, 0, 2 )

GFXDECODE_END

static VIDEO_START(stepstag)
{
}
static VIDEO_UPDATE(stepstag)
{

	return 0;
}

static MACHINE_CONFIG_START( stepstag, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000 ) //??
	MCFG_CPU_PROGRAM_MAP(stepstag_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold) // 4 & 6 valid

	MCFG_CPU_ADD("sub", M68000, 16000000 ) //??
	MCFG_CPU_PROGRAM_MAP(stepstag_sub_map)

	/* video hardware */
	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(0x200)
	MCFG_GFXDECODE(stepstag)
	MCFG_DEFAULT_LAYOUT(layout_stepstag)

	MCFG_VIDEO_START(stepstag)
	MCFG_VIDEO_UPDATE(stepstag)
MACHINE_CONFIG_END


ROM_START( stepstag )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98348ver11.11", 0x00000, 0x80000, CRC(29b7f848) SHA1(c4d89e5c9be622b2d9038c359a5f65ce0dd461b0) )
	ROM_LOAD16_BYTE( "vj98348ver11.14", 0x00001, 0x80000, CRC(e3314c6c) SHA1(61b0e9f9d0126d9f475304866a03cfa21701d9aa) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98344ver11.1", 0x00001, 0x80000, CRC(aedcb225) SHA1(f167c390e79ffbf7c019c326384ae656ae8b7d13) )
	ROM_LOAD16_BYTE( "vj98344ver11.4", 0x00000, 0x80000, CRC(391ca913) SHA1(2cc329aa6419f8a0d7e0fb8a9f4c2b8ca25197b3) )


	ROM_REGION( 0x400000, "gfx1", 0 ) /* */
	ROM_LOAD( "mr99001-06", 0x00000, 0x400000, CRC(cfa27c93) SHA1(a0837877736e8e898f3acc64bc87ee0cc4d9f243) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* */
	ROM_LOAD( "mr99001-05", 0x00000, 0x400000, CRC(3958473b) SHA1(12279a587263290945744b22aafb80460eea77f7) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* */
	ROM_LOAD( "mr99001-04", 0x00000, 0x400000, CRC(d6837981) SHA1(56709d73304f0b186c70844ae96f73400b541609) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* */
	ROM_LOAD( "mr99001-03", 0x00000, 0x400000, CRC(40fee0df) SHA1(94c3567e82f8039b3169bf4dcb1fcd9e39c6eb27) )

	ROM_REGION( 0x400000, "gfx5", 0 ) /* */
	ROM_LOAD( "mr99001-02", 0x00000, 0x400000, CRC(12c65d86) SHA1(7fe5853fa3ba086f8da15702b126eb13c6ea30a9) )

	ROM_REGION( 0x400000, "gfx6", 0 ) /* */
	ROM_LOAD( "mr99001-01", 0x00000, 0x400000,  CRC(aa92cebf) SHA1(2ccc0d2ef9bc92c27f0a625819154bbcf9cfde0c) )

	ROM_REGION( 0x400000, "gfx7", 0 ) /* */
	ROM_LOAD( "s.s.s._vj-98348_19_pr99021-02", 0x00000, 0x400000, CRC(2d98da1a) SHA1(b09375fa1b4b2e0794632d6e237459009f40310d) )

	ROM_REGION( 0x400000, "gfx8", 0 ) /* */
	ROM_LOAD( "s.s.s._vj-98348_26_pr99021-01", 0x00000, 0x400000, CRC(fefb3777) SHA1(df624e105ab1dea52317e318ad29caa02b900788) )
	ROM_LOAD( "s.s.s._vj-98348_3_pr99021-01", 0x00000, 0x400000, CRC(e0fbc6f1) SHA1(7ca4507702f3f81bb9de3f9b5d270d379e439633) )

	DISK_REGION( "disks" )
	DISK_IMAGE("stepstag", 0, NO_DUMP)
ROM_END

ROM_START( step3 )
ROM_REGION( 0x100000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98344ver11.1", 0x00001, 0x80000, NO_DUMP )
	ROM_LOAD16_BYTE( "vj98344ver11.4", 0x00000, 0x80000, NO_DUMP )
	// c'est la programme de stepstag (avoir besoin de modifications, numero de chansons par example)

	ROM_REGION( 0x100000, "sub", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "vj98348_step3_11_v1.1", 0x00000, 0x80000, CRC(9c36aef5) SHA1(bbac48c2c7949a6f8a6ec83515e94a343c88d1b6) )
	ROM_LOAD16_BYTE( "vj98348_step3_14_v1.1", 0x00001, 0x80000, CRC(b86be557) SHA1(49dbd6ef1c50adcf3386d5423da8ae7685649c46) )

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* */
	ROM_LOAD( "mr9930-01.ic2", 0x000000, 0x400000, CRC(9e3e054e) SHA1(06a4fa76cb83dbe9d565d5ccd0a5ecc5067887c9) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* */
	ROM_LOAD( "mr9930-02.ic3", 0x000000, 0x400000, CRC(b23c29f4) SHA1(a7b10a3a9af43db319baf8633bb3728120960923) )

	ROM_REGION( 0x1000000, "gfx3", 0 ) /* */
	ROM_LOAD( "mr9930-03.ic4", 0x000000, 0x400000, CRC(9a5d070f) SHA1(b4668b4f299033140a2c56499cc2712ba111cb57) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* screen centre */
	ROM_LOAD( "mr99030-04.ic17", 0x000000, 0x400000, CRC(3eac3591) SHA1(3b294e94af23fd92fdf51d2c9c43f60d2ebd1688) )

	ROM_REGION( 0x1000000, "gfx5", 0 ) /* */
	ROM_LOAD( "mr99030-05.ic18", 0x000000, 0x400000, CRC(dea7b8d6) SHA1(d7d98675eb3998a8057929f90aa340c1e5f6a617) )

	ROM_REGION( 0x1000000, "gfx6", 0 ) /* */
	ROM_LOAD( "mr99030-06.ic19", 0x000000, 0x400000, CRC(71489d79) SHA1(0398a354c2588e3974cb76a331e46165db6af06d) )

	ROM_REGION( 0x1000000, "gfx7", 0 ) /* */
	ROM_LOAD( "mr9930-01.ic30", 0x000000, 0x400000, CRC(9e3e054e) SHA1(06a4fa76cb83dbe9d565d5ccd0a5ecc5067887c9) )
	ROM_LOAD( "mr9930-02.ic29", 0x400000, 0x400000, CRC(b23c29f4) SHA1(a7b10a3a9af43db319baf8633bb3728120960923) )
	ROM_LOAD( "mr9930-03.ic28", 0x800000, 0x400000, CRC(9a5d070f) SHA1(b4668b4f299033140a2c56499cc2712ba111cb57) )

	ROM_REGION( 0x1000000, "gfx8", 0 ) /* */
	ROM_LOAD( "vj98348_step3_4_v1.1", 0x000000, 0x400000, CRC(dec612df) SHA1(acb86bb90c1cc61c7db3e022c69a5ff0611ffbae) )
	ROM_LOAD( "vj98348_step3_18_v1.1", 0x400000, 0x400000, CRC(bc92f0a0) SHA1(49c08de7a898a27972d4209709ddf447c5dca36a) )
	ROM_LOAD( "vj98348_step3_25_v1.1", 0x800000, 0x400000, CRC(dec612df) SHA1(acb86bb90c1cc61c7db3e022c69a5ff0611ffbae) )

	DISK_REGION( "disks" )
	DISK_IMAGE("step3", 0, NO_DUMP)
ROM_END

//GAME( 1999, stepstag, 0, stepstag, stepstag, 0, ROT0, "Jaleco", "Stepping Stage", GAME_NO_SOUND| GAME_NOT_WORKING)    // Original Game
GAME( 1999, stepstag, 0, stepstag, stepstag, 0, ROT0, "Jaleco", "Stepping Stage Special", GAME_NO_SOUND| GAME_NOT_WORKING)
//GAME( 1999, stepstag, 0, stepstag, stepstag, 0, ROT0, "Jaleco", "Stepping Stage 2 Supreme", GAME_NO_SOUND| GAME_NOT_WORKING)
GAME( 1999, step3, 0,	stepstag, stepstag, 0,	ROT0, "Jaleco", "Stepping 3 Superior", GAME_NO_SOUND| GAME_NOT_WORKING)
