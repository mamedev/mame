/*******************************************************************************************

Best League (c) 1993

A Big Striker Italian bootleg (made by Playmark?) running on a different hardware, and with
Italian Serie A teams instead of the national teams.

driver by David Haywood & Angelo Salese

Changes 29/03/2005 - Pierpaolo Prazzoli
- Fixed tilemaps and sprites offset
- Fixed visible area
- Fixed dip-switches
- Added oki banking
- Added sprites wraparound
- Added sprites color masking

*******************************************************************************************/

#include "driver.h"
#include "sound/okim6295.h"

/* Video Handling */

static UINT16 *bestleag_txram,*bestleag_bgram,*bestleag_fgram,*bestleag_vregs;
static tilemap *tx_tilemap,*bg_tilemap,*fg_tilemap;

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = bestleag_txram[tile_index];

	SET_TILE_INFO(
			0,
			(code & 0x0fff)|0x8000,
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = bestleag_bgram[tile_index];

	SET_TILE_INFO(
			1,
			(code & 0x0fff),
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = bestleag_fgram[tile_index];

	SET_TILE_INFO(
			1,
			(code & 0x0fff)|0x1000,
			((code & 0xf000) >> 12)|0x10,
			0);
}

static TILEMAP_MAPPER( bsb_bg_scan )
{
	int offset;

	offset = ((col&0xf)*16) + (row&0xf);
	offset += (col >> 4) * 0x100;
	offset += (row >> 4) * 0x800;

	return offset;
}

static VIDEO_START(bestleag)
{
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_cols,8,8,256, 32);
	bg_tilemap = tilemap_create(get_bg_tile_info,bsb_bg_scan,16,16,128, 64);
	fg_tilemap = tilemap_create(get_fg_tile_info,bsb_bg_scan,16,16,128, 64);

	tilemap_set_transparent_pen(tx_tilemap,15);
	tilemap_set_transparent_pen(fg_tilemap,15);
}

/*
Note: sprite chip is different than the other Big Striker sets and they
      include several similiarities with other Playmark games (including
      the sprite end code and the data being offset (i.e. spriteram starting from 0x16/2))
*/
static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{

	/*

    Sprites are the same to sslam, but using 16x16 sprites instead of 8x8

    */

	int offs;

	for (offs = 0x16/2;offs < spriteram_size/2;offs += 4)
	{
		int code = spriteram16[offs+3] & 0xfff;
		int color = (spriteram16[offs+2] & 0xf000) >> 12;
		int sx = (spriteram16[offs+2] & 0x1ff) - 20;
		int sy = (0xff - (spriteram16[offs+0] & 0xff)) - 15;
		int flipx = (spriteram16[offs+0] & 0x4000) >> 14;

		/* Sprite list end code */
		if(spriteram16[offs+0] & 0x2000)
			return;

		/* it can change sprites color mask like the original set */
		if(bestleag_vregs[0x00/2] & 0x1000)
			color &= 7;

		drawgfx(bitmap,machine->gfx[2],
					code,
					color,
					flipx, 0,
					flipx ? (sx+16) : (sx),sy,
					cliprect,TRANSPARENCY_PEN,15);

		drawgfx(bitmap,machine->gfx[2],
					code+1,
					color,
					flipx, 0,
					flipx ? (sx) : (sx+16),sy,
					cliprect,TRANSPARENCY_PEN,15);

		/* wraparound x */
		drawgfx(bitmap,machine->gfx[2],
					code,
					color,
					flipx, 0,
					flipx ? (sx+16 - 512) : (sx - 512),sy,
					cliprect,TRANSPARENCY_PEN,15);

		drawgfx(bitmap,machine->gfx[2],
					code+1,
					color,
					flipx, 0,
					flipx ? (sx - 512) : (sx+16 - 512),sy,
					cliprect,TRANSPARENCY_PEN,15);
	}
}

static VIDEO_UPDATE(bestleag)
{
	tilemap_set_scrollx(bg_tilemap,0,(bestleag_vregs[0x00/2] & 0xfff) + (bestleag_vregs[0x08/2] & 0x7) - 3);
	tilemap_set_scrolly(bg_tilemap,0,bestleag_vregs[0x02/2]);
	tilemap_set_scrollx(tx_tilemap,0,bestleag_vregs[0x04/2]);
	tilemap_set_scrolly(tx_tilemap,0,bestleag_vregs[0x06/2]);
	tilemap_set_scrollx(fg_tilemap,0,bestleag_vregs[0x08/2] & 0xfff8);
	tilemap_set_scrolly(fg_tilemap,0,bestleag_vregs[0x0a/2]);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

static WRITE16_HANDLER( bestleag_txram_w )
{
	bestleag_txram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

static WRITE16_HANDLER( bestleag_bgram_w )
{
	bestleag_bgram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static WRITE16_HANDLER( bestleag_fgram_w )
{
	bestleag_fgram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static WRITE16_HANDLER( oki_bank_w )
{
	OKIM6295_set_bank_base(0, 0x40000 * ((data & 3) - 1));
}


/* Memory Map */

static ADDRESS_MAP_START( bestleag_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x0d2000, 0x0d3fff) AM_NOP // left over from the original game (only read / written in memory test)
	AM_RANGE(0x0e0000, 0x0e3fff) AM_RAM_WRITE(bestleag_bgram_w) AM_BASE(&bestleag_bgram)
	AM_RANGE(0x0e8000, 0x0ebfff) AM_RAM_WRITE(bestleag_fgram_w) AM_BASE(&bestleag_fgram)
	AM_RANGE(0x0f0000, 0x0f3fff) AM_RAM_WRITE(bestleag_txram_w) AM_BASE(&bestleag_txram)
	AM_RANGE(0x0f8000, 0x0f800b) AM_RAM AM_BASE(&bestleag_vregs)
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x300010, 0x300011) AM_READ(input_port_0_word_r)
	AM_RANGE(0x300012, 0x300013) AM_READ(input_port_1_word_r)
	AM_RANGE(0x300014, 0x300015) AM_READ(input_port_2_word_r)
	AM_RANGE(0x300016, 0x300017) AM_READ(input_port_3_word_r)
	AM_RANGE(0x300018, 0x300019) AM_READ(input_port_4_word_r)
	AM_RANGE(0x30001c, 0x30001d) AM_WRITE(oki_bank_w)
	AM_RANGE(0x30001e, 0x30001f) AM_READWRITE(OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP
	AM_RANGE(0xfe0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

#define BESTLEAG_PLAYER_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(player) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

static INPUT_PORTS_START( bestleag )
	PORT_START	/* System inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START	/* Player 1 controls */
	BESTLEAG_PLAYER_INPUT( 1 )

	PORT_START	/* Player 2 controls */
	BESTLEAG_PLAYER_INPUT( 2 )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )	// also set "Coin B" to "Free Play"
	/* 0x01 to 0x05 gives 2C_3C */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )	// also set "Coin A" to "Free Play"
	/* 0x10 to 0x50 gives 2C_3C */

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) // Doesn't work ?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Timer Speed" )
	PORT_DIPSETTING(    0x08, "Slow" )				// 65
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )	// 50
	PORT_DIPSETTING(    0x10, "Fast" )				// 35
	PORT_DIPSETTING(    0x00, "Fastest" )			// 25
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "2 Players Game" )
	PORT_DIPSETTING(    0x40, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

/* GFX Decode */
static const gfx_layout bestleag_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 2*8, 4*8, 6*8,1*8,3*8,5*8,7*8},
	8*8
};

static const gfx_layout bestleag_char16layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0,1,2, 3, 4, 5, 6, 7,
		128+0,128+1,128+2,128+3,128+4,128+5,128+6,128+7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*16
};

static GFXDECODE_START( bestleag )
	GFXDECODE_ENTRY( REGION_GFX1, 0, bestleag_charlayout,     0x200, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, bestleag_char16layout,   0x000, 32 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, bestleag_char16layout,   0x300, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( bestleag )
	MDRV_CPU_ADD_TAG("main", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(bestleag_map,0)
	MDRV_CPU_VBLANK_INT("main", irq6_line_hold)


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(bestleag)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(bestleag)
	MDRV_VIDEO_UPDATE(bestleag)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("oki", OKIM6295, 1000000) /* Hand-tuned */
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.00)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.00)
MACHINE_DRIVER_END

/* Rom Loading */

ROM_START( bestleag )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(d2be3431) SHA1(37815c80b9fbc246fcdaa202d40fb40b10f55b45) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(f29c613a) SHA1(c66fa53f38bfa77ce1b894db74f94ce573c62412) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE ) /* 16x16x4 BG and 8x8x4 FG Tiles */
	ROM_LOAD( "4.bin", 0x000000, 0x80000, CRC(47f7c9bc) SHA1(f0e5ef971f3bd6972316c248175436055cb5789d) )
	ROM_LOAD( "5.bin", 0x080000, 0x80000, CRC(6a6f499d) SHA1(cacdccc64d09fa7289221cdea4654e6c2d811647) )
	ROM_LOAD( "6.bin", 0x100000, 0x80000, CRC(0c3d2609) SHA1(6e1f1c5b010ef0dfa3f7b4ff9a832e758fbb97d5) )
	ROM_LOAD( "7.bin", 0x180000, 0x80000, CRC(dcece871) SHA1(7db919ab7f51748b77b3bd35228bbf71b951349f) )

	ROM_REGION( 0x080000, REGION_GFX2, ROMREGION_DISPOSE ) /* 16x16x4 Sprites */
	ROM_LOAD( "8.bin",  0x000000, 0x20000, CRC(a463422a) SHA1(a3b6efd1c57b0a3b0ce4ce734a9a9b79540c4136) )
	ROM_LOAD( "9.bin",  0x020000, 0x20000, CRC(ebec74ed) SHA1(9a1620f4ca163470f5e567f650663ae368bdd3c1) )
	ROM_LOAD( "10.bin", 0x040000, 0x20000, CRC(7ea4e22d) SHA1(3c7f05dfd1c5889bfcbc14d08026e2a484870216) )
	ROM_LOAD( "11.bin", 0x060000, 0x20000, CRC(283d9ba6) SHA1(6054853f76907a4a0f89ad5aa02dde9d3d4ff196) )

	ROM_REGION( 0x80000, REGION_USER1, 0 ) /* Samples */
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(e152138e) SHA1(9d41b61b98414e1d5804b5a9edf4acb4c5f31615) )

	ROM_REGION( 0xc0000, REGION_SOUND1, 0 )
	ROM_COPY( REGION_USER1, 0x000000, 0x000000, 0x020000)
	ROM_COPY( REGION_USER1, 0x020000, 0x020000, 0x020000)
	ROM_COPY( REGION_USER1, 0x000000, 0x040000, 0x020000)
	ROM_COPY( REGION_USER1, 0x040000, 0x060000, 0x020000)
	ROM_COPY( REGION_USER1, 0x000000, 0x080000, 0x020000)
	ROM_COPY( REGION_USER1, 0x060000, 0x0a0000, 0x020000)
ROM_END

/* GAME drivers */

GAME( 1993, bestleag, bigstrik, bestleag, bestleag, 0, ROT0, "bootleg", "Best League", 0 )
