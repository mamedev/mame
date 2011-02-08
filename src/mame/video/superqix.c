/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/superqix.h"




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( pb_get_bg_tile_info )
{
	superqix_state *state = machine->driver_data<superqix_state>();
	int attr = state->videoram[tile_index + 0x400];
	int code = state->videoram[tile_index] + 256 * (attr & 0x7);
	int color = (attr & 0xf0) >> 4;
	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( sqix_get_bg_tile_info )
{
	superqix_state *state = machine->driver_data<superqix_state>();
	int attr = state->videoram[tile_index + 0x400];
	int bank = (attr & 0x04) ? 0 : 1;
	int code = state->videoram[tile_index] + 256 * (attr & 0x03);
	int color = (attr & 0xf0) >> 4;

	if (bank) code += 1024 * state->gfxbank;

	SET_TILE_INFO(bank, code, color, 0);
	tileinfo->group = (attr & 0x08) >> 3;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pbillian )
{
	superqix_state *state = machine->driver_data<superqix_state>();
	state->bg_tilemap = tilemap_create(machine, pb_get_bg_tile_info, tilemap_scan_rows,  8, 8,32,32);

	/* Need to do save state here */
	state->save_item(NAME(state->last_power[0]));
	state->save_item(NAME(state->last_power[1]));
	state->save_item(NAME(state->pbillian_show_power));
}

VIDEO_START( superqix )
{
	superqix_state *state = machine->driver_data<superqix_state>();
	state->fg_bitmap[0] = auto_bitmap_alloc(machine, 256, 256, machine->primary_screen->format());
	state->fg_bitmap[1] = auto_bitmap_alloc(machine, 256, 256, machine->primary_screen->format());
	state->bg_tilemap = tilemap_create(machine, sqix_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);

	tilemap_set_transmask(state->bg_tilemap,0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(state->bg_tilemap,1,0x0001,0xfffe); /* split type 1 has pen 0 transparent in front half */

	state->save_item(NAME(state->gfxbank));
	state->save_item(NAME(state->show_bitmap));
	state->save_item(NAME(*state->fg_bitmap[0]));
	state->save_item(NAME(*state->fg_bitmap[1]));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( superqix_videoram_w )
{
	superqix_state *state = space->machine->driver_data<superqix_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( superqix_bitmapram_w )
{
	superqix_state *state = space->machine->driver_data<superqix_state>();
	if (state->bitmapram[offset] != data)
	{
		int x = 2 * (offset % 128);
		int y = offset / 128 + 16;

		state->bitmapram[offset] = data;

		*BITMAP_ADDR16(state->fg_bitmap[0], y, x)     = data >> 4;
		*BITMAP_ADDR16(state->fg_bitmap[0], y, x + 1) = data & 0x0f;
	}
}

WRITE8_HANDLER( superqix_bitmapram2_w )
{
	superqix_state *state = space->machine->driver_data<superqix_state>();
	if (data != state->bitmapram2[offset])
	{
		int x = 2 * (offset % 128);
		int y = offset / 128 + 16;

		state->bitmapram2[offset] = data;

		*BITMAP_ADDR16(state->fg_bitmap[1], y, x)     = data >> 4;
		*BITMAP_ADDR16(state->fg_bitmap[1], y, x + 1) = data & 0x0f;
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

	coin_counter_w(space->machine, 0,data & 0x02);
	coin_counter_w(space->machine, 1,data & 0x04);

	memory_set_bank(space->machine, "bank1", (data & 0x08) >> 3);

	interrupt_enable_w(space,0,data & 0x10);
	flip_screen_set(space->machine, data & 0x20);
}

WRITE8_HANDLER( superqix_0410_w )
{
	superqix_state *state = space->machine->driver_data<superqix_state>();
	/* bits 0-1 select the tile bank */
	if (state->gfxbank != (data & 0x03))
	{
		state->gfxbank = data & 0x03;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 2 selects which of the two bitmaps to display (for 2 players game) */
	state->show_bitmap = (data & 0x04) >> 2;

	/* bit 3 enables NMI */
	interrupt_enable_w(space,offset,data & 0x08);

	/* bits 4-5 control ROM bank */
	memory_set_bank(space->machine, "bank1", (data & 0x30) >> 4);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void pbillian_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0; offs < machine->generic.spriteram_size; offs += 4)
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
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0; offs < machine->generic.spriteram_size; offs += 4)
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
	superqix_state *state = screen->machine->driver_data<superqix_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	pbillian_draw_sprites(screen->machine, bitmap,cliprect);

	if (state->pbillian_show_power)
	{
		int curr_power;

		curr_power = ((input_port_read(screen->machine, "PADDLE1") & 0x3f) * 100) / 0x3f;
		if (state->last_power[0] != curr_power)
		{
			popmessage	("Power %d%%", curr_power);
			state->last_power[0] = curr_power;
		}

		curr_power = ((input_port_read(screen->machine, "PADDLE2") & 0x3f) * 100) / 0x3f;
		if (state->last_power[1] != curr_power)
		{
			popmessage	("Power %d%%", curr_power);
			state->last_power[1] = curr_power;
		}
	}
	return 0;
}

VIDEO_UPDATE( superqix )
{
	superqix_state *state = screen->machine->driver_data<superqix_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1, 0);
	copybitmap_trans(bitmap,state->fg_bitmap[state->show_bitmap],flip_screen_get(screen->machine),flip_screen_get(screen->machine),0,0,cliprect,0);
	superqix_draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
