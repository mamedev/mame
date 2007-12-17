/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -

******************************************************************************/

#include "driver.h"

static int fromanc2_scrollx[2][4], fromanc2_scrolly[2][4];
static int fromanc2_gfxbank[2][4];
static UINT16 *fromanc2_paletteram[2];
static UINT16 *fromanc2_videoram[2][4];
static tilemap *fromanc2_tilemap[2][4];

/******************************************************************************

  Callbacks for the TileMap code

******************************************************************************/

INLINE void fromanc2_get_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index, int vram, int layer)
{
	int tile, color;

	tile  = (fromanc2_videoram[vram][layer][tile_index] & 0x3fff) | (fromanc2_gfxbank[vram][layer] << 14);
	color = ((fromanc2_videoram[vram][layer][tile_index] & 0xc000) >> 14) | (0x10 * vram);

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( fromanc2_get_v0_l0_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 0); }
static TILE_GET_INFO( fromanc2_get_v0_l1_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 1); }
static TILE_GET_INFO( fromanc2_get_v0_l2_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 2); }
static TILE_GET_INFO( fromanc2_get_v0_l3_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 3); }
static TILE_GET_INFO( fromanc2_get_v1_l0_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 0); }
static TILE_GET_INFO( fromanc2_get_v1_l1_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 1); }
static TILE_GET_INFO( fromanc2_get_v1_l2_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 2); }
static TILE_GET_INFO( fromanc2_get_v1_l3_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 3); }


INLINE void fromancr_get_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index, int vram, int layer)
{
	int tile, color;

	tile  = fromanc2_videoram[vram][layer][tile_index] | (fromanc2_gfxbank[vram][layer] << 16);
	color = vram;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( fromancr_get_v0_l0_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 0, 0); }
static TILE_GET_INFO( fromancr_get_v0_l1_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 0, 1); }
static TILE_GET_INFO( fromancr_get_v0_l2_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 0, 2); }
static TILE_GET_INFO( fromancr_get_v1_l0_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 1, 0); }
static TILE_GET_INFO( fromancr_get_v1_l1_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 1, 1); }
static TILE_GET_INFO( fromancr_get_v1_l2_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 1, 2); }


/******************************************************************************

  Memory handlers

******************************************************************************/

READ16_HANDLER( fromanc2_paletteram_0_r )
{
	return fromanc2_paletteram[0][offset];
}

READ16_HANDLER( fromanc2_paletteram_1_r )
{
	return fromanc2_paletteram[1][offset];
}

WRITE16_HANDLER( fromanc2_paletteram_0_w )
{
	UINT16 color;

	COMBINE_DATA(&fromanc2_paletteram[0][offset]);

	// GGGG_GRRR_RRBB_BBBx
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(Machine, 0x000 + color, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
}

WRITE16_HANDLER( fromanc2_paletteram_1_w )
{
	UINT16 color;

	COMBINE_DATA(&fromanc2_paletteram[1][offset]);

	// GGGG_GRRR_RRBB_BBBx
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(Machine, 0x100 + color, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
}


READ16_HANDLER( fromancr_paletteram_0_r )
{
	return fromanc2_paletteram[0][offset];
}

READ16_HANDLER( fromancr_paletteram_1_r )
{
	return fromanc2_paletteram[1][offset];
}

WRITE16_HANDLER( fromancr_paletteram_0_w )
{
	UINT16 color;

	COMBINE_DATA(&fromanc2_paletteram[0][offset]);

	// xGGG_GGRR_RRRB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(Machine, 0x000 + color, pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));
}

WRITE16_HANDLER( fromancr_paletteram_1_w )
{
	UINT16 color;

	COMBINE_DATA(&fromanc2_paletteram[1][offset]);

	// xGGG_GGRR_RRRB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(Machine, 0x100 + color, pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));
}


READ16_HANDLER( fromanc4_paletteram_0_r )
{
	return fromanc2_paletteram[0][offset];
}

READ16_HANDLER( fromanc4_paletteram_1_r )
{
	return fromanc2_paletteram[1][offset];
}

WRITE16_HANDLER( fromanc4_paletteram_0_w )
{
	UINT16 color;

	COMBINE_DATA(&fromanc2_paletteram[0][offset]);

	// xRRR_RRGG_GGGB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(Machine, 0x000 + color, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

WRITE16_HANDLER( fromanc4_paletteram_1_w )
{
	UINT16 color;

	COMBINE_DATA(&fromanc2_paletteram[1][offset]);

	// xRRR_RRGG_GGGB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(Machine, 0x100 + color, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}


INLINE void fromanc2_dispvram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int vram, int layer)
{
	layer += (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&fromanc2_videoram[vram][layer][offset & 0x0fff]);
	tilemap_mark_tile_dirty(fromanc2_tilemap[vram][layer], offset & 0x0fff);
}

WRITE16_HANDLER( fromanc2_videoram_0_w ) { fromanc2_dispvram_w(offset, data, mem_mask, 0, 0); }
WRITE16_HANDLER( fromanc2_videoram_1_w ) { fromanc2_dispvram_w(offset, data, mem_mask, 0, 2); }
WRITE16_HANDLER( fromanc2_videoram_2_w ) { fromanc2_dispvram_w(offset, data, mem_mask, 1, 0); }
WRITE16_HANDLER( fromanc2_videoram_3_w ) { fromanc2_dispvram_w(offset, data, mem_mask, 1, 2); }

WRITE16_HANDLER( fromanc2_gfxreg_0_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][0] = -(data - 0x000); break;
		case 0x01:	fromanc2_scrolly[0][0] = -(data - 0x000); break;
		case 0x02:	fromanc2_scrollx[0][1] = -(data - 0x004); break;
		case 0x03:	fromanc2_scrolly[0][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxreg_1_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[1][0] = -(data - 0x1be); break;
		case 0x01:	fromanc2_scrolly[1][0] = -(data - 0x1ef); break;
		case 0x02:	fromanc2_scrollx[1][1] = -(data - 0x1c2); break;
		case 0x03:	fromanc2_scrolly[1][1] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxreg_2_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][2] = -(data - 0x1c0); break;
		case 0x01:	fromanc2_scrolly[0][2] = -(data - 0x1ef); break;
		case 0x02:	fromanc2_scrollx[0][3] = -(data - 0x1c3); break;
		case 0x03:	fromanc2_scrolly[0][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxreg_3_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[1][2] = -(data - 0x1bf); break;
		case 0x01:	fromanc2_scrolly[1][2] = -(data - 0x1ef); break;
		case 0x02:	fromanc2_scrollx[1][3] = -(data - 0x1c3); break;
		case 0x03:	fromanc2_scrolly[1][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxbank_0_w )
{
	fromanc2_gfxbank[0][0] = (data & 0x000f) >>  0;
	fromanc2_gfxbank[0][1] = (data & 0x00f0) >>  4;
	fromanc2_gfxbank[0][2] = (data & 0x0f00) >>  8;
	fromanc2_gfxbank[0][3] = (data & 0xf000) >> 12;
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][0]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][1]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][2]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][3]);
}

WRITE16_HANDLER( fromanc2_gfxbank_1_w )
{
	fromanc2_gfxbank[1][0] = (data & 0x000f) >>  0;
	fromanc2_gfxbank[1][1] = (data & 0x00f0) >>  4;
	fromanc2_gfxbank[1][2] = (data & 0x0f00) >>  8;
	fromanc2_gfxbank[1][3] = (data & 0xf000) >> 12;
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][0]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][1]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][2]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][3]);
}


INLINE void fromancr_vram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int layer)
{
	int vram = (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&fromanc2_videoram[vram][layer][offset & 0x0fff]);
	tilemap_mark_tile_dirty(fromanc2_tilemap[vram][layer], offset & 0x0fff);
}

WRITE16_HANDLER( fromancr_videoram_0_w ) { fromancr_vram_w(offset, data, mem_mask, 1); }
WRITE16_HANDLER( fromancr_videoram_1_w ) { fromancr_vram_w(offset, data, mem_mask, 0); }
WRITE16_HANDLER( fromancr_videoram_2_w ) { fromancr_vram_w(offset, data, mem_mask, 2); }

WRITE16_HANDLER( fromancr_gfxreg_0_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][0] = -(data - 0x1bf); break;
		case 0x01:	fromanc2_scrolly[0][0] = -(data - 0x1ef); break;
		case 0x02:	fromanc2_scrollx[1][0] = -(data - 0x1c3); break;
		case 0x03:	fromanc2_scrolly[1][0] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromancr_gfxreg_1_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][1] = -(data - 0x000); break;
		case 0x01:	fromanc2_scrolly[0][1] = -(data - 0x000); break;
		case 0x02:	fromanc2_scrollx[1][1] = -(data - 0x004); break;
		case 0x03:	fromanc2_scrolly[1][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

void fromancr_gfxbank_w(int data)
{
	fromanc2_gfxbank[0][0] = (data & 0x0010) >>  4;	// BG (1P)
	fromanc2_gfxbank[0][1] = (data & 0xf000) >> 12;	// FG (1P)
	fromanc2_gfxbank[1][0] = (data & 0x0008) >>  3;	// BG (2P)
	fromanc2_gfxbank[1][1] = (data & 0x0f00) >>  8;	// FG (2P)
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][0]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][1]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][0]);
	tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][1]);
}


INLINE void fromanc4_vram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int layer)
{
	int vram = (offset < 0x4000) ? 0 : 1;

	COMBINE_DATA(&fromanc2_videoram[vram][layer][offset & 0x3fff]);
	tilemap_mark_tile_dirty(fromanc2_tilemap[vram][layer], offset & 0x3fff);
}

WRITE16_HANDLER( fromanc4_videoram_0_w ) { fromanc4_vram_w(offset, data, mem_mask, 2); }
WRITE16_HANDLER( fromanc4_videoram_1_w ) { fromanc4_vram_w(offset, data, mem_mask, 1); }
WRITE16_HANDLER( fromanc4_videoram_2_w ) { fromanc4_vram_w(offset, data, mem_mask, 0); }

WRITE16_HANDLER( fromanc4_gfxreg_0_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][2] = -(data - 0xfbb); break;
		case 0x01:	fromanc2_scrolly[0][2] = -(data - 0x1e4); break;
		case 0x02:	fromanc2_scrollx[1][2] = -(data - 0xfbb); break;
		case 0x03:	fromanc2_scrolly[1][2] = -(data - 0x1e4); break;
		case 0x05:	fromanc2_gfxbank[0][2] = (data & 0x000f) >> 0;
					fromanc2_gfxbank[1][2] = (data & 0x0f00) >> 8;
					tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][2]);
					tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][2]);
					break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc4_gfxreg_1_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][1] = -(data - 0xfba); break;
		case 0x01:	fromanc2_scrolly[0][1] = -(data - 0x1e4); break;
		case 0x02:	fromanc2_scrollx[1][1] = -(data - 0xfba); break;
		case 0x03:	fromanc2_scrolly[1][1] = -(data - 0x1e4); break;
		case 0x05:	fromanc2_gfxbank[0][1] = (data & 0x000f) >> 0;
					fromanc2_gfxbank[1][1] = (data & 0x0f00) >> 8;
					tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][1]);
					tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][1]);
					break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc4_gfxreg_2_w )
{
	switch (offset) {
		case 0x00:	fromanc2_scrollx[0][0] = -(data - 0xfbb); break;
		case 0x01:	fromanc2_scrolly[0][0] = -(data - 0x1e4); break;
		case 0x02:	fromanc2_scrollx[1][0] = -(data - 0xfbb); break;
		case 0x03:	fromanc2_scrolly[1][0] = -(data - 0x1e4); break;
		case 0x05:	fromanc2_gfxbank[0][0] = (data & 0x000f) >> 0;
					fromanc2_gfxbank[1][0] = (data & 0x0f00) >> 8;
					tilemap_mark_all_tiles_dirty(fromanc2_tilemap[0][0]);
					tilemap_mark_all_tiles_dirty(fromanc2_tilemap[1][0]);
					break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:	break;
	}
}


/******************************************************************************

  Start the video hardware emulation.

******************************************************************************/

VIDEO_START( fromanc2 )
{
	fromanc2_tilemap[0][0] = tilemap_create(fromanc2_get_v0_l0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64, 64);
	fromanc2_tilemap[0][1] = tilemap_create(fromanc2_get_v0_l1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[0][2] = tilemap_create(fromanc2_get_v0_l2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[0][3] = tilemap_create(fromanc2_get_v0_l3_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[1][0] = tilemap_create(fromanc2_get_v1_l0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64, 64);
	fromanc2_tilemap[1][1] = tilemap_create(fromanc2_get_v1_l1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[1][2] = tilemap_create(fromanc2_get_v1_l2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[1][3] = tilemap_create(fromanc2_get_v1_l3_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);

	fromanc2_videoram[0][0] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][1] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][2] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][3] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][0] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][1] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][2] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][3] = auto_malloc((64 * 64) * sizeof(UINT16));

	fromanc2_paletteram[0] = auto_malloc(0x800 * 2);
	fromanc2_paletteram[1] = auto_malloc(0x800 * 2);

	tilemap_set_transparent_pen(fromanc2_tilemap[0][1], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[0][2], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[0][3], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][1], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][2], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][3], 0x000);
}

VIDEO_START( fromancr )
{
	fromanc2_tilemap[0][0] = tilemap_create(fromancr_get_v0_l0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64, 64);
	fromanc2_tilemap[0][1] = tilemap_create(fromancr_get_v0_l1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[0][2] = tilemap_create(fromancr_get_v0_l2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[0][3] = 0;
	fromanc2_tilemap[1][0] = tilemap_create(fromancr_get_v1_l0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 64, 64);
	fromanc2_tilemap[1][1] = tilemap_create(fromancr_get_v1_l1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[1][2] = tilemap_create(fromancr_get_v1_l2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	fromanc2_tilemap[1][3] = 0;

	fromanc2_videoram[0][0] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][1] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][2] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][0] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][1] = auto_malloc((64 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][2] = auto_malloc((64 * 64) * sizeof(UINT16));

	fromanc2_paletteram[0] = auto_malloc(0x800 * 2);
	fromanc2_paletteram[1] = auto_malloc(0x800 * 2);

	tilemap_set_transparent_pen(fromanc2_tilemap[0][1], 0x0ff);
	tilemap_set_transparent_pen(fromanc2_tilemap[0][2], 0x0ff);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][1], 0x0ff);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][2], 0x0ff);
}


VIDEO_START( fromanc4 )
{
	fromanc2_tilemap[0][0] = tilemap_create(fromancr_get_v0_l0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 256, 64);
	fromanc2_tilemap[0][1] = tilemap_create(fromancr_get_v0_l1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 256, 64);
	fromanc2_tilemap[0][2] = tilemap_create(fromancr_get_v0_l2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 256, 64);
	fromanc2_tilemap[0][3] = 0;
	fromanc2_tilemap[1][0] = tilemap_create(fromancr_get_v1_l0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8, 8, 256, 64);
	fromanc2_tilemap[1][1] = tilemap_create(fromancr_get_v1_l1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 256, 64);
	fromanc2_tilemap[1][2] = tilemap_create(fromancr_get_v1_l2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 256, 64);
	fromanc2_tilemap[1][3] = 0;

	fromanc2_videoram[0][0] = auto_malloc((256 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][1] = auto_malloc((256 * 64) * sizeof(UINT16));
	fromanc2_videoram[0][2] = auto_malloc((256 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][0] = auto_malloc((256 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][1] = auto_malloc((256 * 64) * sizeof(UINT16));
	fromanc2_videoram[1][2] = auto_malloc((256 * 64) * sizeof(UINT16));

	fromanc2_paletteram[0] = auto_malloc(0x800 * 2);
	fromanc2_paletteram[1] = auto_malloc(0x800 * 2);

	tilemap_set_transparent_pen(fromanc2_tilemap[0][1], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[0][2], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][1], 0x000);
	tilemap_set_transparent_pen(fromanc2_tilemap[1][2], 0x000);
}

/******************************************************************************

  Display refresh

******************************************************************************/

VIDEO_UPDATE( fromanc2 )
{
	if (fromanc2_tilemap[screen][0]) {
		tilemap_set_scrollx(fromanc2_tilemap[screen][0], 0, -fromanc2_scrollx[screen][0]);
		tilemap_set_scrolly(fromanc2_tilemap[screen][0], 0, -fromanc2_scrolly[screen][0]);
	 	tilemap_draw(bitmap,cliprect, fromanc2_tilemap[screen][0], 0, 0);
	}
	if (fromanc2_tilemap[screen][1]) {
		tilemap_set_scrollx(fromanc2_tilemap[screen][1], 0, -fromanc2_scrollx[screen][1]);
		tilemap_set_scrolly(fromanc2_tilemap[screen][1], 0, -fromanc2_scrolly[screen][1]);
	 	tilemap_draw(bitmap,cliprect, fromanc2_tilemap[screen][1], 0, 0);
	}
	if (fromanc2_tilemap[screen][2]) {
		tilemap_set_scrollx(fromanc2_tilemap[screen][2], 0, -fromanc2_scrollx[screen][2]);
		tilemap_set_scrolly(fromanc2_tilemap[screen][2], 0, -fromanc2_scrolly[screen][2]);
	 	tilemap_draw(bitmap,cliprect, fromanc2_tilemap[screen][2], 0, 0);
	}
	if (fromanc2_tilemap[screen][3]) {
		tilemap_set_scrollx(fromanc2_tilemap[screen][3], 0, -fromanc2_scrollx[screen][3]);
		tilemap_set_scrolly(fromanc2_tilemap[screen][3], 0, -fromanc2_scrolly[screen][3]);
	 	tilemap_draw(bitmap,cliprect, fromanc2_tilemap[screen][3], 0, 0);
	}

	return 0;
}
