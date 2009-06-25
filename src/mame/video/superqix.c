/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *superqix_videoram;
UINT8 *superqix_bitmapram,*superqix_bitmapram2;
int pbillian_show_power;

static int gfxbank;
static bitmap_t *fg_bitmap[2];
static int show_bitmap;
static tilemap *bg_tilemap;
static int last_power[2];


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( pb_get_bg_tile_info )
{
	int attr = superqix_videoram[tile_index + 0x400];
	int code = superqix_videoram[tile_index] + 256 * (attr & 0x7);
	int color = (attr & 0xf0) >> 4;
	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( sqix_get_bg_tile_info )
{
	int attr = superqix_videoram[tile_index + 0x400];
	int bank = (attr & 0x04) ? 0 : 1;
	int code = superqix_videoram[tile_index] + 256 * (attr & 0x03);
	int color = (attr & 0xf0) >> 4;

	if (bank) code += 1024 * gfxbank;

	SET_TILE_INFO(bank, code, color, 0);
	tileinfo->group = (attr & 0x08) >> 3;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pbillian )
{
	bg_tilemap = tilemap_create(machine, pb_get_bg_tile_info, tilemap_scan_rows,  8, 8,32,32);

	/* Need to do save state here */
	state_save_register_global(machine, last_power[0]);
	state_save_register_global(machine, last_power[1]);
	state_save_register_global(machine, pbillian_show_power);
}

VIDEO_START( superqix )
{
	fg_bitmap[0] = auto_bitmap_alloc(machine, 256, 256, video_screen_get_format(machine->primary_screen));
	fg_bitmap[1] = auto_bitmap_alloc(machine, 256, 256, video_screen_get_format(machine->primary_screen));
	bg_tilemap = tilemap_create(machine, sqix_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(bg_tilemap,1,0x0001,0xfffe); /* split type 1 has pen 0 transparent in front half */

	state_save_register_global(machine, gfxbank);
	state_save_register_global(machine, show_bitmap);
	state_save_register_global_bitmap(machine, fg_bitmap[0]);
	state_save_register_global_bitmap(machine, fg_bitmap[1]);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( superqix_videoram_w )
{
	superqix_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( superqix_bitmapram_w )
{
	if (superqix_bitmapram[offset] != data)
	{
		int x = 2 * (offset % 128);
		int y = offset / 128 + 16;

		superqix_bitmapram[offset] = data;

		*BITMAP_ADDR16(fg_bitmap[0], y, x)     = data >> 4;
		*BITMAP_ADDR16(fg_bitmap[0], y, x + 1) = data & 0x0f;
	}
}

WRITE8_HANDLER( superqix_bitmapram2_w )
{
	if (data != superqix_bitmapram2[offset])
	{
		int x = 2 * (offset % 128);
		int y = offset / 128 + 16;

		superqix_bitmapram2[offset] = data;

		*BITMAP_ADDR16(fg_bitmap[1], y, x)     = data >> 4;
		*BITMAP_ADDR16(fg_bitmap[1], y, x + 1) = data & 0x0f;
	}
}

WRITE8_HANDLER( pbillian_0410_w )
{
	/*
     -------0  ? [not used]
     ------1-  coin counter 1
     -----2--  coin counter 2
     ----3---  rom 2 HI (reserved for ROM banking , not used)
     ---4----  nmi enable/disable
     --5-----  flip screen
    */

	coin_counter_w(0,data & 0x02);
	coin_counter_w(1,data & 0x04);

	memory_set_bank(space->machine, 1, (data & 0x08) >> 3);

	interrupt_enable_w(space,0,data & 0x10);
	flip_screen_set(space->machine, data & 0x20);
}

WRITE8_HANDLER( superqix_0410_w )
{
	/* bits 0-1 select the tile bank */
	if (gfxbank != (data & 0x03))
	{
		gfxbank = data & 0x03;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	/* bit 2 selects which of the two bitmaps to display (for 2 players game) */
	show_bitmap = (data & 0x04) >> 2;

	/* bit 3 enables NMI */
	interrupt_enable_w(space,offset,data & 0x08);

	/* bits 4-5 control ROM bank */
	memory_set_bank(space->machine, 1, (data & 0x30) >> 4);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void pbillian_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0; offs < spriteram_size; offs += 4)
	{
		int attr = spriteram[offs + 3];
		int code = ((spriteram[offs] & 0xfc) >> 2) + 64 * (attr & 0x0f);
		int color = (attr & 0xf0) >> 4;
		int sx = spriteram[offs + 1] + 256 * (spriteram[offs] & 0x01);
		int sy = spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,cliprect, machine->gfx[1],
				code,
				color,
				flip_screen_get(machine), flip_screen_get(machine),
				sx, sy, 0);
	}
}

static void superqix_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0; offs < spriteram_size; offs += 4)
	{
		int attr = spriteram[offs + 3];
		int code = spriteram[offs] + 256 * (attr & 0x01);
		int color = (attr & 0xf0) >> 4;
		int flipx = attr & 0x04;
		int flipy = attr & 0x08;
		int sx = spriteram[offs + 1];
		int sy = spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect, machine->gfx[2],
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

VIDEO_UPDATE( pbillian )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	pbillian_draw_sprites(screen->machine, bitmap,cliprect);

	if (pbillian_show_power)
	{
		int curr_power;

		curr_power = ((input_port_read(screen->machine, "PADDLE1") & 0x3f) * 100) / 0x3f;
		if (last_power[0] != curr_power)
		{
			popmessage	("Power %d%%", curr_power);
			last_power[0] = curr_power;
		}

		curr_power = ((input_port_read(screen->machine, "PADDLE2") & 0x3f) * 100) / 0x3f;
		if (last_power[1] != curr_power)
		{
			popmessage	("Power %d%%", curr_power);
			last_power[1] = curr_power;
		}
	}
	return 0;
}

VIDEO_UPDATE( superqix )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_LAYER1, 0);
	copybitmap_trans(bitmap,fg_bitmap[show_bitmap],flip_screen_get(screen->machine),flip_screen_get(screen->machine),0,0,cliprect,0);
	superqix_draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
