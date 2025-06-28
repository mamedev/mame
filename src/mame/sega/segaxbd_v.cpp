// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega X-board hardware

***************************************************************************/

#include "emu.h"
#include "segaxbd.h"


//**************************************************************************
//  VIDEO STARTUP
//**************************************************************************

void segaxbd_state::video_start()
{
	if(!m_segaic16vid->started())
		throw device_missing_dependencies();

	if(!m_segaic16road->started())
		throw device_missing_dependencies();

	// initialize the tile/text layers
	m_segaic16vid->tilemap_init( 0, segaic16_video_device::TILEMAP_16B, 0x1c00, 0, 2);

	// initialize the road
	m_segaic16road->segaic16_road_init(0, segaic16_road_device::ROAD_XBOARD, 0x1700, 0x1720, 0x1780, -166);
}



//**************************************************************************
//  VIDEO UPDATE
//**************************************************************************

uint32_t segaxbd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
	m_segaic16road->segaic16_road_draw(0, bitmap, cliprect, segaic16_road_device::ROAD_BACKGROUND);
	if (m_road_priority == 0)
		m_segaic16road->segaic16_road_draw(0, bitmap, cliprect, segaic16_road_device::ROAD_FOREGROUND);

	// draw background
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_BACKGROUND, 0, 0x01);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_BACKGROUND, 1, 0x02);

	// draw foreground
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_FOREGROUND, 0, 0x02);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_FOREGROUND, 1, 0x04);

	// draw the high priority road
	if (m_road_priority == 1)
		m_segaic16road->segaic16_road_draw(0, bitmap, cliprect, segaic16_road_device::ROAD_FOREGROUND);

	// text layer
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_TEXT, 0, 0x04);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_TEXT, 1, 0x08);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	m_sprites->iterate_dirty_rects(
			cliprect,
			[this, &screen, &bitmap, &sprites] (rectangle const &rect)
			{
				for (int y = rect.min_y; y <= rect.max_y; y++)
				{
					uint16_t *const dest = &bitmap.pix(y);
					uint16_t const *const src = &sprites.pix(y);
					uint8_t const *const pri = &screen.priority().pix(y);
					for (int x = rect.min_x; x <= rect.max_x; x++)
					{
						// only process written pixels
						uint16_t const pix = src[x];
						if (pix != 0xffff)
						{
							// compare sprite priority against tilemap priority
							int const priority = (pix >> 12) & 3;
							if ((1 << priority) > pri[x])
							{
								// if the shadow flag is set, this triggers shadow/hilight for pen 0xa
								if ((pix & 0x400f) == 0x400a)
									dest[x] += m_palette_entries;

								// otherwise, just add in sprite palette base
								else
									dest[x] = pix & 0xfff;
							}
						}
					}
				}
			});

	return 0;
}
