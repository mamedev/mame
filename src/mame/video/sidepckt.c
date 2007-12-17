#include "driver.h"


static tilemap *bg_tilemap;
static int flipscreen;


PALETTE_INIT( sidepckt )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = colorram[tile_index];
	SET_TILE_INFO(
			0,
			videoram[tile_index] + ((attr & 0x07) << 8),
			((attr & 0x10) >> 3) | ((attr & 0x20) >> 5),
			TILE_FLIPX);
	tileinfo->group = (attr & 0x80) >> 7;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( sidepckt )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transmask(bg_tilemap,0,0xff,0x00); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(bg_tilemap,1,0x01,0xfe); /* split type 1 has pen 0 transparent in front half */

	tilemap_set_flip(ALL_TILEMAPS,TILEMAP_FLIPX);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( sidepckt_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( sidepckt_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( sidepckt_flipscreen_w )
{
	flipscreen = data;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? TILEMAP_FLIPY : TILEMAP_FLIPX);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size; offs += 4)
	{
		int sx,sy,code,color,flipx,flipy;

		code = spriteram[offs+3] + ((spriteram[offs+1] & 0x03) << 8);
		color = (spriteram[offs+1] & 0xf0) >> 4;

		sx = spriteram[offs+2]-2;
		sy = spriteram[offs];

		flipx = spriteram[offs+1] & 0x08;
		flipy = spriteram[offs+1] & 0x04;

		drawgfx(bitmap,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
		/* wraparound */
		drawgfx(bitmap,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx-256,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( sidepckt )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	return 0;
}
