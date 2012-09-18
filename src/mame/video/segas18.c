/***************************************************************************

    Sega System 18 hardware

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
#include "includes/genesis.h"
#include "includes/segas18.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DEBUG_VDP				(0)


/*************************************
 *
 *  Video startup
 *
 *************************************/

void segas18_state::video_start()
{
	m_temp_bitmap.allocate(machine().primary_screen->width(), machine().primary_screen->height());
	m_grayscale_enable = false;
	m_vdp_enable = false;
	m_vdp_mixing = 0;

	// initialize the tile/text layers
	segaic16_tilemap_init(machine(), 0, SEGAIC16_TILEMAP_16B, 0x000, 0, 8);

	save_item(NAME(m_grayscale_enable));
	save_item(NAME(m_vdp_enable));
	save_item(NAME(m_vdp_mixing));
	save_item(NAME(m_temp_bitmap));
}



/*************************************
 *
 *  Miscellaneous setters
 *
 *************************************/

void segas18_state::set_grayscale(bool enable)
{
	if (enable != m_grayscale_enable)
	{
		machine().primary_screen->update_partial(machine().primary_screen->vpos());
		m_grayscale_enable = enable;
//      mame_printf_debug("Grayscale = %02X\n", enable);
	}
}


void segas18_state::set_vdp_enable(bool enable)
{
	if (enable != m_vdp_enable)
	{
		machine().primary_screen->update_partial(machine().primary_screen->vpos());
		m_vdp_enable = enable;
#if DEBUG_VDP
		mame_printf_debug("VDP enable = %02X\n", enable);
#endif
	}
}


void segas18_state::set_vdp_mixing(UINT8 mixing)
{
	if (mixing != m_vdp_mixing)
	{
		machine().primary_screen->update_partial(machine().primary_screen->vpos());
		m_vdp_mixing = mixing;
#if DEBUG_VDP
		mame_printf_debug("VDP mixing = %02X\n", mixing);
#endif
	}
}



/*************************************
 *
 *  VDP drawing
 *
 *************************************/

void segas18_state::draw_vdp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{

	bitmap_ind8 &priority_bitmap = machine().priority_bitmap;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
	//  UINT16 *src = vdp->m_render_line; // can't use this because we're not in RGB32, which we'll need to be if there are palette effects
	//  UINT16 *src2 = vdp->m_render_line_raw;

		UINT16 *dst = &bitmap.pix(y);
		UINT8 *pri = &priority_bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (m_vdp->m_render_line_raw[x] & 0x100)
			{
				int pix = m_vdp->m_render_line_raw[x] & 0x3f;
				if (pix & 0xf)
				{
					switch (pix & 0xc0)
					{
						case 0x00:
							dst[x] = pix + 0x2000; /* 0x2040 - would be shadow? */
							break;
						case 0x40:
						case 0x80:
							dst[x] = pix + 0x2000;
							break;
						case 0xc0:
							dst[x] = pix + 0x2000; /* 0x2080 - would be higlight? */
							break;
					}
					pri[x] |= priority;
				}
			}
		}
	}
}

/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 segas18_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
    Current understanding of VDP mixing:

    mixing = 0x00:
        astorm: layer = 0, pri = 0x00 or 0x01
        lghost: layer = 0, pri = 0x00 or 0x01

    mixing = 0x01:
        ddcrew: layer = 0, pri = 0x00 or 0x01 or 0x02

    mixing = 0x02:
        never seen

    mixing = 0x03:
        ddcrew: layer = 1, pri = 0x00 or 0x01 or 0x02

    mixing = 0x04:
        astorm: layer = 2 or 3, pri = 0x00 or 0x01 or 0x02
        mwalk:  layer = 2 or 3, pri = 0x00 or 0x01 or 0x02

    mixing = 0x05:
        ddcrew: layer = 2, pri = 0x04
        wwally: layer = 2, pri = 0x04

    mixing = 0x06:
        never seen

    mixing = 0x07:
        cltchitr: layer = 1 or 2 or 3, pri = 0x02 or 0x04 or 0x08
        mwalk:    layer = 3, pri = 0x04 or 0x08
*/

	int vdplayer = (m_vdp_mixing >> 1) & 3;
	int vdppri = (m_vdp_mixing & 1) ? (1 << vdplayer) : 0;

#if DEBUG_VDP
	if (machine().input().code_pressed(KEYCODE_Q)) vdplayer = 0;
	if (machine().input().code_pressed(KEYCODE_W)) vdplayer = 1;
	if (machine().input().code_pressed(KEYCODE_E)) vdplayer = 2;
	if (machine().input().code_pressed(KEYCODE_R)) vdplayer = 3;
	if (machine().input().code_pressed(KEYCODE_A)) vdppri = 0x00;
	if (machine().input().code_pressed(KEYCODE_S)) vdppri = 0x01;
	if (machine().input().code_pressed(KEYCODE_D)) vdppri = 0x02;
	if (machine().input().code_pressed(KEYCODE_F)) vdppri = 0x04;
	if (machine().input().code_pressed(KEYCODE_G)) vdppri = 0x08;
#endif

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
	if (m_vdp_enable && vdplayer == 0) draw_vdp(screen, bitmap, cliprect, vdppri);

	// draw background again to draw non-transparent pixels over the VDP and set the priority
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);
	if (m_vdp_enable && vdplayer == 1) draw_vdp(screen, bitmap, cliprect, vdppri);

	// draw foreground
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);
	if (m_vdp_enable && vdplayer == 2) draw_vdp(screen, bitmap, cliprect, vdppri);

	// text layer
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);
	if (m_vdp_enable && vdplayer == 3) draw_vdp(screen, bitmap, cliprect, vdppri);

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

#if DEBUG_VDP
	if (m_vdp_enable && machine().input().code_pressed(KEYCODE_V))
	{
		bitmap.fill(get_black_pen(machine()), cliprect);
		update_system18_vdp(bitmap, cliprect);
	}
	if (vdp_enable && machine().input().code_pressed(KEYCODE_B))
	{
		FILE *f = fopen("vdp.bin", "w");
		fwrite(m_temp_bitmap->base(), 1, m_temp_bitmap->rowpixels() * (m_temp_bitmap->bpp() / 8) * m_temp_bitmap->height(), f);
		fclose(f);
	}
#endif
	return 0;
}
