/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/crbaloon.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Balloon has no PROMs, the color code directly maps to a color:
  all bits are inverted
  bit 3 HALF (intensity)
  bit 2 BLUE
  bit 1 GREEN
  bit 0 RED

***************************************************************************/

void crbaloon_state::palette_init()
{
	int i;

	for (i = 0; i < machine().total_colors(); i++)
	{
		UINT8 pen;
		int h, r, g, b;

		if (i & 0x01)
			pen = i >> 1;
		else
			pen = 0x0f;

		h = (~pen & 0x08) ? 0xff : 0x55;
		r = h * ((~pen >> 0) & 1);
		g = h * ((~pen >> 1) & 1);
		b = h * ((~pen >> 2) & 1);

		palette_set_color(machine(), i, MAKE_RGB(r, g, b));
	}
}


WRITE8_MEMBER(crbaloon_state::crbaloon_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(crbaloon_state::crbaloon_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(crbaloon_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void crbaloon_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(crbaloon_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS_FLIP_XY,  8, 8, 32, 32);

	save_item(NAME(m_collision_address));
	save_item(NAME(m_collision_address_clear));
}


UINT16 crbaloon_get_collision_address(running_machine &machine)
{
	crbaloon_state *state = machine.driver_data<crbaloon_state>();
	return state->m_collision_address_clear ? 0xffff : state->m_collision_address;
}


void crbaloon_set_clear_collision_address(running_machine &machine, int _crbaloon_collision_address_clear)
{
	crbaloon_state *state = machine.driver_data<crbaloon_state>();
	state->m_collision_address_clear = !_crbaloon_collision_address_clear; /* active LO */
}



static void draw_sprite_and_check_collision(running_machine &machine, bitmap_ind16 &bitmap)
{
	crbaloon_state *state = machine.driver_data<crbaloon_state>();
	int y;
	UINT8 code = state->m_spriteram[0] & 0x0f;
	UINT8 color = state->m_spriteram[0] >> 4;
	UINT8 sy = state->m_spriteram[2] - 32;

	UINT8 *gfx = state->memregion("gfx2")->base() + (code << 7);


	if (state->flip_screen())
		sy += 32;

	/* assume no collision */
    state->m_collision_address = 0xffff;

	for (y = 0x1f; y >= 0; y--)
	{
		int x;
		UINT8 data = 0;
		UINT8 sx = state->m_spriteram[1];

		for (x = 0x1f; x >= 0; x--)
		{
			int bit;

			if ((x & 0x07) == 0x07)
				/* get next byte to draw, but no drawing in VBLANK */
				data = (sy >= 0xe0) ? 0 : gfx[((x >> 3) << 5) | y];

			bit = data & 0x80;

			/* draw the current pixel, but check collision first */
			if (bit)
			{
				if (bitmap.pix16(sy, sx) & 0x01)
					/* compute the collision address -- the +1 is via observation
                       of the game code, probably wrong for cocktail mode */
					state->m_collision_address = ((((sy ^ 0xff) >> 3) << 5) | ((sx ^ 0xff) >> 3)) + 1;

				bitmap.pix16(sy, sx) = (color << 1) | 1;
			}

			sx = sx + 1;
			data = data << 1;
        }

        sy = sy + 1;
	}
}


UINT32 crbaloon_state::screen_update_crbaloon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprite_and_check_collision(machine(), bitmap);

	return 0;
}
