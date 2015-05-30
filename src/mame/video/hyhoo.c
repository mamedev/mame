// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/01/28 -

******************************************************************************/

#include "emu.h"
#include "includes/hyhoo.h"


WRITE8_MEMBER(hyhoo_state::hyhoo_blitter_w)
{
	switch (offset)
	{
		case 0x00:  m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data;
					m_nb1413m3->gfxradr_l_w(space, 0, data); break;
		case 0x01:  m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8);
					m_nb1413m3->gfxradr_h_w(space, 0, data); break;
		case 0x02:  m_blitter_destx = data; break;
		case 0x03:  m_blitter_desty = data; break;
		case 0x04:  m_blitter_sizex = data; break;
		case 0x05:  m_blitter_sizey = data;
					/* writing here also starts the blit */
					hyhoo_gfxdraw();
					break;
		case 0x06:  m_blitter_direction_x = (data >> 0) & 0x01;
					m_blitter_direction_y = (data >> 1) & 0x01;
					m_flipscreen = (~data >> 2) & 0x01;
					m_dispflag = (~data >> 3) & 0x01;
					break;
		case 0x07:  break;
	}
}


WRITE8_MEMBER(hyhoo_state::hyhoo_romsel_w)
{
	int gfxlen = memregion("gfx1")->bytes();
	m_gfxrom = (((data & 0xc0) >> 4) + (data & 0x03));
	m_highcolorflag = data;
	m_nb1413m3->gfxrombank_w(space, 0, data);

	if ((0x20000 * m_gfxrom) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

void hyhoo_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER:
		m_nb1413m3->m_busyflag = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in hyhoo_state::device_timer");
	}
}

void hyhoo_state::hyhoo_gfxdraw()
{
	UINT8 *GFX = memregion("gfx1")->base();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	int gfxaddr, gfxlen;
	UINT8 color, color1, color2;
	int r, g, b;
	pen_t pen;

	m_nb1413m3->m_busyctr = 0;

	m_gfxrom |= ((m_nb1413m3->m_sndrombank1 & 0x02) << 3);

	startx = m_blitter_destx + m_blitter_sizex;
	starty = m_blitter_desty + m_blitter_sizey;

	if (m_blitter_direction_x)
	{
		sizex = m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		sizex = m_blitter_sizex;
		skipx = -1;
	}

	if (m_blitter_direction_y)
	{
		sizey = m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
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
				gfxaddr = 0;
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x1ff;
			dx2 = (2 * x + 1) & 0x1ff;
			dy = y & 0xff;

			if (m_highcolorflag & 0x04)
			{
				// direct mode

				if (color != 0xff)
				{
					if (m_highcolorflag & 0x20)
					{
						/* least significant bits */

						// src xxxxxxxx_bbbggrrr
						// dst xxbbbxxx_ggxxxrrr

						r = ((color & 0x07) >> 0) & 0x07;
						g = ((color & 0x18) >> 3) & 0x03;
						b = ((color & 0xe0) >> 5) & 0x07;

						pen = rgb_t(pal6bit(r), pal5bit(g), pal5bit(b));

						m_tmpbitmap.pix32(dy, dx1) = m_tmpbitmap.pix32(dy, dx1) | pen;
						m_tmpbitmap.pix32(dy, dx2) = m_tmpbitmap.pix32(dy, dx2) | pen;
					}
					else
					{
						/* most significant bits */

						// src xxxxxxxx_bbgggrrr
						// dst bbxxxggg_xxrrrxxx

						r = ((color & 0x07) >> 0) & 0x07;
						g = ((color & 0x38) >> 3) & 0x07;
						b = ((color & 0xc0) >> 6) & 0x03;

						pen = rgb_t(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

						m_tmpbitmap.pix32(dy, dx1) = pen;
						m_tmpbitmap.pix32(dy, dx2) = pen;
					}
				}
			}
			else
			{
				// lookup table mode

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

				if (m_clut[color1])
				{
					// src xxxxxxxx_bbgggrrr
					// dst bbxxxggg_xxrrrxxx

					r = ((~m_clut[color1] & 0x07) >> 0) & 0x07;
					g = ((~m_clut[color1] & 0x38) >> 3) & 0x07;
					b = ((~m_clut[color1] & 0xc0) >> 6) & 0x03;

					pen = rgb_t(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

					m_tmpbitmap.pix32(dy, dx1) = pen;
				}

				if (m_clut[color2])
				{
					// src xxxxxxxx_bbgggrrr
					// dst bbxxxggg_xxrrrxxx

					r = ((~m_clut[color2] & 0x07) >> 0) & 0x07;
					g = ((~m_clut[color2] & 0x38) >> 3) & 0x07;
					b = ((~m_clut[color2] & 0xc0) >> 6) & 0x03;

					pen = rgb_t(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

					m_tmpbitmap.pix32(dy, dx2) = pen;
				}
			}

			m_nb1413m3->m_busyctr++;
		}
	}

	m_nb1413m3->m_busyflag = 0;
	timer_set(attotime::from_hz(400000) * m_nb1413m3->m_busyctr, TIMER_BLITTER);
}


void hyhoo_state::video_start()
{
	m_screen->register_screen_bitmap(m_tmpbitmap);
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_gfxrom));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_highcolorflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_tmpbitmap));
}


UINT32 hyhoo_state::screen_update_hyhoo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_dispflag)
		copybitmap(bitmap, m_tmpbitmap, m_flipscreen, m_flipscreen, 0, 0, cliprect);
	else
		bitmap.fill(rgb_t::black, cliprect);

	return 0;
}
