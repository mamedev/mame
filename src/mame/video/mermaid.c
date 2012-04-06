#include "emu.h"
#include "includes/mermaid.h"


PALETTE_INIT( mermaid )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x41);

	for (i = 0; i < 0x40; i++)
	{
		int r = 0x21 * BIT(color_prom[i], 0) + 0x47 * BIT(color_prom[i], 1) + 0x97 * BIT(color_prom[i], 2);
		int g = 0x21 * BIT(color_prom[i], 3) + 0x47 * BIT(color_prom[i], 4) + 0x97 * BIT(color_prom[i], 5);
		int b =                                0x47 * BIT(color_prom[i], 6) + 0x97 * BIT(color_prom[i], 7);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* blue background */
	colortable_palette_set_color(machine.colortable, 0x40, MAKE_RGB(0, 0, 0xff));

	/* char/sprite palette */
	for (i = 0; i < 0x40; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* background palette */
	colortable_entry_set_value(machine.colortable, 0x40, 0x20);
	colortable_entry_set_value(machine.colortable, 0x41, 0x21);
	colortable_entry_set_value(machine.colortable, 0x42, 0x40);
	colortable_entry_set_value(machine.colortable, 0x43, 0x21);
}

PALETTE_INIT( rougien )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x41);

	for (i = 0; i < 0x40; i++)
	{
		int r = 0x21 * BIT(color_prom[i], 0) + 0x47 * BIT(color_prom[i], 1) + 0x97 * BIT(color_prom[i], 2);
		int g = 0x21 * BIT(color_prom[i], 3) + 0x47 * BIT(color_prom[i], 4) + 0x97 * BIT(color_prom[i], 5);
		int b =                                0x47 * BIT(color_prom[i], 6) + 0x97 * BIT(color_prom[i], 7);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* blue background */
	colortable_palette_set_color(machine.colortable, 0x40, MAKE_RGB(0, 0, 0));

	/* char/sprite palette */
	for (i = 0; i < 0x40; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* background palette */
	colortable_entry_set_value(machine.colortable, 0x40, 0x40);
	colortable_entry_set_value(machine.colortable, 0x41, 0x00);
	colortable_entry_set_value(machine.colortable, 0x42, 0x00);
	colortable_entry_set_value(machine.colortable, 0x43, 0x02);
}


WRITE8_MEMBER(mermaid_state::mermaid_videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mermaid_state::mermaid_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mermaid_state::mermaid_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mermaid_state::mermaid_flip_screen_x_w)
{
	flip_screen_x_set(machine(), data & 0x01);
}

WRITE8_MEMBER(mermaid_state::mermaid_flip_screen_y_w)
{
	flip_screen_y_set(machine(), data & 0x01);
}

WRITE8_MEMBER(mermaid_state::mermaid_bg_scroll_w)
{
	m_bg_scrollram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}

WRITE8_MEMBER(mermaid_state::mermaid_fg_scroll_w)
{
	m_fg_scrollram[offset] = data;
	m_fg_tilemap->set_scrolly(offset, data);
}

WRITE8_MEMBER(mermaid_state::rougien_gfxbankswitch1_w)
{
	m_rougien_gfxbank1 = data & 0x01;
}

WRITE8_MEMBER(mermaid_state::rougien_gfxbankswitch2_w)
{
	m_rougien_gfxbank2 = data & 0x01;
}

READ8_MEMBER(mermaid_state::mermaid_collision_r)
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

	int collision = 0xff;

	if (m_coll_bit0) collision ^= 0x01;
	if (m_coll_bit1) collision ^= 0x02;
	if (m_coll_bit2) collision ^= 0x04;
	if (m_coll_bit3) collision ^= 0x08;
	if (m_coll_bit6) collision ^= 0x40;

	return collision;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mermaid_state *state = machine.driver_data<mermaid_state>();
	int code = state->m_videoram2[tile_index];
	int sx = tile_index % 32;
	int color = (sx >= 26) ? 0 : 1;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	mermaid_state *state = machine.driver_data<mermaid_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	code |= state->m_rougien_gfxbank1 * 0x2800;
	code |= state->m_rougien_gfxbank2 * 0x2400;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( mermaid )
{
	mermaid_state *state = machine.driver_data<mermaid_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap->set_scroll_cols(32);

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap->set_scroll_cols(32);
	state->m_fg_tilemap->set_transparent_pen(0);

	machine.primary_screen->register_screen_bitmap(state->m_helper);
	machine.primary_screen->register_screen_bitmap(state->m_helper2);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const rectangle spritevisiblearea(0 * 8, 26 * 8 - 1, 2 * 8, 30 * 8 - 1);
	const rectangle flip_spritevisiblearea(6 * 8, 31 * 8 - 1, 2 * 8, 30 * 8 - 1);

	mermaid_state *state = machine.driver_data<mermaid_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
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

		code |= state->m_rougien_gfxbank1 * 0x2800;
		code |= state->m_rougien_gfxbank2 * 0x2400;

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

		drawgfx_transpen(bitmap, (flip_screen_x_get(machine) ? flip_spritevisiblearea : spritevisiblearea),
			machine.gfx[1], code, color, flipx, flipy, sx, sy, 0);
	}
}

SCREEN_UPDATE_IND16( mermaid )
{
	mermaid_state *state = screen.machine().driver_data<mermaid_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

static UINT8 collision_check( running_machine &machine, rectangle& rect )
{
	mermaid_state *state = machine.driver_data<mermaid_state>();
	UINT8 data = 0;

	int x;
	int y;

	for (y = rect.min_y; y <= rect.max_y; y++)
		for (x = rect.min_x; x <= rect.max_x; x++)
		{
			UINT16 a = colortable_entry_get_value(machine.colortable, state->m_helper.pix16(y, x)) & 0x3f;
			UINT16 b = colortable_entry_get_value(machine.colortable, state->m_helper2.pix16(y, x)) & 0x3f;

			if (b)
				if (a)
					data |= 0x01;
		}

	return data;
}

SCREEN_VBLANK( mermaid )
{
	// rising edge
	if (vblank_on)
	{
		mermaid_state *state = screen.machine().driver_data<mermaid_state>();
		const rectangle &visarea = screen.machine().primary_screen->visible_area();
		UINT8 *spriteram = state->m_spriteram;

		int offs, offs2;

		state->m_coll_bit0 = 0;
		state->m_coll_bit1 = 0;
		state->m_coll_bit2 = 0;
		state->m_coll_bit3 = 0;
		state->m_coll_bit6 = 0;

		// check for bit 0 (sprite-sprite), 1 (sprite-foreground), 2 (sprite-background)

		for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
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

			code |= state->m_rougien_gfxbank1 * 0x2800;
			code |= state->m_rougien_gfxbank2 * 0x2400;

			if (flip_screen_x_get(screen.machine()))
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			if (flip_screen_y_get(screen.machine()))
			{
				flipy = !flipy;
				sy = 240 - sy;
			}

			rect.min_x = sx;
			rect.min_y = sy;
			rect.max_x = sx + screen.machine().gfx[1]->width - 1;
			rect.max_y = sy + screen.machine().gfx[1]->height - 1;

			rect &= visarea;

			// check collision sprite - background

			state->m_helper.fill(0, rect);
			state->m_helper2.fill(0, rect);

			state->m_bg_tilemap->draw(state->m_helper, rect, 0, 0);

			drawgfx_transpen(state->m_helper2, rect, screen.machine().gfx[1], code, 0, flipx, flipy, sx, sy, 0);

			state->m_coll_bit2 |= collision_check(screen.machine(), rect);

			// check collision sprite - foreground

			state->m_helper.fill(0, rect);
			state->m_helper2.fill(0, rect);

			state->m_fg_tilemap->draw(state->m_helper, rect, 0, 0);

			drawgfx_transpen(state->m_helper2, rect, screen.machine().gfx[1], code, 0, flipx, flipy, sx, sy, 0);

			state->m_coll_bit1 |= collision_check(screen.machine(), rect);

			// check collision sprite - sprite

			state->m_helper.fill(0, rect);
			state->m_helper2.fill(0, rect);

			for (offs2 = state->m_spriteram_size - 4; offs2 >= 0; offs2 -= 4)
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

					code2 |= state->m_rougien_gfxbank1 * 0x2800;
					code2 |= state->m_rougien_gfxbank2 * 0x2400;

					if (flip_screen_x_get(screen.machine()))
					{
						flipx2 = !flipx2;
						sx2 = 240 - sx2;
					}

					if (flip_screen_y_get(screen.machine()))
					{
						flipy2 = !flipy2;
						sy2 = 240 - sy2;
					}

					drawgfx_transpen(state->m_helper, rect, screen.machine().gfx[1], code2, 0, flipx2, flipy2, sx2, sy2, 0);
				}

			drawgfx_transpen(state->m_helper2, rect, screen.machine().gfx[1], code, 0, flipx, flipy, sx, sy, 0);

			state->m_coll_bit0 |= collision_check(screen.machine(), rect);
		}

		// check for bit 3 (sprite-sprite)

		for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
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

			code |= state->m_rougien_gfxbank1 * 0x2800;
			code |= state->m_rougien_gfxbank2 * 0x2400;

			if (flip_screen_x_get(screen.machine()))
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			if (flip_screen_y_get(screen.machine()))
			{
				flipy = !flipy;
				sy = 240 - sy;
			}

			rect.min_x = sx;
			rect.min_y = sy;
			rect.max_x = sx + screen.machine().gfx[1]->width - 1;
			rect.max_y = sy + screen.machine().gfx[1]->height - 1;

			rect &= visarea;

			// check collision sprite - sprite

			state->m_helper.fill(0, rect);
			state->m_helper2.fill(0, rect);

			for (offs2 = state->m_spriteram_size - 4; offs2 >= 0; offs2 -= 4)
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

					code2 |= state->m_rougien_gfxbank1 * 0x2800;
					code2 |= state->m_rougien_gfxbank2 * 0x2400;

					if (flip_screen_x_get(screen.machine()))
					{
						flipx2 = !flipx2;
						sx2 = 240 - sx2;
					}

					if (flip_screen_y_get(screen.machine()))
					{
						flipy2 = !flipy2;
						sy2 = 240 - sy2;
					}

					drawgfx_transpen(state->m_helper, rect, screen.machine().gfx[1], code2, 0, flipx2, flipy2, sx2, sy2, 0);
				}

			drawgfx_transpen(state->m_helper2, rect, screen.machine().gfx[1], code, 0, flipx, flipy, sx, sy, 0);

			state->m_coll_bit3 |= collision_check(screen.machine(), rect);
		}

		// check for bit 6

		for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
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

			code |= state->m_rougien_gfxbank1 * 0x2800;
			code |= state->m_rougien_gfxbank2 * 0x2400;

			if (flip_screen_x_get(screen.machine()))
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			if (flip_screen_y_get(screen.machine()))
			{
				flipy = !flipy;
				sy = 240 - sy;
			}

			rect.min_x = sx;
			rect.min_y = sy;
			rect.max_x = sx + screen.machine().gfx[1]->width - 1;
			rect.max_y = sy + screen.machine().gfx[1]->height - 1;

			rect &= visarea;

			// check collision sprite - sprite

			state->m_helper.fill(0, rect);
			state->m_helper2.fill(0, rect);

			for (offs2 = state->m_spriteram_size - 4; offs2 >= 0; offs2 -= 4)
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

					code2 |= state->m_rougien_gfxbank1 * 0x2800;
					code2 |= state->m_rougien_gfxbank2 * 0x2400;

					if (flip_screen_x_get(screen.machine()))
					{
						flipx2 = !flipx2;
						sx2 = 240 - sx2;
					}

					if (flip_screen_y_get(screen.machine()))
					{
						flipy2 = !flipy2;
						sy2 = 240 - sy2;
					}

					drawgfx_transpen(state->m_helper, rect, screen.machine().gfx[1], code2, 0, flipx2, flipy2, sx2, sy2, 0);
				}

			drawgfx_transpen(state->m_helper2, rect, screen.machine().gfx[1], code, 0, flipx, flipy, sx, sy, 0);

			state->m_coll_bit6 |= collision_check(screen.machine(), rect);
		}
	}
}
