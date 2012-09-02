/***************************************************************************

    Sega System 16B hardware

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
#include "includes/segas16b.h"



//-------------------------------------------------
//  video_start - initialize the video system
//-------------------------------------------------

void segas16b_state::video_start()
{
	// initialize the tile/text layers
	segaic16_tilemap_init(machine(), 0, m_tilemap_type, 0x000, 0, 2);
}


//-------------------------------------------------
//  screen_update - render all graphics
//-------------------------------------------------

UINT32 segas16b_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// if no drawing is happening, fill with black and get out
	if (!segaic16_display_enable)
	{
		bitmap.fill(get_black_pen(machine()), cliprect);
		return 0;
	}

	// start the sprites drawing
	m_sprites->draw_async(cliprect);

	// reset priorities
	machine().priority_bitmap.fill(0, cliprect);

	// draw background opaquely first, not setting any priorities
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);

	// draw background again, just to set the priorities on non-transparent pixels
	bitmap_ind16 dummy_bitmap;
	segaic16_tilemap_draw(screen, dummy_bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, dummy_bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	// draw foreground
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	// text layer
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	for (const sparse_dirty_rect *rect = m_sprites->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
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
					int priority = (pix >> 10) & 3;
					if ((1 << priority) > pri[x])
					{
						// if the color is set to maximum, shadow pixels underneath us
						if ((pix & 0x03f0) == 0x03f0)
							dest[x] += (m_paletteram[dest[x]] & 0x8000) ? m_palette_entries*2 : m_palette_entries;

						// otherwise, just add in sprite palette base
						else
							dest[x] = 1024 + (pix & 0x3ff);
					}
				}
			}
		}

	return 0;
}
