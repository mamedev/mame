// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Hang On hardware

***************************************************************************/

#include "emu.h"
#include "includes/segahang.h"



//-------------------------------------------------
//  video_start - initialize the video system
//-------------------------------------------------

void segahang_state::video_start()
{
	// initialize the tile/text layers
	m_segaic16vid->tilemap_init( 0, SEGAIC16_TILEMAP_HANGON, 0x000, 0, 2);

	// initialize the road
	m_segaic16road->segaic16_road_init(machine(), 0, m_sharrier_video ? SEGAIC16_ROAD_SHARRIER : SEGAIC16_ROAD_HANGON, 0x038, 0x7c0, 0x7c0, 0);
}


//-------------------------------------------------
//  screen_update - render all graphics
//-------------------------------------------------

UINT32 segahang_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// if no drawing is happening, fill with black and get out
	if (!m_segaic16vid->m_display_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	// start the sprites drawing
	m_sprites->draw_async(cliprect);

	// reset priorities
	screen.priority().fill(0, cliprect);

	// draw the low priority road layer
	m_segaic16road->segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_BACKGROUND);

	// draw background
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	// draw foreground
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	// draw the high priority road
	m_segaic16road->segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_FOREGROUND);

	// text layer
	// note that we inflate the priority of the text layer to prevent sprites
	// from drawing over the high scores
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x08);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	for (const sparse_dirty_rect *rect = m_sprites->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *dest = &bitmap.pix(y);
			UINT16 *src = &sprites.pix(y);
			UINT8 *pri = &screen.priority().pix(y);

			// hangon mixing
			if (!m_sharrier_video)
			{
				for (int x = rect->min_x; x <= rect->max_x; x++)
				{
					// only process written pixels
					UINT16 pix = src[x];
					if (pix != 0xffff)
					{
						// compare sprite priority against tilemap priority
						int priority = pix >> 10;
						if ((1 << priority) > pri[x])
						{
							// if color bits are all 1, this triggers shadow/hilight
							if ((pix & 0x3f0) == 0x3f0)
								dest[x] += m_shadow ? m_palette_entries*2 : m_palette_entries;

							// otherwise, just add in sprite palette base
							else
								dest[x] = 0x400 | (pix & 0x3ff);
						}
					}
				}
			}

			// sharrier mixing
			else
			{
				for (int x = rect->min_x; x <= rect->max_x; x++)
				{
					// only process written pixels
					UINT16 pix = src[x];
					if (pix != 0xffff)
					{
						// compare sprite priority against tilemap priority
						int priority = ((pix >> 9) & 2) | 1;
						if ((1 << priority) > pri[x])
						{
							// if shadow bit is 0 and pix data is 0xa, this triggers shadow/hilight
							if ((pix & 0x80f) == 0x00a)
								dest[x] += (m_paletteram[dest[x]] & 0x8000) ? m_palette_entries*2 : m_palette_entries;

							// otherwise, just add in sprite palette base
							else
								dest[x] = 0x400 | (pix & 0x3ff);
						}
					}
				}
			}
		}

	return 0;
}
