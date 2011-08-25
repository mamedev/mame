/***************************************************************************

    SNK 68000 video routines

Notes:
    Search & Rescue uses Y flip on sprites only.
    Street Smart uses X flip on sprites only.

    Seems to be controlled in same byte as flipscreen.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/snk68.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pow_tile_info )
{
	snk68_state *state = machine.driver_data<snk68_state>();
	int tile = state->m_fg_tile_offset + (state->m_pow_fg_videoram[2*tile_index] & 0xff);
	int color = state->m_pow_fg_videoram[2*tile_index+1] & 0x07;

	SET_TILE_INFO(0, tile, color, 0);
}

static TILE_GET_INFO( get_searchar_tile_info )
{
	snk68_state *state = machine.driver_data<snk68_state>();
	int data = state->m_pow_fg_videoram[2*tile_index];
	int tile = data & 0x7ff;
	int color = (data & 0x7000) >> 12;

	// used in the ikari3 intro
	int flags = (data & 0x8000) ? TILE_FORCE_LAYER0 : 0;

	SET_TILE_INFO(0, tile, color, flags);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void common_video_start(running_machine &machine)
{
	snk68_state *state = machine.driver_data<snk68_state>();

	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);

	tilemap_set_scrolldx(state->m_fg_tilemap, 0, machine.primary_screen->width() - 256);
	tilemap_set_scrolldy(state->m_fg_tilemap, 0, machine.primary_screen->height() - 256);
}

VIDEO_START( pow )
{
	snk68_state *state = machine.driver_data<snk68_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_pow_tile_info, tilemap_scan_cols, 8, 8, 32, 32);
	state->m_fg_tile_offset = 0;

	common_video_start(machine);
}

VIDEO_START( searchar )
{
	snk68_state *state = machine.driver_data<snk68_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_searchar_tile_info, tilemap_scan_cols, 8, 8, 32, 32);

	common_video_start(machine);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_HANDLER( pow_spriteram_r )
{
	snk68_state *state = space->machine().driver_data<snk68_state>();

	// streetsj expects the MSB of every 32-bit word to be FF. Presumably RAM
	// exists only for 3 bytes out of 4 and the fourth is unmapped.
	if (!(offset & 1))
		return state->m_spriteram[offset] | 0xff00;
	else
		return state->m_spriteram[offset];
}

WRITE16_HANDLER( pow_spriteram_w )
{
	snk68_state *state = space->machine().driver_data<snk68_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	UINT16 newword = spriteram16[offset];

	if (!(offset & 1))
		data |= 0xff00;

	COMBINE_DATA(&newword);

	if (spriteram16[offset] != newword)
	{
		int vpos = space->machine().primary_screen->vpos();

		if (vpos > 0)
			space->machine().primary_screen->update_partial(vpos - 1);

		spriteram16[offset] = newword;
	}
}

READ16_HANDLER( pow_fg_videoram_r )
{
	snk68_state *state = space->machine().driver_data<snk68_state>();

	// RAM is only 8-bit
	return state->m_pow_fg_videoram[offset] | 0xff00;
}

WRITE16_HANDLER( pow_fg_videoram_w )
{
	snk68_state *state = space->machine().driver_data<snk68_state>();

	data |= 0xff00;
	COMBINE_DATA(&state->m_pow_fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset >> 1);
}

WRITE16_HANDLER( searchar_fg_videoram_w )
{
	snk68_state *state = space->machine().driver_data<snk68_state>();

	// RAM is full 16-bit, though only half of it is used by the hardware
	COMBINE_DATA(&state->m_pow_fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset >> 1);
}

WRITE16_HANDLER( pow_flipscreen16_w )
{
	if (ACCESSING_BITS_0_7)
	{
		snk68_state *state = space->machine().driver_data<snk68_state>();
		state->m_flipscreen = data & 0x08;
		tilemap_set_flip_all(space->machine(), state->m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

		state->m_sprite_flip_axis = data & 0x04;	// for streetsm? though might not be present on this board

		if (state->m_fg_tile_offset != ((data & 0x70) << 4))
		{
			state->m_fg_tile_offset = (data & 0x70) << 4;
			tilemap_mark_all_tiles_dirty(state->m_fg_tilemap);
		}
	}
}

WRITE16_HANDLER( searchar_flipscreen16_w )
{
	if (ACCESSING_BITS_0_7)
	{
		snk68_state *state = space->machine().driver_data<snk68_state>();

		state->m_flipscreen = data & 0x08;
		tilemap_set_flip_all(space->machine(), state->m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

		state->m_sprite_flip_axis = data & 0x04;
	}
}

WRITE16_HANDLER( pow_paletteram16_word_w )
{
	snk68_state *state = space->machine().driver_data<snk68_state>();
	UINT16 newword;
	int r,g,b;

	COMBINE_DATA(&state->m_paletteram[offset]);
	newword = state->m_paletteram[offset];

	r = ((newword >> 7) & 0x1e) | ((newword >> 14) & 0x01);
	g = ((newword >> 3) & 0x1e) | ((newword >> 13) & 0x01) ;
	b = ((newword << 1) & 0x1e) | ((newword >> 12) & 0x01) ;

	palette_set_color_rgb(space->machine(),offset,pal5bit(r),pal5bit(g),pal5bit(b));
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int group)
{
	snk68_state *state = machine.driver_data<snk68_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int flipscreen = state->m_flipscreen;
	int sprite_flip_axis = state->m_sprite_flip_axis;
	const UINT16* tiledata = &spriteram16[0x800*group];

	// pow has 0x4000 tiles and independent x/y flipping
	// the other games have > 0x4000 tiles and flipping in only one direction
	// (globally selected)
	int const is_pow = (machine.gfx[1]->total_elements <= 0x4000);
	int offs;

	for (offs = 0; offs < 0x800; offs += 0x40)
	{
		int mx = (spriteram16[offs + 2*group] & 0xff) << 4;
		int my = spriteram16[offs + 2*group + 1];
		int i;

		mx = mx | (my >> 12);

		mx = ((mx + 16) & 0x1ff) - 16;
		my = -my;

		if (flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		// every sprite is a column 32 tiles (512 pixels) tall
		for (i = 0; i < 0x20; ++i)
		{
			my &= 0x1ff;

			if (my <= cliprect->max_y && my + 15 >= cliprect->min_y)
			{
				int color = *(tiledata++) & 0x7f;
				int tile = *(tiledata++);
				int fx,fy;

				if (is_pow)
				{
					fx = tile & 0x4000;
					fy = tile & 0x8000;
					tile &= 0x3fff;
				}
				else
				{
					if (sprite_flip_axis)
					{
						fx = 0;
						fy = tile & 0x8000;
					}
					else
					{
						fx = tile & 0x8000;
						fy = 0;
					}
					tile &= 0x7fff;
				}

				if (flipscreen)
				{
					fx = !fx;
					fy = !fy;
				}

				drawgfx_transpen(bitmap,cliprect, machine.gfx[1],
						tile,
						color,
						fx, fy,
						mx, my, 0);
			}
			else
			{
				tiledata += 2;
			}

			if (flipscreen)
				my -= 16;
			else
				my += 16;
		}
	}
}


SCREEN_UPDATE( pow )
{
	snk68_state *state = screen->machine().driver_data<snk68_state>();

	bitmap_fill(bitmap, cliprect, 0x7ff);

	/* This appears to be the correct priority order */
	draw_sprites(screen->machine(), bitmap, cliprect, 2);
	draw_sprites(screen->machine(), bitmap, cliprect, 3);
	draw_sprites(screen->machine(), bitmap, cliprect, 1);

	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);
	return 0;
}
