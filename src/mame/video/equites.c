#include "emu.h"
#include "includes/equites.h"


/*************************************
 *
 *  Palette handling
 *
 *************************************/

PALETTE_INIT( equites )
{
	int i;

	machine->colortable = colortable_alloc(machine, 256);

	for (i = 0; i < 256; i++)
		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(pal4bit(color_prom[i]), pal4bit(color_prom[i + 0x100]), pal4bit(color_prom[i + 0x200])));

	// point to the CLUT
	color_prom += 0x380;

	for (i = 0; i < 256; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	for (i = 0; i < 0x80; i++)
		colortable_entry_set_value(machine->colortable, i + 0x100, color_prom[i]);
}

PALETTE_INIT( splndrbt )
{
	int i;

	machine->colortable = colortable_alloc(machine, 256);

	for (i = 0; i < 0x100; i++)
		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(pal4bit(color_prom[i]), pal4bit(color_prom[i + 0x100]), pal4bit(color_prom[i + 0x200])));

	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	// point to the bg CLUT
	color_prom += 0x300;

	for (i = 0; i < 0x80; i++)
		colortable_entry_set_value(machine->colortable, i + 0x100, color_prom[i] + 0x10);

	// point to the sprite CLUT
	color_prom += 0x100;

	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine->colortable, i + 0x180, color_prom[i]);
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

static TILE_GET_INFO( equites_fg_info )
{
	equites_state *state = (equites_state *)machine->driver_data;
	int tile = state->fg_videoram[2 * tile_index];
	int color = state->fg_videoram[2 * tile_index + 1] & 0x1f;

	SET_TILE_INFO(0, tile, color, 0);
	if (color & 0x10)
		tileinfo->flags |= TILE_FORCE_LAYER0;
}

static TILE_GET_INFO( splndrbt_fg_info )
{
	equites_state *state = (equites_state *)machine->driver_data;
	int tile = state->fg_videoram[2 * tile_index] + (state->fg_char_bank << 8);
	int color = state->fg_videoram[2 * tile_index + 1] & 0x3f;

	SET_TILE_INFO(0, tile, color, 0);
	if (color & 0x10)
		tileinfo->flags |= TILE_FORCE_LAYER0;
}

static TILE_GET_INFO( equites_bg_info )
{
	equites_state *state = (equites_state *)machine->driver_data;
	int data = state->bg_videoram[tile_index];
	int tile = data & 0x1ff;
	int color = (data & 0xf000) >> 12;
	int fxy = (data & 0x0600) >> 9;

	SET_TILE_INFO(1, tile, color, TILE_FLIPXY(fxy));
}

static TILE_GET_INFO( splndrbt_bg_info )
{
	equites_state *state = (equites_state *)machine->driver_data;
	int data = state->bg_videoram[tile_index];
	int tile = data & 0x1ff;
	int color = (data & 0xf800) >> 11;
	int fxy = (data & 0x0600) >> 9;

	SET_TILE_INFO(1, tile, color, TILE_FLIPXY(fxy));
	tileinfo->group = color;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( equites )
{
	equites_state *state = (equites_state *)machine->driver_data;
	state->fg_videoram = auto_alloc_array(machine, UINT8, 0x800);
	state_save_register_global_pointer(machine, state->fg_videoram, 0x800);

	state->fg_tilemap = tilemap_create(machine, equites_fg_info, tilemap_scan_cols,  8, 8, 32, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state->bg_tilemap = tilemap_create(machine, equites_bg_info, tilemap_scan_rows, 16, 16, 16, 16);
	tilemap_set_transparent_pen(state->bg_tilemap, 0);
	tilemap_set_scrolldx(state->bg_tilemap, 0, -10);
}

VIDEO_START( splndrbt )
{
	equites_state *state = (equites_state *)machine->driver_data;
	assert(machine->primary_screen->format() == BITMAP_FORMAT_INDEXED16);

	state->fg_videoram = auto_alloc_array(machine, UINT8, 0x800);
	state_save_register_global_pointer(machine, state->fg_videoram, 0x800);

	state->fg_tilemap = tilemap_create(machine, splndrbt_fg_info, tilemap_scan_cols,  8, 8, 32, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_scrolldx(state->fg_tilemap, 8, -8);

	state->bg_tilemap = tilemap_create(machine, splndrbt_bg_info, tilemap_scan_rows, 16, 16, 32, 32);
	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[1], 0x10);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ16_HANDLER(equites_fg_videoram_r)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	return 0xff00 | state->fg_videoram[offset];
}

WRITE16_HANDLER(equites_fg_videoram_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
	{
		state->fg_videoram[offset] = data & 0xff;

		tilemap_mark_tile_dirty(state->fg_tilemap, offset >> 1);
	}
}

WRITE16_HANDLER(equites_bg_videoram_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	COMBINE_DATA(state->bg_videoram + offset);

	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE16_HANDLER(equites_bgcolor_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	if (ACCESSING_BITS_8_15)
		state->bgcolor = data >> 8;
}

WRITE16_HANDLER(equites_scrollreg_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
		tilemap_set_scrolly(state->bg_tilemap, 0, data & 0xff);

	if (ACCESSING_BITS_8_15)
		tilemap_set_scrollx(state->bg_tilemap, 0, data >> 8);
}

WRITE16_HANDLER(splndrbt_selchar0_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	if (state->fg_char_bank != 0)
	{
		state->fg_char_bank = 0;
		tilemap_mark_all_tiles_dirty(state->fg_tilemap);
	}
}

WRITE16_HANDLER(splndrbt_selchar1_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	if (state->fg_char_bank != 1)
	{
		state->fg_char_bank = 1;
		tilemap_mark_all_tiles_dirty(state->fg_tilemap);
	}
}

WRITE16_HANDLER(equites_flip0_w)
{
	flip_screen_set(space->machine, 0);
}

WRITE16_HANDLER(equites_flip1_w)
{
	flip_screen_set(space->machine, 1);
}

WRITE16_HANDLER(splndrbt_flip0_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
		flip_screen_set(space->machine, 0);

	if (ACCESSING_BITS_8_15)
		state->bgcolor = data >> 8;
}

WRITE16_HANDLER(splndrbt_flip1_w)
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(space->machine, 1);
}

WRITE16_HANDLER(splndrbt_bg_scrollx_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	COMBINE_DATA(&state->splndrbt_bg_scrollx);
}

WRITE16_HANDLER(splndrbt_bg_scrolly_w)
{
	equites_state *state = (equites_state *)space->machine->driver_data;
	COMBINE_DATA(&state->splndrbt_bg_scrolly);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void equites_draw_sprites_block( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int start, int end )
{
	equites_state *state = (equites_state *)machine->driver_data;
	int offs;

	for (offs = end - 2; offs >= start; offs -= 2)
	{
		int attr = state->spriteram[offs + 1];
		if (!(attr & 0x800))	// disable or x MSB?
		{
			int tile = attr & 0x1ff;
			int fx = ~attr & 0x400;
			int fy = ~attr & 0x200;
			int color = (~attr & 0xf000) >> 12;
			int sx = (state->spriteram[offs] & 0xff00) >> 8;
			int sy = (state->spriteram[offs] & 0x00ff);
			int transmask = colortable_get_transpen_mask(machine->colortable, machine->gfx[2], color, 0);

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				fx = !fx;
				fy = !fy;
			}

			// align
			sx -= 4;

			// sprites are 16x14 centered in a 16x16 square, so skip the first line
			sy += 1;

			drawgfx_transmask(bitmap,cliprect, machine->gfx[2],
					tile,
					color,
					fx, fy,
					sx, sy, transmask);
		}
	}
}

static void equites_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	// note that we draw the sprites in three blocks; in each blocks, sprites at
	// a lower address have priority. This gives good priorities in gekisou.
	equites_draw_sprites_block(machine, bitmap, cliprect, 0x000/2, 0x060/2);
	equites_draw_sprites_block(machine, bitmap, cliprect, 0x0e0/2, 0x100/2);
	equites_draw_sprites_block(machine, bitmap, cliprect, 0x1a4/2, 0x200/2);
}


/*
This is (probabbly) the sprite x scaling PROM.
The layout is strange. Clearly every line os for one xscale setting. However,
it seems that bytes 0-3 are handled separately from bytes 4-F.
Also, note that sprites are 30x30, not 32x32.
00020200 00000000 00000000 00000000
00020200 01000000 00000000 00000002
00020200 01000000 01000002 00000002
00020200 01000100 01000002 00020002
00020200 01000100 01010202 00020002
02020201 01000100 01010202 00020002
02020201 01010100 01010202 00020202
02020201 01010101 01010202 02020202
02020201 03010101 01010202 02020203
02020201 03010103 01010202 03020203
02020201 03010103 01030302 03020203
02020201 03010303 01030302 03030203
03020203 03010303 01030302 03030203
03020203 03030303 01030302 03030303
03020203 03030303 03030303 03030303
03020303 03030303 03030303 03030303
*/

static void splndrbt_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	equites_state *state = (equites_state *)machine->driver_data;
	const UINT8 * const xrom = memory_region(machine, "user2");
	const UINT8 * const yrom = xrom + 0x100;
	const gfx_element* const gfx = machine->gfx[2];
	int offs;

	// note that sprites are actually 30x30, contained in 32x32 squares. The outer edge is not used.

	for (offs = 0x3f; offs < 0x6f; offs += 2)	// 24 sprites
	{
		int data = state->spriteram[offs];
		int fx = (data & 0x2000) >> 13;
		int fy = (data & 0x1000) >> 12;
		int tile = data & 0x007f;
		int scaley = (data & 0x0f00) >> 8;
		int data2 = state->spriteram[offs + 1];
		int color = (data2 & 0x1f00) >> 8;
		int sx = data2 & 0x00ff;
		int sy = state->spriteram_2[offs + 0] & 0x00ff;
		int scalex = state->spriteram_2[offs + 1] & 0x000f;
		int transmask = colortable_get_transpen_mask(machine->colortable, gfx, color, 0);

//      const UINT8 * const xromline = xrom + (scalex << 4);
		const UINT8 * const yromline = yrom + (scaley << 4) + (15 - scaley);
		const UINT8* const srcgfx = gfx_element_get_data(gfx, tile);
		const pen_t *paldata = &machine->pens[gfx->color_base + gfx->color_granularity * color];
		int x,yy;

		sy += 16;

		if (flip_screen_get(machine))
		{
			// sx NOT inverted
			fx = fx ^ 1;
			fy = fy ^ 1;
		}
		else
		{
			sy = 256 - sy;
		}

		for (yy = 0; yy <= scaley; ++yy)
		{
			int const line = yromline[yy];
			int yhalf;

			for (yhalf = 0; yhalf < 2; ++yhalf)	// top or bottom half
			{
				int const y = yhalf ? sy + 1 + yy : sy - yy;

				if (y >= cliprect->min_y && y <= cliprect->max_y)
				{
					for (x = 0; x <= (scalex << 1); ++x)
					{
						int bx = (sx + x) & 0xff;

						if (bx >= cliprect->min_x && bx <= cliprect->max_x)
						{
							int xx = scalex ? (x * 29 + scalex) / (scalex << 1) + 1 : 16;	// FIXME This is wrong. Should use the PROM.
							int const offset = (fx ? (31 - xx) : xx) + ((fy ^ yhalf) ? (16 + line) : (15 - line) ) * gfx->line_modulo;

							int pen = srcgfx[offset];

							if ((transmask & (1 << pen)) == 0)
								*BITMAP_ADDR16(bitmap, y, bx) = paldata[pen];
						}
					}
				}
			}
		}
	}
}


static void splndrbt_copy_bg( running_machine *machine, bitmap_t *dst_bitmap, const rectangle *cliprect )
{
	equites_state *state = (equites_state *)machine->driver_data;
	bitmap_t * const src_bitmap = tilemap_get_pixmap(state->bg_tilemap);
	bitmap_t * const flags_bitmap = tilemap_get_flagsmap(state->bg_tilemap);
	const UINT8 * const xrom = memory_region(machine, "user1");
	const UINT8 * const yrom = xrom + 0x2000;
	int scroll_x = state->splndrbt_bg_scrollx;
	int scroll_y = state->splndrbt_bg_scrolly;
	int const dinvert = flip_screen_get(machine) ? 0xff : 0x00;
	int src_y = 0;
	int dst_y;

	if (flip_screen_get(machine))
	{
		scroll_x = -scroll_x - 8;
		scroll_y = -scroll_y;
	}

	for (dst_y = 32; dst_y < 256-32; ++dst_y)
	{
		if (dst_y >= cliprect->min_y && dst_y <= cliprect->max_y)
		{
			const UINT8 * const romline = &xrom[(dst_y ^ dinvert) << 5];
			const UINT16 * const src_line = BITMAP_ADDR16(src_bitmap, (src_y + scroll_y) & 0x1ff, 0);
			const UINT8 * const flags_line = BITMAP_ADDR8(flags_bitmap, (src_y + scroll_y) & 0x1ff, 0);
			UINT16 * const dst_line = BITMAP_ADDR16(dst_bitmap, dst_y, 0);
			int dst_x = 0;
			int src_x;

			for (src_x = 0; src_x < 256 && dst_x < 128; ++src_x)
			{
				if ((romline[31 - (src_x >> 3)] >> (src_x & 7)) & 1)
				{
					int sx;

					sx = (256+128 + scroll_x + src_x) & 0x1ff;
					if (flags_line[sx] & TILEMAP_PIXEL_LAYER0)
						dst_line[128 + dst_x] = src_line[sx];

					sx = (255+128 + scroll_x - src_x) & 0x1ff;
					if (flags_line[sx] & TILEMAP_PIXEL_LAYER0)
						dst_line[127 - dst_x] = src_line[sx];

					++dst_x;
				}
			}
		}

		src_y += 1 + yrom[dst_y ^ dinvert];
	}
}



VIDEO_UPDATE( equites )
{
	equites_state *state = (equites_state *)screen->machine->driver_data;
	bitmap_fill(bitmap, cliprect, state->bgcolor);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	equites_draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	return 0;
}

VIDEO_UPDATE( splndrbt )
{
	equites_state *state = (equites_state *)screen->machine->driver_data;
	bitmap_fill(bitmap, cliprect, state->bgcolor);

	splndrbt_copy_bg(screen->machine, bitmap, cliprect);

	if (state->fg_char_bank)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	splndrbt_draw_sprites(screen->machine, bitmap, cliprect);

	if (!state->fg_char_bank)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	return 0;
}
