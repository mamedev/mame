/*

Poker game

Copyright (C) 1992 HI-TECH Software..Brisbane, QLD Australia
 is present in the ROMs, although the game is in Italian
 and has a 1997 copyright, its probably a bootleg

*/

#include "driver.h"
#include "deprecat.h"
#include "sound/okim6295.h"


static tilemap *jackpool_layer0_tilemap;
static UINT16 *jackpool_layer0_videoram;
static tilemap *jackpool_layer1_tilemap;
static UINT16 *jackpool_layer1_videoram;
static tilemap *jackpool_layer2_tilemap;
static UINT16 *jackpool_layer2_videoram;


static TILE_GET_INFO( get_jackpool_layer0_tile_info )
{
	int tileno,attr;

	tileno = jackpool_layer0_videoram[tile_index]&0x7fff;
	attr = jackpool_layer0_videoram[tile_index+0x800]&0x1f;

	SET_TILE_INFO(0,tileno,attr,0);
}

static WRITE16_HANDLER( jackpool_layer0_videoram_w )
{
	COMBINE_DATA(&jackpool_layer0_videoram[offset]);

	tilemap_mark_tile_dirty(jackpool_layer0_tilemap,offset&0x07ff);
}

static TILE_GET_INFO( get_jackpool_layer1_tile_info )
{
	int tileno,attr;

	tileno = jackpool_layer1_videoram[tile_index]&0x7fff;
	attr = jackpool_layer1_videoram[tile_index+0x800]&0x1f;

	SET_TILE_INFO(0,tileno,attr,0);
}

static WRITE16_HANDLER( jackpool_layer1_videoram_w )
{
	COMBINE_DATA(&jackpool_layer1_videoram[offset]);

	tilemap_mark_tile_dirty(jackpool_layer1_tilemap,offset&0x07ff);
}

static TILE_GET_INFO( get_jackpool_layer2_tile_info )
{
	int tileno,attr;

	tileno = jackpool_layer2_videoram[tile_index]&0x7fff;
	attr = jackpool_layer2_videoram[tile_index+0x800]&0x1f;

	SET_TILE_INFO(0,tileno,attr,0);
}

static WRITE16_HANDLER( jackpool_layer2_videoram_w )
{
	COMBINE_DATA(&jackpool_layer2_videoram[offset]);

	tilemap_mark_tile_dirty(jackpool_layer2_tilemap,offset&0x07ff);
}



static READ16_HANDLER( jackpool_io_r )
{
	return 0xffff;
}

static ADDRESS_MAP_START( jackpool_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_READ(MRA16_RAM)

	AM_RANGE(0x120000, 0x1200ff) AM_READ(MRA16_RAM) // maybe nvram?

	AM_RANGE(0x340000, 0x34ffff) AM_READ(MRA16_RAM)
	AM_RANGE(0x360000, 0x3603ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x380000, 0x38005f) AM_READ(MRA16_RAM)

	AM_RANGE(0x800000, 0x80000f) AM_READ(jackpool_io_r)


ADDRESS_MAP_END

static ADDRESS_MAP_START( jackpool_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x120000, 0x1200ff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x340000, 0x341fff) AM_WRITE(jackpool_layer0_videoram_w) AM_BASE(&jackpool_layer0_videoram)
	AM_RANGE(0x342000, 0x343fff) AM_WRITE(jackpool_layer1_videoram_w) AM_BASE(&jackpool_layer1_videoram)
	AM_RANGE(0x344000, 0x345fff) AM_WRITE(jackpool_layer2_videoram_w) AM_BASE(&jackpool_layer2_videoram) // is this the same screen?
	/* are the ones below used? */
	AM_RANGE(0x346000, 0x347fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x348000, 0x349fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x34a000, 0x34bfff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x34c000, 0x34dfff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x34e000, 0x34ffff) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x360000, 0x3603ff) AM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x380000, 0x38005f) AM_WRITE(MWA16_RAM)
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( roulette_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
ADDRESS_MAP_END
#endif



static INPUT_PORTS_START( jackpool )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};




static VIDEO_START(jackpool)
{
	jackpool_layer0_tilemap = tilemap_create(get_jackpool_layer0_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	jackpool_layer1_tilemap = tilemap_create(get_jackpool_layer1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	jackpool_layer2_tilemap = tilemap_create(get_jackpool_layer2_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);

	tilemap_set_transparent_pen(jackpool_layer0_tilemap,0);
	tilemap_set_transparent_pen(jackpool_layer2_tilemap,0);
}

static VIDEO_UPDATE(jackpool)
{
	tilemap_draw(bitmap,cliprect,jackpool_layer1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,jackpool_layer0_tilemap,0,0);

//  tilemap_draw(bitmap,cliprect,jackpool_layer2_tilemap,0,0); // is this actually a second display?

	return 0;
}


static GFXDECODE_START( jackpool )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout,   0x000, 256  ) /* sprites */
GFXDECODE_END

/* verify */
static INTERRUPT_GEN( jackpool_interrupt )
{
	cpunum_set_input_line(machine, 0, cpu_getiloops()+1, HOLD_LINE);	/* IRQs 3, 2, and 1 */
}


static MACHINE_DRIVER_START( jackpool )
	MDRV_CPU_ADD(M68000, 12000000) // ?
	MDRV_CPU_PROGRAM_MAP(jackpool_readmem,jackpool_writemem)
	MDRV_CPU_VBLANK_INT(jackpool_interrupt,3)  // ?

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_GFXDECODE(jackpool)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(jackpool)
	MDRV_VIDEO_UPDATE(jackpool)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, 1056000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END


ROM_START( jackpool )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jpc2", 0x00001, 0x20000,CRC(5aad51ff) SHA1(af504d15c356c241efb6410a5dad09494d693eca) )
	ROM_LOAD16_BYTE( "jpc3", 0x00000, 0x20000,CRC(249c7073) SHA1(e654232d5f454932a108591deacadc9da9fd8055) )

	ROM_REGION( 0x080000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "jpc1", 0x00000, 0x40000, CRC(0f1372a1) SHA1(cec8a9bfb03945af4e1e2d2b916b9ded52a8d0bd) )

	ROM_REGION( 0x100000, REGION_GFX1, 0 ) /* Sprites */
	ROM_LOAD( "jpc4", 0x00000, 0x40000,  CRC(b719f138) SHA1(82799cbccab4e39627e48855f6003917602b42c7) )
	ROM_LOAD( "jpc5", 0x40000, 0x40000,  CRC(09661ed9) SHA1(fb298252c95a9040441c12c9d0e9280843d56a0d) )
	ROM_LOAD( "jpc6", 0x80000, 0x40000,  CRC(c3117411) SHA1(8ed044beb1d6ab7ac48595f7d6bf879f1264454a) )
	ROM_LOAD( "jpc7", 0xc0000, 0x40000,  CRC(b1d40623) SHA1(fb76ae6b53474bd4bee19dbce9537da0f2b63ff4) )
ROM_END

ROM_START( suprpool )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "3-1.22a.u2", 0x00001, 0x20000,CRC(c762cd1c) SHA1(ee05a9e8147d613eb14333e6e7b743fc05982e7c) )
	ROM_LOAD16_BYTE( "2-1.22a.u3", 0x00000, 0x20000,CRC(5d15037a) SHA1(74cab79a1b08910267262a4c6b501126a4df6cda) )

	ROM_REGION( 0x080000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "1.u32", 0x00000, 0x40000, CRC(47804af7) SHA1(602dc0361869b52532e2adcb0de3cbdd042761b3) )

	ROM_REGION( 0x80000, REGION_GFX1, 0 ) /* Sprites */
	ROM_LOAD( "4.u38", 0x00000, 0x20000,  CRC(0660a169) SHA1(1cb34b3da4b144028519a3c5b32ef7da44af0624) )
	ROM_LOAD( "5.u37", 0x20000, 0x20000,  CRC(6e99af07) SHA1(85e7a76724fd9ce8d07b5088cb6e0d933fd95692) )
	ROM_LOAD( "6.u36", 0x40000, 0x20000,  CRC(c4448813) SHA1(6e168eb8503b852179f2d743f1cba935592e0a60) )
	ROM_LOAD( "7.u35", 0x60000, 0x20000,  CRC(357d145f) SHA1(9fea0d0c5d6c27bf520c4f81eb0f48a65ff60142) )
ROM_END

GAME( 1997, jackpool, 0, jackpool, jackpool, 0, ROT0, "Electronic Projects", "Jackpot Pool (Italy, bootleg)",GAME_NOT_WORKING )
GAME( 1997, suprpool, 0, jackpool, jackpool, 0, ROT0, "Nazionale Elettronica Giochi S.A.S.", "Super Pool (Italy)",GAME_NOT_WORKING )
