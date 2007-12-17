/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


UINT8 *gng_fgvideoram;
UINT8 *gng_bgvideoram;

static tilemap *bg_tilemap,*fg_tilemap;

static UINT8 scrollx[2];
static UINT8 scrolly[2];


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 attr = gng_fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			gng_fgvideoram[tile_index] + ((attr & 0xc0) << 2),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 attr = gng_bgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			1,
			gng_bgvideoram[tile_index] + ((attr & 0xc0) << 2),
			attr & 0x07,
			TILE_FLIPYX((attr & 0x30) >> 4));
	tileinfo->group = (attr & 0x08) >> 3;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gng )
{
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,    16,16,32,32);

	tilemap_set_transparent_pen(fg_tilemap,3);

	tilemap_set_transmask(bg_tilemap,0,0xff,0x00); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(bg_tilemap,1,0x41,0xbe); /* split type 1 has pens 0 and 6 transparent in front half */

	/* register to save state */
	state_save_register_global_array(scrollx);
	state_save_register_global_array(scrolly);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( gng_fgvideoram_w )
{
	gng_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( gng_bgvideoram_w )
{
	gng_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}


WRITE8_HANDLER( gng_bgscrollx_w )
{
	scrollx[offset] = data;
	tilemap_set_scrollx( bg_tilemap, 0, scrollx[0] + 256 * scrollx[1] );
}

WRITE8_HANDLER( gng_bgscrolly_w )
{
	scrolly[offset] = data;
	tilemap_set_scrolly( bg_tilemap, 0, scrolly[0] + 256 * scrolly[1] );
}


WRITE8_HANDLER( gng_flipscreen_w )
{
	flip_screen_set(~data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[2];
	int offs;


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		UINT8 attributes = buffered_spriteram[offs+1];
		int sx = buffered_spriteram[offs + 3] - 0x100 * (attributes & 0x01);
		int sy = buffered_spriteram[offs + 2];
		int flipx = attributes & 0x04;
		int flipy = attributes & 0x08;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,gfx,
				buffered_spriteram[offs] + ((attributes<<2) & 0x300),
				(attributes >> 4) & 3,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,15);
	}
}

VIDEO_UPDATE( gng )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}

VIDEO_EOF( gng )
{
	buffer_spriteram_w(0,0);
}
