#include "emu.h"
#include "includes/mermaid.h"


static const rectangle spritevisiblearea =
{
	0 * 8, 26 * 8 - 1,
	2 * 8, 30 * 8 - 1
};

static const rectangle flip_spritevisiblearea =
{
	6 * 8, 31 * 8 - 1,
	2 * 8, 30 * 8 - 1
};

PALETTE_INIT( mermaid )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x41);

	for (i = 0; i < 0x40; i++)
	{
		int r = 0x21 * BIT(color_prom[i], 0) + 0x47 * BIT(color_prom[i], 1) + 0x97 * BIT(color_prom[i], 2);
		int g = 0x21 * BIT(color_prom[i], 3) + 0x47 * BIT(color_prom[i], 4) + 0x97 * BIT(color_prom[i], 5);
		int b =                                0x47 * BIT(color_prom[i], 6) + 0x97 * BIT(color_prom[i], 7);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* blue background */
	colortable_palette_set_color(machine->colortable, 0x40, MAKE_RGB(0, 0, 0xff));

	/* char/sprite palette */
	for (i = 0; i < 0x40; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* background palette */
	colortable_entry_set_value(machine->colortable, 0x40, 0x20);
	colortable_entry_set_value(machine->colortable, 0x41, 0x21);
	colortable_entry_set_value(machine->colortable, 0x42, 0x40);
	colortable_entry_set_value(machine->colortable, 0x43, 0x21);
}

WRITE8_HANDLER( mermaid_videoram2_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( mermaid_videoram_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( mermaid_colorram_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( mermaid_flip_screen_x_w )
{
	flip_screen_x_set(space->machine, data & 0x01);
}

WRITE8_HANDLER( mermaid_flip_screen_y_w )
{
	flip_screen_y_set(space->machine, data & 0x01);
}

WRITE8_HANDLER( mermaid_bg_scroll_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->bg_scrollram[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, offset, data);
}

WRITE8_HANDLER( mermaid_fg_scroll_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->fg_scrollram[offset] = data;
	tilemap_set_scrolly(state->fg_tilemap, offset, data);
}

WRITE8_HANDLER( rougien_gfxbankswitch1_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->rougien_gfxbank1 = data & 0x01;
}

WRITE8_HANDLER( rougien_gfxbankswitch2_w )
{
	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	state->rougien_gfxbank2 = data & 0x01;
}

READ8_HANDLER( mermaid_collision_r )
{
	/*
        collision register active LOW:

    with coll = spriteram[offs + 2] & 0xc0

        Bit 0 - Sprite (coll = 0x40) - Sprite (coll = 0x00)
        Bit 1 - Sprite (coll = 0x40) - Foreground
        Bit 2 - Sprite (coll = 0x40) - Background
        Bit 3 - Sprite (coll = 0x80) - Sprite (coll = 0x00)
        Bit 4
        Bit 5
        Bit 6 - Sprite (coll = 0x40) - Sprite (coll = 0x80)
        Bit 7
    */

	mermaid_state *state = (mermaid_state *)space->machine->driver_data;
	int collision = 0xff;

	if (state->coll_bit0) collision &= 0xfe;
	if (state->coll_bit1) collision &= 0xfd;
	if (state->coll_bit2) collision &= 0xfb;
	if (state->coll_bit3) collision &= 0xf7;
	if (state->coll_bit6) collision &= 0xbf;

	return collision;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mermaid_state *state = (mermaid_state *)machine->driver_data;
	int code = state->videoram2[tile_index];
	int sx = tile_index % 32;
	int color = (sx >= 26) ? 0 : 1;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	mermaid_state *state = (mermaid_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	code |= state->rougien_gfxbank1 * 0x2800;
	code |= state->rougien_gfxbank2 * 0x2400;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( mermaid )
{
	mermaid_state *state = (mermaid_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->fg_tilemap, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state->helper = machine->primary_screen->alloc_compatible_bitmap();
	state->helper2 = machine->primary_screen->alloc_compatible_bitmap();
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	mermaid_state *state = (mermaid_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int color = attr & 0x0f;
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		if (sx >= 0xf0) sx -= 256;

		code |= state->rougien_gfxbank1 * 0x2800;
		code |= state->rougien_gfxbank2 * 0x2400;

		if (flip_screen_x_get(machine))
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get(machine))
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap, (flip_screen_x_get(machine) ? &flip_spritevisiblearea : &spritevisiblearea),
			machine->gfx[1], code, color, flipx, flipy, sx, sy, 0);
	}
}

VIDEO_UPDATE( mermaid )
{
	mermaid_state *state = (mermaid_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

static UINT8 collision_check( running_machine *machine, rectangle* rect )
{
	mermaid_state *state = (mermaid_state *)machine->driver_data;
	UINT8 data = 0;

	int x;
	int y;

	for (y = rect->min_y; y <= rect->max_y; y++)
		for (x = rect->min_x; x <= rect->max_x; x++)
		{
			UINT16 a = colortable_entry_get_value(machine->colortable, *BITMAP_ADDR16(state->helper, y, x));
			UINT16 b = colortable_entry_get_value(machine->colortable, *BITMAP_ADDR16(state->helper2, y, x));

			if (b != 0)
				if ((a != 0) & (a != 0x40))
					data |= 0x01;
		}

	return data;
}

VIDEO_EOF( mermaid )
{
	mermaid_state *state = (mermaid_state *)machine->driver_data;
	const rectangle &visarea = machine->primary_screen->visible_area();
	UINT8 *spriteram = state->spriteram;

	int offs, offs2;

	state->coll_bit1 = 0;
	state->coll_bit2 = 0;
	state->coll_bit3 = 0;
	state->coll_bit6 = 0;
	state->coll_bit0 = 0;

	// check for bit 0 (sprite-sprite), 1 (sprite-foreground), 2 (sprite-background)

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int coll = (attr & 0xc0) >> 6;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		rectangle rect;

		if (coll != 1) continue;

		code |= state->rougien_gfxbank1 * 0x2800;
		code |= state->rougien_gfxbank2 * 0x2400;

		if (flip_screen_x_get(machine))
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get(machine))
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		rect.min_x = sx;
		rect.min_y = sy;
		rect.max_x = sx + machine->gfx[1]->width - 1;
		rect.max_y = sy + machine->gfx[1]->height - 1;

		if (rect.min_x < visarea.min_x)
			rect.min_x = visarea.min_x;
		if (rect.min_y < visarea.min_y)
			rect.min_y = visarea.min_y;
		if (rect.max_x > visarea.max_x)
			rect.max_x = visarea.max_x;
		if (rect.max_y > visarea.max_y)
			rect.max_y = visarea.max_y;

		// check collision sprite - background

		bitmap_fill(state->helper, &rect, 0);
		bitmap_fill(state->helper2, &rect, 0);

		tilemap_draw(state->helper, &rect, state->bg_tilemap, 0, 0);

		drawgfx_transpen(state->helper2, &rect, machine->gfx[1], code, 0, flipx, flipy, sx, sy, 0);

		state->coll_bit2 |= collision_check(machine, &rect);

		// check collision sprite - foreground

		bitmap_fill(state->helper, &rect, 0);
		bitmap_fill(state->helper2, &rect, 0);

		tilemap_draw(state->helper, &rect, state->fg_tilemap, 0, 0);

		drawgfx_transpen(state->helper2, &rect, machine->gfx[1], code, 0, flipx, flipy, sx, sy, 0);

		state->coll_bit1 |= collision_check(machine, &rect);

		// check collision sprite - sprite

		bitmap_fill(state->helper, &rect, 0);
		bitmap_fill(state->helper2, &rect, 0);

		for (offs2 = state->spriteram_size - 4; offs2 >= 0; offs2 -= 4)
			if (offs != offs2)
			{
				int attr2 = spriteram[offs2 + 2];
				int bank2 = (attr2 & 0x30) >> 4;
				int coll2 = (attr2 & 0xc0) >> 6;
				int code2 = (spriteram[offs2] & 0x3f) | (bank2 << 6);
				int flipx2 = spriteram[offs2] & 0x40;
				int flipy2 = spriteram[offs2] & 0x80;
				int sx2 = spriteram[offs2 + 3] + 1;
				int sy2 = 240 - spriteram[offs2 + 1];

				if (coll2 != 0) continue;

				code2 |= state->rougien_gfxbank1 * 0x2800;
				code2 |= state->rougien_gfxbank2 * 0x2400;

				if (flip_screen_x_get(machine))
				{
					flipx2 = !flipx2;
					sx2 = 240 - sx2;
				}

				if (flip_screen_y_get(machine))
				{
					flipy2 = !flipy2;
					sy2 = 240 - sy2;
				}

				drawgfx_transpen(state->helper, &rect, machine->gfx[1], code2, 0, flipx2, flipy2, sx2, sy2, 0);
			}

		drawgfx_transpen(state->helper2, &rect, machine->gfx[1], code, 0, flipx, flipy, sx, sy, 0);

		state->coll_bit0 |= collision_check(machine, &rect);
	}

	// check for bit 3 (sprite-sprite)

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int coll = (attr & 0xc0) >> 6;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		rectangle rect;

		if (coll != 2) continue;

		code |= state->rougien_gfxbank1 * 0x2800;
		code |= state->rougien_gfxbank2 * 0x2400;

		if (flip_screen_x_get(machine))
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get(machine))
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		rect.min_x = sx;
		rect.min_y = sy;
		rect.max_x = sx + machine->gfx[1]->width - 1;
		rect.max_y = sy + machine->gfx[1]->height - 1;

		if (rect.min_x < visarea.min_x)
			rect.min_x = visarea.min_x;
		if (rect.min_y < visarea.min_y)
			rect.min_y = visarea.min_y;
		if (rect.max_x > visarea.max_x)
			rect.max_x = visarea.max_x;
		if (rect.max_y > visarea.max_y)
			rect.max_y = visarea.max_y;

		// check collision sprite - sprite

		bitmap_fill(state->helper, &rect, 0);
		bitmap_fill(state->helper2, &rect, 0);

		for (offs2 = state->spriteram_size - 4; offs2 >= 0; offs2 -= 4)
			if (offs != offs2)
			{
				int attr2 = spriteram[offs2 + 2];
				int bank2 = (attr2 & 0x30) >> 4;
				int coll2 = (attr2 & 0xc0) >> 6;
				int code2 = (spriteram[offs2] & 0x3f) | (bank2 << 6);
				int flipx2 = spriteram[offs2] & 0x40;
				int flipy2 = spriteram[offs2] & 0x80;
				int sx2 = spriteram[offs2 + 3] + 1;
				int sy2 = 240 - spriteram[offs2 + 1];

				if (coll2 != 0) continue;

				code2 |= state->rougien_gfxbank1 * 0x2800;
				code2 |= state->rougien_gfxbank2 * 0x2400;

				if (flip_screen_x_get(machine))
				{
					flipx2 = !flipx2;
					sx2 = 240 - sx2;
				}

				if (flip_screen_y_get(machine))
				{
					flipy2 = !flipy2;
					sy2 = 240 - sy2;
				}

				drawgfx_transpen(state->helper, &rect, machine->gfx[1], code2, 0, flipx2, flipy2, sx2, sy2, 0);
			}

		drawgfx_transpen(state->helper2, &rect, machine->gfx[1], code, 0, flipx, flipy, sx, sy, 0);

		state->coll_bit3 |= collision_check(machine, &rect);
	}

	// check for bit 6

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int coll = (attr & 0xc0) >> 6;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		rectangle rect;

		if (coll != 1) continue;

		code |= state->rougien_gfxbank1 * 0x2800;
		code |= state->rougien_gfxbank2 * 0x2400;

		if (flip_screen_x_get(machine))
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get(machine))
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		rect.min_x = sx;
		rect.min_y = sy;
		rect.max_x = sx + machine->gfx[1]->width - 1;
		rect.max_y = sy + machine->gfx[1]->height - 1;

		if (rect.min_x < visarea.min_x)
			rect.min_x = visarea.min_x;
		if (rect.min_y < visarea.min_y)
			rect.min_y = visarea.min_y;
		if (rect.max_x > visarea.max_x)
			rect.max_x = visarea.max_x;
		if (rect.max_y > visarea.max_y)
			rect.max_y = visarea.max_y;

		// check collision sprite - sprite

		bitmap_fill(state->helper, &rect, 0);
		bitmap_fill(state->helper2, &rect, 0);

		for (offs2 = state->spriteram_size - 4; offs2 >= 0; offs2 -= 4)
			if (offs != offs2)
			{
				int attr2 = spriteram[offs2 + 2];
				int bank2 = (attr2 & 0x30) >> 4;
				int coll2 = (attr2 & 0xc0) >> 6;
				int code2 = (spriteram[offs2] & 0x3f) | (bank2 << 6);
				int flipx2 = spriteram[offs2] & 0x40;
				int flipy2 = spriteram[offs2] & 0x80;
				int sx2 = spriteram[offs2 + 3] + 1;
				int sy2 = 240 - spriteram[offs2 + 1];

				if (coll2 != 2) continue;

				code2 |= state->rougien_gfxbank1 * 0x2800;
				code2 |= state->rougien_gfxbank2 * 0x2400;

				if (flip_screen_x_get(machine))
				{
					flipx2 = !flipx2;
					sx2 = 240 - sx2;
				}

				if (flip_screen_y_get(machine))
				{
					flipy2 = !flipy2;
					sy2 = 240 - sy2;
				}

				drawgfx_transpen(state->helper, &rect, machine->gfx[1], code2, 0, flipx2, flipy2, sx2, sy2, 0);
			}

		drawgfx_transpen(state->helper2, &rect, machine->gfx[1], code, 0, flipx, flipy, sx, sy, 0);

		state->coll_bit6 |= collision_check(machine, &rect);
	}

}
