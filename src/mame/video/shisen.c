#include "driver.h"

static int gfxbank;

static tilemap *bg_tilemap;

WRITE8_HANDLER( sichuan2_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( sichuan2_bankswitch_w )
{
	int bankaddress;
	int bank;
	UINT8 *RAM = memory_region(REGION_CPU1);

	if (data & 0xc0) logerror("bank switch %02x\n",data);

	/* bits 0-2 select ROM bank */
	bankaddress = 0x10000 + (data & 0x07) * 0x4000;
	memory_set_bankptr(1, &RAM[bankaddress]);

	/* bits 3-5 select gfx bank */
	bank = (data & 0x38) >> 3;

	if (gfxbank != bank)
	{
		gfxbank = bank;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* bits 6-7 unknown */
}

WRITE8_HANDLER( sichuan2_paletteram_w )
{
	paletteram[offset] = data;

	offset &= 0xff;

	palette_set_color_rgb(Machine, offset, pal5bit(paletteram[offset + 0x000]), pal5bit(paletteram[offset + 0x100]), pal5bit(paletteram[offset + 0x200]));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int code = videoram[offs] + ((videoram[offs + 1] & 0x0f) << 8) + (gfxbank << 12);
	int color = (videoram[offs + 1] & 0xf0) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( sichuan2 )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);
}

VIDEO_UPDATE( sichuan2 )
{
	tilemap_draw(bitmap, &machine->screen[0].visarea, bg_tilemap, 0, 0);
	return 0;
}
