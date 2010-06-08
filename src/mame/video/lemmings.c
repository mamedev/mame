/***************************************************************************

    Lemmings video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    There are two sets of sprites, the combination of custom chips 52 & 71.
    There is a background pixel layer implemented with discrete logic
    rather than a custom chip and a foreground VRAM tilemap layer that the
    game mostly uses as a pixel layer (the vram format is arranged as
    sequential pixels, rather than sequential characters).

***************************************************************************/

#include "emu.h"
#include "includes/lemmings.h"

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16 *spritedata, int gfxbank, UINT16 pri )
{
	int offs;

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = spritedata[offs + 1] & 0x3fff;

		if ((spritedata[offs + 2] & 0x2000) != pri)
			continue;

		y = spritedata[offs];
		flash = y & 0x1000;
		if (flash && (machine->primary_screen->frame_number() & 1))
			continue;

		x = spritedata[offs + 2];
		colour = (x >>9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

		if (x > 320 || x < -16) continue;

		sprite &= ~multi;
		if (fy)
			inc = 1;
		else
		{
			sprite += multi;
			inc = -1;
		}
		mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[gfxbank],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

/******************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	lemmings_state *state = (lemmings_state *)machine->driver_data;
	UINT16 tile = state->vram_data[tile_index];

	SET_TILE_INFO(
			2,
			tile&0x7ff,
			(tile>>12)&0xf,
			0);
}

VIDEO_START( lemmings )
{
	lemmings_state *state = (lemmings_state *)machine->driver_data;
	state->bitmap0 = auto_bitmap_alloc(machine, 2048, 256, machine->primary_screen->format());
	state->vram_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_cols, 8, 8, 64, 32);

	state->vram_buffer = auto_alloc_array(machine, UINT8, 2048 * 64); /* 64 bytes per VRAM character */
	state->sprite_triple_buffer_0 = auto_alloc_array(machine, UINT16, 0x800 / 2);
	state->sprite_triple_buffer_1 = auto_alloc_array(machine, UINT16, 0x800 / 2);

	tilemap_set_transparent_pen(state->vram_tilemap, 0);
	bitmap_fill(state->bitmap0, 0, 0x100);

	gfx_element_set_source(machine->gfx[2], state->vram_buffer);

	state_save_register_global_bitmap(machine, state->bitmap0);
	state_save_register_global_pointer(machine, state->vram_buffer, 2048 * 64);
	state_save_register_global_pointer(machine, state->sprite_triple_buffer_0, 0x800 / 2);
	state_save_register_global_pointer(machine, state->sprite_triple_buffer_1, 0x800 / 2);
}

VIDEO_EOF( lemmings )
{
	lemmings_state *state = (lemmings_state *)machine->driver_data;

	memcpy(state->sprite_triple_buffer_0, machine->generic.buffered_spriteram.u16, 0x800);
	memcpy(state->sprite_triple_buffer_1, machine->generic.buffered_spriteram2.u16, 0x800);
}

/******************************************************************************/

WRITE16_HANDLER( lemmings_pixel_0_w )
{
	lemmings_state *state = (lemmings_state *)space->machine->driver_data;
	int sx, sy, src, old;

	old = state->pixel_0_data[offset];
	COMBINE_DATA(&state->pixel_0_data[offset]);
	src = state->pixel_0_data[offset];
	if (old == src)
		return;

	sy = (offset << 1) / 0x800;
	sx = (offset << 1) & 0x7ff;

	if (sx > 2047 || sy > 255)
		return;

	*BITMAP_ADDR16(state->bitmap0, sy, sx + 0) = ((src >> 8) & 0xf) | 0x100;
	*BITMAP_ADDR16(state->bitmap0, sy, sx + 1) = ((src >> 0) & 0xf) | 0x100;
}

WRITE16_HANDLER( lemmings_pixel_1_w )
{
	lemmings_state *state = (lemmings_state *)space->machine->driver_data;
	int sx, sy, src, /*old,*/ tile;

//  old = state->pixel_1_data[offset];
	COMBINE_DATA(&state->pixel_1_data[offset]);
	src = state->pixel_1_data[offset];
//  if (old == src)
//      return;

	sy = ((offset << 1) / 0x200);
	sx = ((offset << 1) & 0x1ff);

	/* Copy pixel to buffer for easier decoding later */
	tile = ((sx / 8) * 32) + (sy / 8);
	gfx_element_mark_dirty(space->machine->gfx[2], tile);
	state->vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 8) & 0xf;

	sx += 1; /* Update both pixels in the word */
	state->vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 0) & 0xf;
}

WRITE16_HANDLER( lemmings_vram_w )
{
	lemmings_state *state = (lemmings_state *)space->machine->driver_data;
	COMBINE_DATA(&state->vram_data[offset]);
	tilemap_mark_tile_dirty(state->vram_tilemap, offset);
}

VIDEO_UPDATE( lemmings )
{
	lemmings_state *state = (lemmings_state *)screen->machine->driver_data;
	int x1 = -state->control_data[0];
	int x0 = -state->control_data[2];
	int y = 0;
	rectangle rect;
	rect.max_y = cliprect->max_y;
	rect.min_y = cliprect->min_y;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	draw_sprites(screen->machine, bitmap, cliprect, state->sprite_triple_buffer_1, 1, 0x0000);

	/* Pixel layer can be windowed in hardware (two player mode) */
	if ((state->control_data[6] & 2) == 0)
		copyscrollbitmap_trans(bitmap, state->bitmap0, 1, &x1, 1, &y, cliprect, 0x100);
	else
	{
		rect.max_x = 159;
		rect.min_x = 0;
		copyscrollbitmap_trans(bitmap, state->bitmap0, 1, &x0, 1, &y, &rect, 0x100);
		rect.max_x = 319;
		rect.min_x = 160;
		copyscrollbitmap_trans(bitmap, state->bitmap0, 1, &x1, 1, &y, &rect, 0x100);
	}
	draw_sprites(screen->machine, bitmap, cliprect, state->sprite_triple_buffer_0, 0, 0x0000);
	draw_sprites(screen->machine, bitmap, cliprect, state->sprite_triple_buffer_1, 1, 0x2000);
	tilemap_draw(bitmap, cliprect, state->vram_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect, state->sprite_triple_buffer_0, 0, 0x2000);
	return 0;
}
