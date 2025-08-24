// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 16B hardware

***************************************************************************/

#include "emu.h"
#include "segas16b.h"



//-------------------------------------------------
//  video_start - initialize the video system
//-------------------------------------------------

void segas16b_state::video_start()
{
	// initialize the tile/text layers
	m_segaic16vid->tilemap_init( 0, m_tilemap_type, 0x000, 0, 2);
}


//-------------------------------------------------
//  screen_update - render all graphics
//-------------------------------------------------

uint32_t segas16b_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// if no drawing is happening, fill with black and get out
	if (!m_segaic16vid->m_display_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	// start the sprites drawing
	if (m_sprites.found())
		m_sprites->draw_async(cliprect);

	// reset priorities
	screen.priority().fill(0, cliprect);

	// draw background opaquely first, not setting any priorities
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);

	// draw background again, just to set the priorities on non-transparent pixels
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_BACKGROUND, 0, 0x01);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_BACKGROUND, 1, 0x02);

	// draw foreground
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_FOREGROUND, 0, 0x02);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_FOREGROUND, 1, 0x04);

	// text layer
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_TEXT, 0, 0x04);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, segaic16_video_device::TILEMAP_TEXT, 1, 0x08);

	// mix in sprites
	if (!m_sprites)
		return 0;
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
							int const priority = (pix >> 10) & 3;
							if ((1 << priority) > pri[x])
							{
								if ((pix & 0x03f0) == 0x03f0)
								{
									// if the color is set to maximum, shadow pixels underneath us
									dest[x] += m_palette_entries;
								}
								else
								{
									// otherwise, just add in sprite palette base
									dest[x] = m_spritepalbase | (pix & 0x3ff);
								}
							}
						}
					}
				}
			});

	return 0;
}
