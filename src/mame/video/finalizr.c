/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/finalizr.h"


PALETTE_INIT( finalizr )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int r = pal4bit(color_prom[i + 0x00] >> 0);
		int g = pal4bit(color_prom[i + 0x00] >> 4);
		int b = pal4bit(color_prom[i + 0x20] >> 0);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	finalizr_state *state = machine.driver_data<finalizr_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0xc0) << 2) + (state->m_charbank << 10);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(0, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	finalizr_state *state = machine.driver_data<finalizr_state>();
	int attr = state->m_colorram2[tile_index];
	int code = state->m_videoram2[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( finalizr )
{
	finalizr_state *state = machine.driver_data<finalizr_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}



WRITE8_MEMBER(finalizr_state::finalizr_videoctrl_w)
{
	m_charbank = data & 3;
	m_spriterambank = data & 8;
	/* other bits unknown */
}



SCREEN_UPDATE_IND16( finalizr )
{
	finalizr_state *state = screen.machine().driver_data<finalizr_state>();
	int offs;

	state->m_bg_tilemap->mark_all_dirty();
	state->m_fg_tilemap->mark_all_dirty();

	state->m_bg_tilemap->set_scrollx(0, *state->m_scroll - 32);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* Draw the sprites. */
	{
		const gfx_element *gfx1 = screen.machine().gfx[1];
		const gfx_element *gfx2 = screen.machine().gfx[2];

		UINT8 *sr = state->m_spriterambank ? state->m_spriteram_2 : state->m_spriteram;


		for (offs = 0; offs <= state->m_spriteram_size - 5; offs += 5)
		{
			int sx, sy, flipx, flipy, code, color, size;


			sx = 32 + 1 + sr[offs + 3] - ((sr[offs + 4] & 0x01) << 8);
			sy = sr[offs + 2];
			flipx = sr[offs + 4] & 0x20;
			flipy = sr[offs + 4] & 0x40;
			code = sr[offs] + ((sr[offs + 1] & 0x0f) << 8);
			color = ((sr[offs + 1] & 0xf0)>>4);

//          (sr[offs + 4] & 0x02) is used, meaning unknown

			size = sr[offs + 4] & 0x1c;

			if (size >= 0x10)	/* 32x32 */
			{
				if (flip_screen_get(screen.machine()))
				{
					sx = 256 - sx;
					sy = 224 - sy;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx_transpen(bitmap,cliprect,gfx1,
						code,
						color,
						flipx,flipy,
						flipx?sx+16:sx,flipy?sy+16:sy,0);
				drawgfx_transpen(bitmap,cliprect,gfx1,
						code + 1,
						color,
						flipx,flipy,
						flipx?sx:sx+16,flipy?sy+16:sy,0);
				drawgfx_transpen(bitmap,cliprect,gfx1,
						code + 2,
						color,
						flipx,flipy,
						flipx?sx+16:sx,flipy?sy:sy+16,0);
				drawgfx_transpen(bitmap,cliprect,gfx1,
						code + 3,
						color,
						flipx,flipy,
						flipx?sx:sx+16,flipy?sy:sy+16,0);
			}
			else
			{
				if (flip_screen_get(screen.machine()))
				{
					sx = ((size & 0x08) ? 280:272) - sx;
					sy = ((size & 0x04) ? 248:240) - sy;
					flipx = !flipx;
					flipy = !flipy;
				}

				if (size == 0x00)	/* 16x16 */
				{
					drawgfx_transpen(bitmap,cliprect,gfx1,
							code,
							color,
							flipx,flipy,
							sx,sy,0);
				}
				else
				{
					code = ((code & 0x3ff) << 2) | ((code & 0xc00) >> 10);

					if (size == 0x04)	/* 16x8 */
					{
						drawgfx_transpen(bitmap,cliprect,gfx2,
								code & ~1,
								color,
								flipx,flipy,
								flipx?sx+8:sx,sy,0);
						drawgfx_transpen(bitmap,cliprect,gfx2,
								code | 1,
								color,
								flipx,flipy,
								flipx?sx:sx+8,sy,0);
					}
					else if (size == 0x08)	/* 8x16 */
					{
						drawgfx_transpen(bitmap,cliprect,gfx2,
								code & ~2,
								color,
								flipx,flipy,
								sx,flipy?sy+8:sy,0);
						drawgfx_transpen(bitmap,cliprect,gfx2,
								code | 2,
								color,
								flipx,flipy,
								sx,flipy?sy:sy+8,0);
					}
					else if (size == 0x0c)	/* 8x8 */
					{
						drawgfx_transpen(bitmap,cliprect,gfx2,
								code,
								color,
								flipx,flipy,
								sx,sy,0);
					}
				}
			}
		}
	}

	{
		const rectangle &visarea = screen.visible_area();
		rectangle clip = cliprect;

		/* draw top status region */
		clip.min_x = visarea.min_x;
		clip.max_x = visarea.min_x + 31;
		state->m_fg_tilemap->set_scrolldx(0,-32);
		state->m_fg_tilemap->draw(bitmap, clip, 0, 0);
	}
	return 0;
}
