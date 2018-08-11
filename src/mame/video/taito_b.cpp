// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#include "emu.h"
#include "includes/taito_b.h"

WRITE16_MEMBER(taitob_state::hitice_pixelram_w)
{
	int sy = offset >> 9;
	int sx = offset & 0x1ff;

	COMBINE_DATA(&m_pixelram[offset]);

	if (ACCESSING_BITS_0_7)
	{
		/* bit 15 of pixel_scroll[0] is probably flip screen */
		m_pixel_bitmap->pix16(sy, 2 * sx + 0) = m_b_fg_color_base * 16 + (data & 0xff);
		m_pixel_bitmap->pix16(sy, 2 * sx + 1) = m_b_fg_color_base * 16 + (data & 0xff);
	}
}

WRITE16_MEMBER(taitob_state::hitice_pixel_scroll_w)
{
	COMBINE_DATA(&m_pixel_scroll[offset]);
}

void taitob_state::hitice_clear_pixel_bitmap(  )
{
	int i;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	for (i = 0; i < 0x40000; i++)
		hitice_pixelram_w(space, i, 0, 0xffff);
}

WRITE16_MEMBER(taitob_c_state::realpunc_video_ctrl_w)
{
	COMBINE_DATA(&m_realpunc_video_ctrl);
}

VIDEO_START_MEMBER(taitob_state,taitob_core)
{
	m_pixel_bitmap = nullptr;  /* only hitice needs this */

	save_item(NAME(m_pixel_scroll));
}

VIDEO_START_MEMBER(taitob_state,hitice)
{
	VIDEO_START_CALL_MEMBER(taitob_core);

	m_b_fg_color_base = 0x80;       /* hitice also uses this for the pixel_bitmap */

	m_pixel_bitmap = std::make_unique<bitmap_ind16>(1024, 512);

	save_item(NAME(*m_pixel_bitmap));
}

VIDEO_RESET_MEMBER(taitob_state,hitice)
{
	/* kludge: clear the bitmap on startup */
	hitice_clear_pixel_bitmap();
}


VIDEO_START_MEMBER(taitob_c_state,realpunc)
{
	m_realpunc_bitmap = std::make_unique<bitmap_ind16>(m_screen->width(), m_screen->height());

	VIDEO_START_CALL_MEMBER(taitob_core);
}


uint32_t taitob_state::screen_update_taitob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const video_control = m_tc0180vcu->get_videoctrl();

	if ((video_control & 0x20) == 0)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* Draw playfields */
	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 0, 1);

	m_tc0180vcu->draw_framebuffer(bitmap, cliprect, 1);

	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 1, 0);

	// TODO: only hiticej properly enables this up during attract mode,
	//       hitice / hiticerb keeps this disabled, maybe a btanb fixed in later revision?
	if (m_pixel_bitmap && (m_pixel_scroll[0] & 0x5800) == 0x5000)  /* hitice only */
	{
		int scrollx = -2 * m_pixel_scroll[1]; //+320;
		int scrolly = 16 - m_pixel_scroll[2]; //+240;
		/* bit 15 of pixel_scroll[0] is probably flip screen */

		copyscrollbitmap_trans(bitmap, *m_pixel_bitmap, 1, &scrollx, 1, &scrolly, cliprect, m_b_fg_color_base * 16);
	}

	m_tc0180vcu->draw_framebuffer(bitmap, cliprect, 0);

	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 2, 0);

	return 0;
}



uint32_t taitob_c_state::screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *palette = m_palette->pens();
	uint8_t const video_control = m_tc0180vcu->get_videoctrl();
	int x, y;

	/* Video blanked? */
	if (!(video_control & 0x20))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* Draw the palettized playfields to an indexed bitmap */
	m_tc0180vcu->tilemap_draw(screen, *m_realpunc_bitmap, cliprect, 0, 1);

	m_tc0180vcu->draw_framebuffer(*m_realpunc_bitmap, cliprect, 1);

	m_tc0180vcu->tilemap_draw(screen, *m_realpunc_bitmap, cliprect, 1, 0);

	if (m_realpunc_video_ctrl & 0x0001)
		m_tc0180vcu->draw_framebuffer(*m_realpunc_bitmap, cliprect, 0);

	/* Copy the intermediate bitmap to the output bitmap, applying the palette */
	for (y = 0; y <= cliprect.max_y; y++)
		for (x = 0; x <= cliprect.max_x; x++)
			bitmap.pix32(y, x) = palette[m_realpunc_bitmap->pix16(y, x)];

	/* Draw the 15bpp raw CRTC frame buffer directly to the output bitmap */
	if (m_realpunc_video_ctrl & 0x0002)
	{
//      scrollx = taitob_scroll[0];
//      scrolly = taitob_scroll[1];

		m_hd63484->update_screen(screen, *m_realpunc_bitmap, cliprect);

		for (y = 0; y <= cliprect.max_y; y++)
		{
			for (x = 0; x <= cliprect.max_x; x++)
			{
				int r, g, b;
				uint16_t srcpix = m_realpunc_bitmap->pix16(cliprect.min_y + y, cliprect.min_x + x);

				r = (BIT(srcpix, 1)) | ((srcpix >> 11) & 0x1e);
				g = (BIT(srcpix, 2)) | ((srcpix >> 7) & 0x1e);
				b = (BIT(srcpix, 3)) | ((srcpix >> 3) & 0x1e);

				if (srcpix)
					bitmap.pix32(y, x) = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
			}
		}
	}
	/* Draw the 15bpp raw output of the camera ADCs (TODO) */
	else if (m_realpunc_video_ctrl & 0x0004)
	{
		for (y = 0; y <= cliprect.max_y; y++)
		{
			for (x = 0; x <= cliprect.max_x; x++)
				bitmap.pix32(y, x) = rgb_t(0x00, 0x00, 0x00);
		}
	}

	/* Clear the indexed bitmap and draw the final indexed layers */
	m_realpunc_bitmap->fill(0, cliprect);

	if (!(m_realpunc_video_ctrl & 0x0001))
		m_tc0180vcu->draw_framebuffer(*m_realpunc_bitmap, cliprect, 0);

	m_tc0180vcu->tilemap_draw(screen, *m_realpunc_bitmap, cliprect, 2, 0);

	/* Merge the indexed layers with the output bitmap */
	for (y = 0; y <= cliprect.max_y; y++)
	{
		for (x = 0; x <= cliprect.max_x; x++)
		{
			if (m_realpunc_bitmap->pix16(y, x))
				bitmap.pix32(y, x) = palette[m_realpunc_bitmap->pix16(y, x)];
		}
	}

	return 0;
}

