// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#include "emu.h"
#include "taito_b.h"

void hitice_state::pixelram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const sy = offset >> 9;
	int const sx = offset & 0x1ff;

	COMBINE_DATA(&m_pixelram[offset]);

	if (ACCESSING_BITS_0_7)
	{
		/* bit 15 of pixel_scroll[0] is probably flip screen */
		m_pixel_bitmap.pix(sy, 2 * sx + 0) = m_b_fg_color_base * 16 + (data & 0xff);
		m_pixel_bitmap.pix(sy, 2 * sx + 1) = m_b_fg_color_base * 16 + (data & 0xff);
	}
}

void hitice_state::pixel_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pixel_scroll[offset]);
}

void hitice_state::clear_pixel_bitmap()
{
	for (int i = 0; i < 0x40000; i++)
		pixelram_w(i, 0);
}

void taitob_c_state::video_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_ctrl);
}

void taitob_state::video_start()
{
}

void hitice_state::video_start()
{
	taitob_state::video_start();

	m_b_fg_color_base = 0x80;       /* hitice also uses this for the pixel_bitmap */

	m_pixel_bitmap.allocate(1024, 512);

	save_item(NAME(m_pixel_bitmap));
	save_item(NAME(m_pixel_scroll));
}

void hitice_state::video_reset()
{
	/* kludge: clear the bitmap on startup */
	clear_pixel_bitmap();
}


void taitob_c_state::video_start()
{
	m_screen->register_screen_bitmap(m_realpunc_bitmap);

	taitob_state::video_start();

	save_item(NAME(m_realpunc_bitmap));
	save_item(NAME(m_video_ctrl));
}


uint32_t taitob_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const video_control = m_tc0180vcu->get_videoctrl();

	if (BIT(~video_control, 5))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* Draw playfields */
	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 0, 1);
	m_tc0180vcu->draw_framebuffer(bitmap, cliprect, 1);
	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 1, 0);
	m_tc0180vcu->draw_framebuffer(bitmap, cliprect, 0);
	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 2, 0);

	return 0;
}


uint32_t hitice_state::screen_update_hitice(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const video_control = m_tc0180vcu->get_videoctrl();

	if (BIT(~video_control, 5))
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
	if ((m_pixel_scroll[0] & 0x5800) == 0x5000)  /* hitice only */
	{
		int scrollx = -2 * m_pixel_scroll[1]; //+320;
		int scrolly = 16 - m_pixel_scroll[2]; //+240;
		/* bit 15 of pixel_scroll[0] is probably flip screen */

		copyscrollbitmap_trans(bitmap, m_pixel_bitmap, 1, &scrollx, 1, &scrolly, cliprect, m_b_fg_color_base * 16);
	}

	m_tc0180vcu->draw_framebuffer(bitmap, cliprect, 0);

	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 2, 0);

	return 0;
}



uint32_t taitob_c_state::screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const palette = m_palette->pens();
	uint8_t const video_control = m_tc0180vcu->get_videoctrl();

	/* Video blanked? */
	if (BIT(~video_control, 5))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* Draw the palettized playfields to an indexed bitmap */
	m_tc0180vcu->tilemap_draw(screen, m_realpunc_bitmap, cliprect, 0, 1);

	m_tc0180vcu->draw_framebuffer(m_realpunc_bitmap, cliprect, 1);

	m_tc0180vcu->tilemap_draw(screen, m_realpunc_bitmap, cliprect, 1, 0);

	if (BIT(m_video_ctrl, 0))
		m_tc0180vcu->draw_framebuffer(m_realpunc_bitmap, cliprect, 0);

	/* Copy the intermediate bitmap to the output bitmap, applying the palette */
	for (int y = 0; y <= cliprect.max_y; y++)
		for (int x = 0; x <= cliprect.max_x; x++)
			bitmap.pix(y, x) = palette[m_realpunc_bitmap.pix(y, x)];

	/* Draw the 15bpp raw CRTC frame buffer directly to the output bitmap */
	if (BIT(m_video_ctrl, 1))
	{
//      scrollx = taitob_scroll[0];
//      scrolly = taitob_scroll[1];

		m_hd63484->update_screen(screen, m_realpunc_bitmap, cliprect);

		for (int y = 0; y <= cliprect.max_y; y++)
		{
			for (int x = 0; x <= cliprect.max_x; x++)
			{
				uint16_t const srcpix = m_realpunc_bitmap.pix(cliprect.min_y + y, cliprect.min_x + x);

				int const r = (BIT(srcpix, 1)) | ((srcpix >> 11) & 0x1e);
				int const g = (BIT(srcpix, 2)) | ((srcpix >> 7) & 0x1e);
				int const b = (BIT(srcpix, 3)) | ((srcpix >> 3) & 0x1e);

				if (srcpix)
					bitmap.pix(y, x) = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
			}
		}
	}
	/* Draw the 15bpp raw output of the camera ADCs (TODO) */
	else if (BIT(m_video_ctrl, 2))
	{
		for (int y = 0; y <= cliprect.max_y; y++)
		{
			for (int x = 0; x <= cliprect.max_x; x++)
				bitmap.pix(y, x) = rgb_t(0x00, 0x00, 0x00);
		}
	}

	/* Clear the indexed bitmap and draw the final indexed layers */
	m_realpunc_bitmap.fill(0, cliprect);

	if (BIT(~m_video_ctrl, 0))
		m_tc0180vcu->draw_framebuffer(m_realpunc_bitmap, cliprect, 0);

	m_tc0180vcu->tilemap_draw(screen, m_realpunc_bitmap, cliprect, 2, 0);

	/* Merge the indexed layers with the output bitmap */
	for (int y = 0; y <= cliprect.max_y; y++)
	{
		for (int x = 0; x <= cliprect.max_x; x++)
		{
			if (m_realpunc_bitmap.pix(y, x))
				bitmap.pix(y, x) = palette[m_realpunc_bitmap.pix(y, x)];
		}
	}

	return 0;
}

