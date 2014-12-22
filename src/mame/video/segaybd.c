// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Y-board hardware

***************************************************************************/

#include "emu.h"
#include "includes/segaybd.h"


//**************************************************************************
//  VIDEO STARTUP
//**************************************************************************

void segaybd_state::video_start()
{
	// initialize the rotation layer
	m_segaic16vid->rotate_init(0, SEGAIC16_ROTATE_YBOARD, 0x000);
	m_ysprites->set_rotate_ptr(m_segaic16vid->m_rotate);
}



//**************************************************************************
//  VIDEO UPDATE
//**************************************************************************

UINT32 segaybd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// if no drawing is happening, fill with black and get out
	if (!m_segaic16vid->m_display_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	// start the sprites drawing
	rectangle yboard_clip(0, 511, 0, 511);
	m_ysprites->bitmap().fill(0xffff);
	m_ysprites->draw_async(yboard_clip);
	m_bsprites->draw_async(cliprect);

	// apply rotation
	m_segaic16vid->rotate_draw(0, bitmap, cliprect, screen.priority(), m_ysprites->bitmap());

	// mix in 16B sprites
	bitmap_ind16 &sprites = m_bsprites->bitmap();
	for (const sparse_dirty_rect *rect = m_bsprites->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
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
					int priority = (pix >> 11) & 0x1e;
					if (priority < pri[x])
					{
						// if the color is set to maximum, shadow pixels underneath us
						if ((pix & 0xf) == 0xe)
							dest[x] += (m_paletteram[dest[x]] & 0x8000) ? m_palette_entries*2 : m_palette_entries;

						// otherwise, just add in sprite palette base
						else
							dest[x] = 0x800 | (pix & 0x7ff);
					}
				}
			}
		}

	return 0;
}
