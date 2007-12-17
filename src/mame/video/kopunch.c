#include "driver.h"

UINT8 *kopunch_videoram2;

static INT8 scroll[2]; // REMOVE
static int gfxbank, gfxflip;

static tilemap *bg_tilemap, *fg_tilemap;

PALETTE_INIT( kopunch )
{
	int i;

	color_prom+=24;	/* first 24 colors are black */
	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( kopunch_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( kopunch_videoram2_w )
{
	kopunch_videoram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( kopunch_scroll_x_w )
{
	scroll[0] = data; // REMOVE
	tilemap_set_scrollx(fg_tilemap, 0, data);
}

WRITE8_HANDLER( kopunch_scroll_y_w )
{
	scroll[1] = data; // REMOVE
	tilemap_set_scrolly(fg_tilemap, 0, data);
}

WRITE8_HANDLER( kopunch_gfxbank_w )
{
	if (gfxbank != (data & 0x07))
	{
		gfxbank = data & 0x07;
		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}

	gfxflip = data & 0x08; // REMOVE

	tilemap_set_flip(fg_tilemap, (data & 0x08) ? TILEMAP_FLIPY : 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = (kopunch_videoram2[tile_index] & 0x7f) + 128 * gfxbank;

	SET_TILE_INFO(1, code, 0, 0);
}

VIDEO_START( kopunch )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 16, 16);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

VIDEO_UPDATE( kopunch )
{
	int offs;

	tilemap_draw(bitmap, &machine->screen[0].visarea, bg_tilemap, 0, 0);
	//tilemap_draw(bitmap, &machine->screen[0].visarea, fg_tilemap, 0, 0);

	for (offs = 1023;offs >= 0;offs--)
	{
		int sx,sy;

		sx = offs % 16;
		sy = offs / 16;

		drawgfx(bitmap,machine->gfx[1],
				(kopunch_videoram2[offs] & 0x7f) + 128 * gfxbank,
				0,
				0,gfxflip,
				8*(sx+8)+scroll[0],8*(8+(gfxflip ? 15-sy : sy))+scroll[1],
				&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
	}
	return 0;
}
