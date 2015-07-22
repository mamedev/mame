// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "emu.h"
#include "includes/nbmj8991.h"

/******************************************************************************


******************************************************************************/

WRITE8_MEMBER(nbmj8991_state::palette_type1_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_generic_paletteram_8[offset + 0] & 0x0f) >> 0);
	g = ((m_generic_paletteram_8[offset + 1] & 0xf0) >> 4);
	b = ((m_generic_paletteram_8[offset + 1] & 0x0f) >> 0);

	m_palette->set_pen_color((offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

WRITE8_MEMBER(nbmj8991_state::palette_type2_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_generic_paletteram_8[offset + 0] & 0x7c) >> 2);
	g = (((m_generic_paletteram_8[offset + 0] & 0x03) << 3) | ((m_generic_paletteram_8[offset + 1] & 0xe0) >> 5));
	b = ((m_generic_paletteram_8[offset + 1] & 0x1f) >> 0);

	m_palette->set_pen_color((offset / 2), pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_MEMBER(nbmj8991_state::palette_type3_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_generic_paletteram_8[offset + 1] & 0x0f) >> 0);
	g = ((m_generic_paletteram_8[offset + 0] & 0xf0) >> 4);
	b = ((m_generic_paletteram_8[offset + 0] & 0x0f) >> 0);

	m_palette->set_pen_color((offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(nbmj8991_state::blitter_w)
{
	int gfxlen = memregion("gfx1")->bytes();

	switch (offset)
	{
		case 0x00:  m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 0x01:  m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 0x02:  break;
		case 0x03:  break;
		case 0x04:  m_blitter_sizex = data; break;
		case 0x05:  m_blitter_sizey = data;
					/* writing here also starts the blit */
					gfxdraw();
					break;
		case 0x06:  m_blitter_direction_x = (data & 0x01) ? 1 : 0;
					m_blitter_direction_y = (data & 0x02) ? 1 : 0;
					m_flipscreen = (data & 0x04) ? 0 : 1;
					m_dispflag = (data & 0x10) ? 0 : 1;
					vramflip();
					break;
		case 0x07:  break;
		case 0x10:  m_blitter_destx = (m_blitter_destx & 0xff00) | data; break;
		case 0x20:  m_blitter_desty = (m_blitter_desty & 0xff00) | data; break;
		case 0x30:  m_scrollx = (m_scrollx & 0xff00) | data; break;
		case 0x40:  m_scrolly = (m_scrolly & 0xff00) | data; break;
		case 0x50:  m_blitter_destx = (m_blitter_destx & 0x00ff) | ((data & 0x01) << 8);
					m_blitter_desty = (m_blitter_desty & 0x00ff) | ((data & 0x02) << 7);
					m_scrollx = (m_scrollx & 0x00ff) | ((data & 0x04) << 6);
					m_scrolly = (m_scrolly & 0x00ff) | ((data & 0x08) << 5);
					break;
		case 0x60:  m_gfxrom = data; break;
		case 0x70:  m_clutsel = data; break;
	}

	if ((0x20000 * m_gfxrom) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

READ8_MEMBER(nbmj8991_state::clut_r)
{
	return m_clut[offset];
}

WRITE8_MEMBER(nbmj8991_state::clut_w)
{
	m_clut[((m_clutsel & 0x7f) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
void nbmj8991_state::vramflip()
{
	int x, y;
	UINT8 color1, color2;
	int width = m_screen->width();
	int height = m_screen->height();

	if (m_flipscreen == m_flipscreen_old) return;

	for (y = 0; y < height / 2; y++)
	{
		for (x = 0; x < width / 2; x++)
		{
			// rotate 180 degrees (   0,   0) - ( 511, 511)
			color1 = m_videoram[(y * width) + x];
			color2 = m_videoram[(((height - 1) - y) * width) + (((width / 2) - 1) - x)];
			m_videoram[(y * width) + x] = color2;
			m_videoram[(((height - 1) - y) * width) + (((width / 2) - 1) - x)] = color1;
			// rotate 180 degrees ( 512,   0) - (1023, 511)
			color1 = m_videoram[(y * width) + (x + (width / 2))];
			color2 = m_videoram[(((height - 1) - y) * width) + ((((width / 2) - 1) - x) + (width / 2))];
			m_videoram[(y * width) + (x + (width / 2))] = color2;
			m_videoram[(((height - 1) - y) * width) + ((((width / 2) - 1) - x) + (width / 2))] = color1;
		}
	}

	m_flipscreen_old = m_flipscreen;
	m_screen_refresh = 1;
}

void nbmj8991_state::update_pixel(int x, int y)
{
	UINT8 color = m_videoram[(y * m_screen->width()) + x];
	m_tmpbitmap.pix16(y, x) = color;
}

void nbmj8991_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER:
		m_nb1413m3->m_busyflag = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in nbmj8991_state::device_timer");
	}
}

void nbmj8991_state::gfxdraw()
{
	UINT8 *GFX = memregion("gfx1")->base();
	int width = m_screen->width();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT8 color, color1, color2;
	int gfxaddr, gfxlen;

	m_nb1413m3->m_busyctr = 0;

	if (m_blitter_direction_x)
	{
		startx = m_blitter_destx;
		sizex = m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		startx = m_blitter_destx + m_blitter_sizex;
		sizex = m_blitter_sizex;
		skipx = -1;
	}

	if (m_blitter_direction_y)
	{
		starty = m_blitter_desty;
		sizey = m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		starty = m_blitter_desty + m_blitter_sizey;
		sizey = m_blitter_sizey;
		skipy = -1;
	}

	gfxlen = memregion("gfx1")->bytes();
	gfxaddr = (m_gfxrom << 17) + (m_blitter_src_addr << 1);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr &= (gfxlen - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x3ff;
			dx2 = (2 * x + 1) & 0x3ff;
			dy = y & 0x1ff;

			if (!m_flipscreen)
			{
				dx1 ^= 0x1ff;
				dx2 ^= 0x1ff;
				dy ^= 0x1ff;
			}

			if (m_blitter_direction_x)
			{
				// flip
				color1 = (color & 0x0f) >> 0;
				color2 = (color & 0xf0) >> 4;
			}
			else
			{
				// normal
				color1 = (color & 0xf0) >> 4;
				color2 = (color & 0x0f) >> 0;
			}

			color1 = m_clut[((m_clutsel & 0x7f) * 0x10) + color1];
			color2 = m_clut[((m_clutsel & 0x7f) * 0x10) + color2];

			if (color1 != 0xff)
			{
				m_videoram[(dy * width) + dx1] = color1;
				update_pixel(dx1, dy);
			}
			if (color2 != 0xff)
			{
				m_videoram[(dy * width) + dx2] = color2;
				update_pixel(dx2, dy);
			}

			m_nb1413m3->m_busyctr++;
		}
	}

	m_nb1413m3->m_busyflag = 0;
	m_blitter_timer->adjust(attotime::from_nsec(1650) * m_nb1413m3->m_busyctr);
}

/******************************************************************************


******************************************************************************/
void nbmj8991_state::video_start()
{
	m_blitter_timer = timer_alloc(TIMER_BLITTER);

	int width = m_screen->width();
	int height = m_screen->height();

	m_screen->register_screen_bitmap(m_tmpbitmap);
	m_videoram = auto_alloc_array(machine(), UINT8, width * height);
	m_clut = auto_alloc_array(machine(), UINT8, 0x800);
	memset(m_videoram, 0x00, (width * height * sizeof(UINT8)));

	m_screen_refresh = 1;

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_gfxrom));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_clutsel));
	save_pointer(NAME(m_videoram), width * height);
	save_pointer(NAME(m_clut), 0x800);
	save_item(NAME(m_flipscreen_old));

	machine().save().register_postload(save_prepost_delegate(FUNC(nbmj8991_state::postload), this));
}

void nbmj8991_state::postload()
{
	m_screen_refresh = 1;
}

UINT32 nbmj8991_state::screen_update_type1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	if (m_screen_refresh)
	{
		int width = m_screen->width();
		int height = m_screen->height();

		m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				update_pixel(x, y);
	}

	if (m_dispflag)
	{
		int scrollx, scrolly;

		if (m_flipscreen)
		{
			scrollx = (((-m_scrollx) + 0x000) & 0x1ff) * 2;
			scrolly =  ((-m_scrolly) - 0x00f) & 0x1ff;
		}
		else
		{
			scrollx = (((-m_scrollx) - 0x100) & 0x1ff) * 2;
			scrolly =  (( m_scrolly) + 0x0f1) & 0x1ff;
		}

		copyscrollbitmap(bitmap, m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}
	else
		bitmap.fill(0);

	return 0;
}

UINT32 nbmj8991_state::screen_update_type2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	if (m_screen_refresh)
	{
		int width = screen.width();
		int height = screen.height();

		m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				update_pixel(x, y);
	}

	if (m_nb1413m3->m_inputport & 0x20)
	{
		int scrollx, scrolly;

		if (m_flipscreen)
		{
			scrollx = (((-m_scrollx) + 0x000) & 0x1ff) * 2;
			scrolly =  ((-m_scrolly) - 0x00f) & 0x1ff;
		}
		else
		{
			scrollx = (((-m_scrollx) - 0x100) & 0x1ff) * 2;
			scrolly =  (( m_scrolly) + 0x0f1) & 0x1ff;
		}

		copyscrollbitmap(bitmap, m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}
	else
		bitmap.fill(0);

	return 0;
}
