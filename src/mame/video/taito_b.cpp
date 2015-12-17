// license:???
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

WRITE16_MEMBER(taitob_state::realpunc_video_ctrl_w)
{
	COMBINE_DATA(&m_realpunc_video_ctrl);
}

VIDEO_START_MEMBER(taitob_state,taitob_core)
{
	m_framebuffer[0] = std::make_unique<bitmap_ind16>(512, 256);
	m_framebuffer[1] = std::make_unique<bitmap_ind16>(512, 256);
	m_pixel_bitmap = nullptr;  /* only hitice needs this */

	save_item(NAME(m_pixel_scroll));

	save_item(NAME(*m_framebuffer[0]));
	save_item(NAME(*m_framebuffer[1]));
}

VIDEO_START_MEMBER(taitob_state,taitob_color_order0)
{
	/*graphics are shared, only that they use different palette*/
	/*this is the basic layout used in: Nastar, Ashura Blaster, Hit the Ice, Rambo3, Tetris*/

	/*Note that in both this and color order 1 pixel_color_base/color_granularity is equal to sprites color base. Pure coincidence? */

	m_b_sp_color_base = 0x40 * 16;  /*sprites   */

	/* bg, fg, tx color_base are set in the tc0180vcu interface */

	VIDEO_START_CALL_MEMBER(taitob_core);
}

VIDEO_START_MEMBER(taitob_state,taitob_color_order1)
{
	/* this is the reversed layout used in: Crime City, Puzzle Bobble */
	m_b_sp_color_base = 0x80 * 16;

	VIDEO_START_CALL_MEMBER(taitob_core);
}

VIDEO_START_MEMBER(taitob_state,taitob_color_order2)
{
	/*this is used in: rambo3a, masterw, silentd, selfeena, ryujin */
	m_b_sp_color_base = 0x10 * 16;

	VIDEO_START_CALL_MEMBER(taitob_core);
}


VIDEO_START_MEMBER(taitob_state,hitice)
{
	VIDEO_START_CALL_MEMBER(taitob_color_order0);

	m_b_fg_color_base = 0x80;       /* hitice also uses this for the pixel_bitmap */

	m_pixel_bitmap = std::make_unique<bitmap_ind16>(1024, 512);

	save_item(NAME(*m_pixel_bitmap));
}

VIDEO_RESET_MEMBER(taitob_state,hitice)
{
	/* kludge: clear the bitmap on startup */
	hitice_clear_pixel_bitmap();
}


VIDEO_START_MEMBER(taitob_state,realpunc)
{
	m_realpunc_bitmap = std::make_unique<bitmap_ind16>(m_screen->width(), m_screen->height());

	VIDEO_START_CALL_MEMBER(taitob_color_order0);
}


READ16_MEMBER(taitob_state::tc0180vcu_framebuffer_word_r)
{
	int sy = offset >> 8;
	int sx = 2 * (offset & 0xff);

	return (m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 0) << 8) | m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 1);
}

WRITE16_MEMBER(taitob_state::tc0180vcu_framebuffer_word_w)
{
	int sy = offset >> 8;
	int sx = 2 * (offset & 0xff);

	if (ACCESSING_BITS_8_15)
		m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 0) = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_framebuffer[sy >> 8]->pix16(sy & 0xff, sx + 1) = data & 0xff;
}


void taitob_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
/*  Sprite format: (16 bytes per sprite)
  offs:             bits:
  0000: 0xxxxxxxxxxxxxxx: tile code - 0x0000 to 0x7fff in qzshowby
  0002: 0000000000xxxxxx: color (0x00 - 0x3f)
        x000000000000000: flipy
        0x00000000000000: flipx
        00????????000000: unused ?
  0004: xxxxxx0000000000: doesn't matter - some games (eg nastar) fill this with sign bit, some (eg ashura) do not
        000000xxxxxxxxxx: x-coordinate 10 bits signed (all zero for sprites forming up a big sprite, except for the first one)
  0006: xxxxxx0000000000: doesn't matter - same as x
        000000xxxxxxxxxx: y-coordinate 10 bits signed (same as x)
  0008: xxxxxxxx00000000: sprite x-zoom level
        00000000xxxxxxxx: sprite y-zoom level
      0x00 - non scaled = 100%
      0x80 - scaled to 50%
      0xc0 - scaled to 25%
      0xe0 - scaled to 12.5%
      0xff - scaled to zero pixels size (off)
      Sprite zoom is used in Ashura Blaster just in the beginning
      where you can see a big choplifter and a japanese title.
      This japanese title is a scaled sprite.
      It is used in Crime City also at the end of the third level (in the garage)
      where there are four columns on the sides of the screen
      Heaviest usage is in Rambo 3 - almost every sprite in game is scaled
  000a: xxxxxxxx00000000: x-sprites number (big sprite) decremented by one
        00000000xxxxxxxx: y-sprites number (big sprite) decremented by one
  000c - 000f: unused
*/

	int x, y, xlatch = 0, ylatch = 0, x_no = 0, y_no = 0, x_num = 0, y_num = 0, big_sprite = 0;
	int offs, code, color, flipx, flipy;
	UINT32 data, zoomx, zoomy, zx, zy, zoomxlatch = 0, zoomylatch = 0;

	for (offs = (0x1980 - 16) / 2; offs >=0; offs -= 8)
	{
		code = m_spriteram[offs];

		color = m_spriteram[offs + 1];
		flipx = color & 0x4000;
		flipy = color & 0x8000;
#if 0
		/*check the unknown bits*/
		if (color & 0x3fc0)
		{
			logerror("sprite color (taitob)=%4x ofs=%4x\n", color, offs);
			color = rand() & 0x3f;
		}
#endif
		color = (color & 0x3f) * 16;

		x = m_spriteram[offs + 2] & 0x3ff;
		y = m_spriteram[offs + 3] & 0x3ff;
		if (x >= 0x200)  x -= 0x400;
		if (y >= 0x200)  y -= 0x400;

		data = m_spriteram[offs + 5];
		if (data)
		{
			if (!big_sprite)
			{
				x_num = (data >> 8) & 0xff;
				y_num = (data >> 0) & 0xff;
				x_no  = 0;
				y_no  = 0;
				xlatch = x;
				ylatch = y;
				data = m_spriteram[offs + 4];
				zoomxlatch = (data >> 8) & 0xff;
				zoomylatch = (data >> 0) & 0xff;
				big_sprite = 1;
			}
		}

		data = m_spriteram[offs + 4];
		zoomx = (data >> 8) & 0xff;
		zoomy = (data >> 0) & 0xff;
		zx = (0x100 - zoomx) / 16;
		zy = (0x100 - zoomy) / 16;

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;

			/* Note: like taito_f2.c, this zoom implementation is wrong,
			chopped up into 16x16 sections instead of one sprite. This
			is especially visible in rambo3. */

			x = xlatch + (x_no * (0xff - zoomx) + 15) / 16;
			y = ylatch + (y_no * (0xff - zoomy) + 15) / 16;
			zx = xlatch + ((x_no + 1) * (0xff - zoomx) + 15) / 16 - x;
			zy = ylatch + ((y_no + 1) * (0xff - zoomy) + 15) / 16 - y;
			y_no++;

			if (y_no > y_num)
			{
				y_no = 0;
				x_no++;

				if (x_no > x_num)
					big_sprite = 0;
			}
		}

		if ( zoomx || zoomy )
		{
			m_gfxdecode->gfx(1)->zoom_transpen_raw(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				x,y,
				(zx << 16) / 16,(zy << 16) / 16,0);
		}
		else
		{
			m_gfxdecode->gfx(1)->transpen_raw(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				x,y,
				0);
		}
	}
}


void taitob_state::draw_framebuffer( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	rectangle myclip = cliprect;
	int x, y;
	address_space &space = machine().driver_data()->generic_space();
	UINT8 video_control = m_tc0180vcu->get_videoctrl(space, 0);
	UINT8 framebuffer_page = m_tc0180vcu->get_fb_page(space, 0);

g_profiler.start(PROFILER_USER1);

	priority <<= 4;

	if (video_control & 0x08)
	{
		if (priority)
		{
			g_profiler.stop();
			return;
		}

		if (video_control & 0x10)   /*flip screen*/
		{
			/*popmessage("1. X[%3i;%3i] Y[%3i;%3i]", myclip.min_x, myclip.max_x, myclip.min_y, myclip.max_y);*/
			for (y = myclip.min_y; y <= myclip.max_y; y++)
			{
				UINT16 *src = &m_framebuffer[framebuffer_page]->pix16(y, myclip.min_x);
				UINT16 *dst;

				dst = &bitmap.pix16(bitmap.height()-1-y, myclip.max_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					UINT16 c = *src++;

					if (c != 0)
						*dst = m_b_sp_color_base + c;

					dst--;
				}
			}
		}
		else
		{
			for (y = myclip.min_y; y <= myclip.max_y; y++)
			{
				UINT16 *src = &m_framebuffer[framebuffer_page]->pix16(y, myclip.min_x);
				UINT16 *dst = &bitmap.pix16(y, myclip.min_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					UINT16 c = *src++;

					if (c != 0)
						*dst = m_b_sp_color_base + c;

					dst++;
				}
			}
		}
	}
	else
	{
		if (video_control & 0x10)   /*flip screen*/
		{
			/*popmessage("3. X[%3i;%3i] Y[%3i;%3i]", myclip.min_x, myclip.max_x, myclip.min_y, myclip.max_y);*/
			for (y = myclip.min_y ;y <= myclip.max_y; y++)
			{
				UINT16 *src = &m_framebuffer[framebuffer_page]->pix16(y, myclip.min_x);
				UINT16 *dst;

				dst = &bitmap.pix16(bitmap.height()-1-y, myclip.max_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					UINT16 c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = m_b_sp_color_base + c;

					dst--;
				}
			}
		}
		else
		{
			for (y = myclip.min_y; y <= myclip.max_y; y++)
			{
				UINT16 *src = &m_framebuffer[framebuffer_page]->pix16(y, myclip.min_x);
				UINT16 *dst = &bitmap.pix16(y, myclip.min_x);

				for (x = myclip.min_x; x <= myclip.max_x; x++)
				{
					UINT16 c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = m_b_sp_color_base + c;

					dst++;
				}
			}
		}
	}
g_profiler.stop();
}

UINT32 taitob_state::screen_update_taitob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT8 video_control = m_tc0180vcu->get_videoctrl(space, 0);

	if ((video_control & 0x20) == 0)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* Draw playfields */
	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 0, 1);

	draw_framebuffer(bitmap, cliprect, 1);

	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 1, 0);

	if (m_pixel_bitmap)  /* hitice only */
	{
		int scrollx = -2 * m_pixel_scroll[0]; //+320;
		int scrolly = - m_pixel_scroll[1]; //+240;
		/* bit 15 of pixel_scroll[0] is probably flip screen */

		copyscrollbitmap_trans(bitmap, *m_pixel_bitmap, 1, &scrollx, 1, &scrolly, cliprect, m_b_fg_color_base * 16);
	}

	draw_framebuffer(bitmap, cliprect, 0);

	m_tc0180vcu->tilemap_draw(screen, bitmap, cliprect, 2, 0);

	return 0;
}



UINT32 taitob_state::screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	const pen_t *palette = m_palette->pens();
	UINT8 video_control = m_tc0180vcu->get_videoctrl(space, 0);
	int x, y;

	/* Video blanked? */
	if (!(video_control & 0x20))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	/* Draw the palettized playfields to an indexed bitmap */
	m_tc0180vcu->tilemap_draw(screen, *m_realpunc_bitmap, cliprect, 0, 1);

	draw_framebuffer(*m_realpunc_bitmap, cliprect, 1);

	m_tc0180vcu->tilemap_draw(screen, *m_realpunc_bitmap, cliprect, 1, 0);

	if (m_realpunc_video_ctrl & 0x0001)
		draw_framebuffer(*m_realpunc_bitmap, cliprect, 0);

	/* Copy the intermediate bitmap to the output bitmap, applying the palette */
	for (y = 0; y <= cliprect.max_y; y++)
		for (x = 0; x <= cliprect.max_x; x++)
			bitmap.pix32(y, x) = palette[m_realpunc_bitmap->pix16(y, x)];

	/* Draw the 15bpp raw CRTC frame buffer directly to the output bitmap */
	if (m_realpunc_video_ctrl & 0x0002)
	{
		int base = (m_hd63484->regs_r(space, 0xcc/2, 0xffff) << 16) + m_hd63484->regs_r(space, 0xce/2, 0xffff);
		int stride = m_hd63484->regs_r(space, 0xca/2, 0xffff);

//      scrollx = taitob_scroll[0];
//      scrolly = taitob_scroll[1];

		for (y = 0; y <= cliprect.max_y; y++)
		{
			int addr = base + (y*stride);
			for (x = 0; x <= cliprect.max_x; x++)
			{
				int r, g, b;
				UINT16 srcpix = m_hd63484->ram_r(space, addr++, 0xffff);

				r = (BIT(srcpix, 1)) | ((srcpix >> 11) & 0x1e);
				g = (BIT(srcpix, 2)) | ((srcpix >> 7) & 0x1e);
				b = (BIT(srcpix, 3)) | ((srcpix >> 3) & 0x1e);

				if (srcpix)
					bitmap.pix32(y, x) = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
			}

			addr += stride;
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
		draw_framebuffer(*m_realpunc_bitmap, cliprect, 0);

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



void taitob_state::screen_eof_taitob(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		address_space &space = machine().driver_data()->generic_space();
		UINT8 video_control = m_tc0180vcu->get_videoctrl(space, 0);
		UINT8 framebuffer_page = m_tc0180vcu->get_fb_page(space, 0);

		if (~video_control & 0x01)
			m_framebuffer[framebuffer_page]->fill(0, screen.visible_area());

		if (~video_control & 0x80)
		{
			framebuffer_page ^= 1;
			m_tc0180vcu->set_fb_page(space, 0, framebuffer_page);
		}

		draw_sprites(*m_framebuffer[framebuffer_page], screen.visible_area());
	}
}
