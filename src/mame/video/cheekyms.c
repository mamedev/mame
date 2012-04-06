/*************************************************************************
    Universal Cheeky Mouse Driver
    (c)Lee Taylor May 1998, All rights reserved.

    For use only in offical MAME releases.
    Not to be distrabuted as part of any commerical work.
***************************************************************************
Functions to emulate the video hardware of the machine.
***************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "includes/cheekyms.h"

/* bit 3 and 7 of the char color PROMs are used for something -- not currently emulated -
   thus GAME_IMPERFECT_GRAPHICS */

PALETTE_INIT( cheekyms )
{
	int i, j, bit, r, g, b;

	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 0x20; j++)
		{
			/* red component */
			bit = (color_prom[0x20 * (i / 2) + j] >> ((4 * (i & 1)) + 0)) & 0x01;
			r = 0xff * bit;
			/* green component */
			bit = (color_prom[0x20 * (i / 2) + j] >> ((4 * (i & 1)) + 1)) & 0x01;
			g = 0xff * bit;
			/* blue component */
			bit = (color_prom[0x20 * (i / 2) + j] >> ((4 * (i & 1)) + 2)) & 0x01;
			b = 0xff * bit;

			palette_set_color(machine, (i * 0x20) + j, MAKE_RGB(r,g,b));
		}
	}
}


WRITE8_MEMBER(cheekyms_state::cheekyms_port_40_w)
{

	/* the lower bits probably trigger sound samples */
	dac_data_w(m_dac, data ? 0x80 : 0);
}


WRITE8_MEMBER(cheekyms_state::cheekyms_port_80_w)
{

	/* d0-d1 - sound enables, not sure which bit is which */
	/* d3-d5 - man scroll amount */
	/* d6 - palette select (selects either 0 = PROM M9, 1 = PROM M8) */
	/* d7 - screen flip */
	*m_port_80 = data;

	/* d2 - interrupt enable */
	m_irq_mask = data & 4;
}



static TILE_GET_INFO( cheekyms_get_tile_info )
{
	cheekyms_state *state = machine.driver_data<cheekyms_state>();
	int color;

	int x = tile_index & 0x1f;
	int y = tile_index >> 5;
	int code = state->m_videoram[tile_index];
	int palette = (*state->m_port_80 >> 2) & 0x10;

	if (x >= 0x1e)
	{
		if (y < 0x0c)
			color = 0x15;
		else if (y < 0x14)
			color = 0x16;
		else
			color = 0x14;
	}
	else
	{
		if ((y == 0x04) || (y == 0x1b))
			color = palette | 0x0c;
		else
			color = palette | (x >> 1);
	}

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( cheekyms )
{
	cheekyms_state *state = machine.driver_data<cheekyms_state>();
	int width, height;

	width = machine.primary_screen->width();
	height = machine.primary_screen->height();
	state->m_bitmap_buffer = auto_bitmap_ind16_alloc(machine, width, height);

	state->m_cm_tilemap = tilemap_create(machine, cheekyms_get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_cm_tilemap->set_transparent_pen(0);
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const gfx_element *gfx, int flip )
{
	cheekyms_state *state = machine.driver_data<cheekyms_state>();
	offs_t offs;

	for (offs = 0; offs < 0x20; offs += 4)
	{
		int x, y, code, color;

		if ((state->m_spriteram[offs + 3] & 0x08) == 0x00) continue;

		x  = 256 - state->m_spriteram[offs + 2];
		y  = state->m_spriteram[offs + 1];
		code =  (~state->m_spriteram[offs + 0] & 0x0f) << 1;
		color = (~state->m_spriteram[offs + 3] & 0x07);

		if (state->m_spriteram[offs + 0] & 0x80)
		{
			if (!flip)
				code++;

			drawgfx_transpen(bitmap, cliprect, gfx, code, color, 0, 0, x, y, 0);
		}
		else
		{
			if (state->m_spriteram[offs + 0] & 0x02)
			{
				drawgfx_transpen(bitmap, cliprect, gfx, code | 0x20, color, 0, 0,        x, y, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, code | 0x21, color, 0, 0, 0x10 + x, y, 0);
			}
			else
			{
				drawgfx_transpen(bitmap, cliprect, gfx, code | 0x20, color, 0, 0, x,        y, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, code | 0x21, color, 0, 0, x, 0x10 + y, 0);
			}
		}
	}
}


SCREEN_UPDATE_IND16( cheekyms )
{
	cheekyms_state *state = screen.machine().driver_data<cheekyms_state>();
	int y, x;
	int scrolly = ((*state->m_port_80 >> 3) & 0x07);
	int flip = *state->m_port_80 & 0x80;

	screen.machine().tilemap().mark_all_dirty();
	screen.machine().tilemap().set_flip_all(flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	bitmap.fill(0, cliprect);
	state->m_bitmap_buffer->fill(0, cliprect);

	/* sprites go under the playfield */
	draw_sprites(screen.machine(), bitmap, cliprect, screen.machine().gfx[1], flip);

	/* draw the tilemap to a temp bitmap */
	state->m_cm_tilemap->draw(*state->m_bitmap_buffer, cliprect, 0, 0);

	/* draw the tilemap to the final bitmap applying the scroll to the man character */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int in_man_area;

			if (flip)
			{
				in_man_area = (x >= (32 - 12 - 1) * 8 && x < (32 - 8) * 8 && y > 5 * 8 && y < 27 * 8);
			}
			else
			{
				in_man_area = (x >= 8 * 8 && x < 12 * 8 && y > 5 * 8 && y < 27 * 8);
			}

			if (in_man_area)
			{
				if ((y + scrolly) < 27 * 8 && state->m_bitmap_buffer->pix16(y + scrolly, x) != 0)
					bitmap.pix16(y, x) = state->m_bitmap_buffer->pix16(y + scrolly, x);
			}
			else
			{
				if(state->m_bitmap_buffer->pix16(y, x) != 0)
					bitmap.pix16(y, x) = state->m_bitmap_buffer->pix16(y, x);
			}
		}
	}


	return 0;
}
