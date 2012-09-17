/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/finalizr.h"


void finalizr_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int r = pal4bit(color_prom[i + 0x00] >> 0);
		int g = pal4bit(color_prom[i + 0x00] >> 4);
		int b = pal4bit(color_prom[i + 0x20] >> 0);

		colortable_palette_set_color(machine().colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(finalizr_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xc0) << 2) + (m_charbank << 10);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(finalizr_state::get_fg_tile_info)
{
	int attr = m_colorram2[tile_index];
	int code = m_videoram2[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void finalizr_state::video_start()
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(finalizr_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(finalizr_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



WRITE8_MEMBER(finalizr_state::finalizr_videoctrl_w)
{
	m_charbank = data & 3;
	m_spriterambank = data & 8;
	/* other bits unknown */
}



UINT32 finalizr_state::screen_update_finalizr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	m_bg_tilemap->mark_all_dirty();
	m_fg_tilemap->mark_all_dirty();

	m_bg_tilemap->set_scrollx(0, *m_scroll - 32);
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* Draw the sprites. */
	{
		gfx_element *gfx1 = screen.machine().gfx[1];
		gfx_element *gfx2 = screen.machine().gfx[2];

		UINT8 *sr = m_spriterambank ? m_spriteram_2 : m_spriteram;


		for (offs = 0; offs <= m_spriteram.bytes() - 5; offs += 5)
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
				if (flip_screen())
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
				if (flip_screen())
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
		m_fg_tilemap->set_scrolldx(0,-32);
		m_fg_tilemap->draw(bitmap, clip, 0, 0);
	}
	return 0;
}
