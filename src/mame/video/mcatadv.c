/* Magical Cat Adventure / Nostradamus Video Hardware */

/*
Notes:
Tilemap drawing is a killer on the first level of Nost due to the whole tilemap being dirty every frame.
Sprite drawing is quite fast (See USER1 in the profiler)

Nost final boss, the priority of the arms is under the tilemaps, everything else is above. Should it be blended? i.e. Shadow.

ToDo: Fix Sprites & Rowscroll/Select for Cocktail
*/

#include "emu.h"
#include "profiler.h"
#include "includes/mcatadv.h"

static TILE_GET_INFO( get_mcatadv_tile_info1 )
{
	mcatadv_state *state = (mcatadv_state *)machine->driver_data;
	int tileno = state->videoram1[tile_index * 2 + 1];
	int colour = (state->videoram1[tile_index * 2] & 0x3f00) >> 8;
	int pri = (state->videoram1[tile_index * 2] & 0xc000) >> 14;

	SET_TILE_INFO(0,tileno,colour + state->palette_bank1 * 0x40, 0);
	tileinfo->category = pri;
}

WRITE16_HANDLER( mcatadv_videoram1_w )
{
	mcatadv_state *state = (mcatadv_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram1[offset]);
	tilemap_mark_tile_dirty(state->tilemap1, offset / 2);
}

static TILE_GET_INFO( get_mcatadv_tile_info2 )
{
	mcatadv_state *state = (mcatadv_state *)machine->driver_data;
	int tileno = state->videoram2[tile_index * 2 + 1];
	int colour = (state->videoram2[tile_index * 2] & 0x3f00) >> 8;
	int pri = (state->videoram2[tile_index * 2] & 0xc000) >> 14;

	SET_TILE_INFO(1, tileno, colour + state->palette_bank2 * 0x40, 0);
	tileinfo->category = pri;
}

WRITE16_HANDLER( mcatadv_videoram2_w )
{
	mcatadv_state *state = (mcatadv_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram2[offset]);
	tilemap_mark_tile_dirty(state->tilemap2, offset / 2);
}


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	mcatadv_state *state = (mcatadv_state *)machine->driver_data;
	UINT16 *source = state->spriteram_old;
	UINT16 *finish = source + (state->spriteram_size / 2) / 2;
	int global_x = state->vidregs[0] - 0x184;
	int global_y = state->vidregs[1] - 0x1f1;

	UINT16 *destline;
	UINT8 *priline;
	UINT8 *sprdata = memory_region(machine, "gfx1");

	int xstart, xend, xinc;
	int ystart, yend, yinc;

	if (state->vidregs_old[2] == 0x0001) /* Double Buffered */
	{
		source += (state->spriteram_size / 2) / 2;
		finish += (state->spriteram_size / 2) / 2;
	}
	else if (state->vidregs_old[2]) /* I suppose it's possible that there is 4 banks, haven't seen it used though */
	{
		logerror("Spritebank != 0/1\n");
	}

	while (source < finish)
	{
		int pen = (source[0] & 0x3f00) >> 8;
		int tileno = source[1] & 0xffff;
		int pri = (source[0] & 0xc000) >> 14;
		int x = source[2] & 0x3ff;
		int y = source[3] & 0x3ff;
		int flipy = source[0] & 0x0040;
		int flipx = source[0] & 0x0080;

		int height = ((source[3] & 0xf000) >> 12) * 16;
		int width = ((source[2] & 0xf000) >> 12) * 16;
		int offset = tileno * 256;

		int drawxpos, drawypos;
		int xcnt, ycnt;
		int pix;

		if (x & 0x200) x-=0x400;
		if (y & 0x200) y-=0x400;

#if 0 // For Flipscreen/Cocktail
		if(state->vidregs[0] & 0x8000)
		{
			flipx = !flipx;
		}
		if(state->vidregs[1] & 0x8000)
		{
			flipy = !flipy;
		}
#endif

		if (source[3] != source[0]) // 'hack' don't draw sprites while its testing the ram!
		{
			if(!flipx) { xstart = 0;        xend = width;  xinc = 1; }
			else       { xstart = width-1;  xend = -1;     xinc = -1; }
			if(!flipy) { ystart = 0;        yend = height; yinc = 1; }
			else       { ystart = height-1; yend = -1;     yinc = -1; }

			for (ycnt = ystart; ycnt != yend; ycnt += yinc)
			{
				drawypos = y + ycnt - global_y;

				if ((drawypos >= cliprect->min_y) && (drawypos <= cliprect->max_y))
				{
					destline = BITMAP_ADDR16(bitmap, drawypos, 0);
					priline = BITMAP_ADDR8(machine->priority_bitmap, drawypos, 0);

					for (xcnt = xstart; xcnt != xend; xcnt += xinc)
					{
						drawxpos = x + xcnt - global_x;

						if((priline[drawxpos] < pri))
						{
							if (offset >= 0x500000 * 2)
								offset = 0;
							pix = sprdata[offset / 2];

							if (offset & 1)
								pix = pix >> 4;
							pix &= 0x0f;

							if ((drawxpos >= cliprect->min_x) && (drawxpos <= cliprect->max_x) && pix)
								destline[drawxpos] = (pix + (pen << 4));
						}
						offset++;
					}
				}
				else
				{
					offset += width;
				}
			}
		}
		source += 4;
	}
}

static void mcatadv_draw_tilemap_part( UINT16* current_scroll, UINT16* current_videoram1, int i, tilemap_t* current_tilemap, bitmap_t *bitmap, const rectangle *cliprect )
{
	int flip;
	UINT32 drawline;
	rectangle clip;

	clip.min_x = cliprect->min_x;
	clip.max_x = cliprect->max_x;

	for (drawline = cliprect->min_y; drawline <= cliprect->max_y; drawline++)
	{
		int scrollx, scrolly;

		clip.min_y = drawline;
		clip.max_y = drawline;

		scrollx = (current_scroll[0] & 0x1ff) - 0x194;
		scrolly = (current_scroll[1] & 0x1ff) - 0x1df;

		if ((current_scroll[1] & 0x4000) == 0x4000)
		{
			int rowselect = current_videoram1[0x1000 / 2 + (((drawline + scrolly) & 0x1ff) * 2) + 1];
			scrolly = rowselect - drawline;
		}

		if ((current_scroll[0] & 0x4000) == 0x4000)
		{
			int rowscroll = current_videoram1[0x1000 / 2 + (((drawline + scrolly) & 0x1ff) * 2) + 0];
			scrollx += rowscroll;
		}

		/* Global Flip */
		if (!(current_scroll[0] & 0x8000)) scrollx -= 0x19;
		if (!(current_scroll[1] & 0x8000)) scrolly -= 0x141;
		flip = ((current_scroll[0] & 0x8000) ? 0 : TILEMAP_FLIPX) | ((current_scroll[1] & 0x8000) ? 0 : TILEMAP_FLIPY);

		tilemap_set_scrollx(current_tilemap, 0, scrollx);
		tilemap_set_scrolly(current_tilemap, 0, scrolly);
		tilemap_set_flip(current_tilemap, flip);

		tilemap_draw(bitmap, &clip, current_tilemap, i, i);
	}
}

VIDEO_UPDATE( mcatadv )
{
	mcatadv_state *state = (mcatadv_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (state->scroll1[2] != state->palette_bank1)
	{
		state->palette_bank1 = state->scroll1[2];
		tilemap_mark_all_tiles_dirty(state->tilemap1);
	}

	if (state->scroll2[2] != state->palette_bank2)
	{
		state->palette_bank2 = state->scroll2[2];
		tilemap_mark_all_tiles_dirty(state->tilemap2);
	}

/*
    popmessage("%02x %02x %02x %02x",
        (mcatadv_scroll1[0]  & 0x4000) >> 8,
        (mcatadv_scroll1[1]  & 0x4000) >> 8,
        (mcatadv_scroll2[0] & 0x4000) >> 8,
        (mcatadv_scroll2[1] & 0x4000) >> 8);
*/

	for (i = 0; i <= 3; i++)
	{
	#ifdef MAME_DEBUG
			if (!input_code_pressed(screen->machine, KEYCODE_Q))
	#endif
			mcatadv_draw_tilemap_part(state->scroll1,  state->videoram1, i, state->tilemap1, bitmap, cliprect);

	#ifdef MAME_DEBUG
			if (!input_code_pressed(screen->machine, KEYCODE_W))
	#endif
				mcatadv_draw_tilemap_part(state->scroll2, state->videoram2, i, state->tilemap2, bitmap, cliprect);
	}

	profiler_mark_start(PROFILER_USER1);
#ifdef MAME_DEBUG
	if (!input_code_pressed(screen->machine, KEYCODE_E))
#endif
		draw_sprites (screen->machine, bitmap, cliprect);
	profiler_mark_end();
	return 0;
}

VIDEO_START( mcatadv )
{
	mcatadv_state *state = (mcatadv_state *)machine->driver_data;
	state->tilemap1 = tilemap_create(machine, get_mcatadv_tile_info1, tilemap_scan_rows, 16, 16, 32, 32);
	tilemap_set_transparent_pen(state->tilemap1, 0);

	state->tilemap2 = tilemap_create(machine, get_mcatadv_tile_info2, tilemap_scan_rows, 16, 16, 32, 32);
	tilemap_set_transparent_pen(state->tilemap2, 0);

	state->spriteram_old = auto_alloc_array_clear(machine, UINT16, state->spriteram_size / 2);
	state->vidregs_old = auto_alloc_array(machine, UINT16, (0x0f + 1) / 2);

	state->palette_bank1 = 0;
	state->palette_bank2 = 0;

	state_save_register_global_pointer(machine, state->spriteram_old, state->spriteram_size / 2);
	state_save_register_global_pointer(machine, state->vidregs_old, (0x0f + 1) / 2);
}

VIDEO_EOF( mcatadv )
{
	mcatadv_state *state = (mcatadv_state *)machine->driver_data;
	memcpy(state->spriteram_old, state->spriteram, state->spriteram_size);
	memcpy(state->vidregs_old, state->vidregs, 0xf);
}
