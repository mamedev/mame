// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega pre-System 16 & System 16A hardware

***************************************************************************/

#include "emu.h"
#include "includes/segas16a.h"



//-------------------------------------------------
//  video_start - initialize the video system
//-------------------------------------------------

void segas16a_state::video_start()
{
	// initialize the tile/text layers
	m_segaic16vid->tilemap_init( 0, SEGAIC16_TILEMAP_16A, 0x000, 0, 1);
}


//-------------------------------------------------
//  screen_update - render all graphics
//-------------------------------------------------

UINT32 segas16a_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

	// draw background opaquely first, not setting any priorities
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);

	// draw background again, just to set the priorities on non-transparent pixels
	bitmap_ind16 dummy_bitmap;
	m_segaic16vid->tilemap_draw( screen, dummy_bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	m_segaic16vid->tilemap_draw( screen, dummy_bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	// draw foreground
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	// text layer
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	for (const sparse_dirty_rect *rect = m_sprites->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *dest = &bitmap.pix(y);
			UINT16 *src = &sprites.pix(y);
			UINT8 *pri = &screen.priority().pix(y);
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
							dest[x] += (m_paletteram[dest[x]] & 0x8000) ? m_palette_entries*2 : m_palette_entries;

						// otherwise, just add in sprite palette base
						else
							dest[x] = 0x400 | (pix & 0x3ff);
					}
				}
			}
		}

	return 0;
}
