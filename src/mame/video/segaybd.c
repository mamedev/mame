/***************************************************************************

    Sega Y-board hardware

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "includes/segaybd.h"


//**************************************************************************
//  VIDEO STARTUP
//**************************************************************************

void segaybd_state::video_start()
{
	// initialize the rotation layer
	segaic16_rotate_init(machine(), 0, SEGAIC16_ROTATE_YBOARD, 0x000);
}



//**************************************************************************
//  VIDEO UPDATE
//**************************************************************************

UINT32 segaybd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// if no drawing is happening, fill with black and get out
	if (!segaic16_display_enable)
	{
		bitmap.fill(get_black_pen(machine()), cliprect);
		return 0;
	}

	// start the sprites drawing
	rectangle yboard_clip(0, 511, 0, 511);
	m_ysprites->bitmap().fill(0xffff);
	m_ysprites->draw_async(yboard_clip);
	m_bsprites->draw_async(cliprect);

	// apply rotation
	segaic16_rotate_draw(machine(), 0, bitmap, cliprect, m_ysprites->bitmap());

	// mix in 16B sprites
	bitmap_ind16 &sprites = m_bsprites->bitmap();
	for (const sparse_dirty_rect *rect = m_bsprites->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *dest = &bitmap.pix(y);
			UINT16 *src = &sprites.pix(y);
			UINT8 *pri = &machine().priority_bitmap.pix(y);
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
							dest[x] = 2048 + (pix & 0x3ff);
					}
				}
			}
		}

	return 0;
}
