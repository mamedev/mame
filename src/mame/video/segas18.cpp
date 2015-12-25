// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 18 hardware

***************************************************************************/

#include "emu.h"
#include "includes/segas18.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DEBUG_VDP               (0)


/*************************************
 *
 *  Video startup
 *
 *************************************/

void segas18_state::video_start()
{
	m_temp_bitmap.allocate(m_screen->width(), m_screen->height());
	m_grayscale_enable = 0;
	m_vdp_enable = 0;
	m_vdp_mixing = 0;

	// initialize the tile/text layers
	m_segaic16vid->tilemap_init( 0, SEGAIC16_TILEMAP_16B, 0x000, 0, 8);

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

WRITE_LINE_MEMBER(segas18_state::set_grayscale)
{
	if (state != m_grayscale_enable)
	{
		m_screen->update_partial(m_screen->vpos());
		m_grayscale_enable = state;
//      osd_printf_debug("Grayscale = %02X\n", enable);
	}
}


WRITE_LINE_MEMBER(segas18_state::set_vdp_enable)
{
	if (state != m_vdp_enable)
	{
		m_screen->update_partial(m_screen->vpos());
		m_vdp_enable = state;
#if DEBUG_VDP
		osd_printf_debug("VDP enable = %02X\n", enable);
#endif
	}
}


void segas18_state::set_vdp_mixing(UINT8 mixing)
{
	if (mixing != m_vdp_mixing)
	{
		m_screen->update_partial(m_screen->vpos());
		m_vdp_mixing = mixing;
#if DEBUG_VDP
		osd_printf_debug("VDP mixing = %02X\n", mixing);
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
	bitmap_ind8 &priority_bitmap = screen.priority();
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
	if (m_vdp_enable && vdplayer == 0) draw_vdp(screen, bitmap, cliprect, vdppri);

	// draw background again to draw non-transparent pixels over the VDP and set the priority
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);
	if (m_vdp_enable && vdplayer == 1) draw_vdp(screen, bitmap, cliprect, vdppri);

	// draw foreground
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);
	if (m_vdp_enable && vdplayer == 2) draw_vdp(screen, bitmap, cliprect, vdppri);

	// text layer
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	m_segaic16vid->tilemap_draw( screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);
	if (m_vdp_enable && vdplayer == 3) draw_vdp(screen, bitmap, cliprect, vdppri);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	for (const sparse_dirty_rect *rect = m_sprites->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
	{
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
					int priority = (pix >> 10) & 3;
					if ((1 << priority) > pri[x])
					{
						// if the color is set to maximum, shadow pixels underneath us
						if ((pix & 0x03f0) == 0x03f0)
							dest[x] += (m_paletteram[dest[x]] & 0x8000) ? m_palette_entries*2 : m_palette_entries;

						// otherwise, just add in sprite palette base
						else
							dest[x] = 0x400 | (pix & 0x3ff);
					}
				}
			}
		}
	}

#if DEBUG_VDP
	if (m_vdp_enable && machine().input().code_pressed(KEYCODE_V))
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
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
