// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "emu.h"
#include "includes/nbmj8900.h"


/******************************************************************************


******************************************************************************/
READ8_MEMBER(nbmj8900_state::palette_type1_r)
{
	return m_palette_ptr[offset];
}

WRITE8_MEMBER(nbmj8900_state::palette_type1_w)
{
	int r, g, b;

	m_palette_ptr[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_palette_ptr[offset + 0] & 0x0f) >> 0);
	g = ((m_palette_ptr[offset + 1] & 0xf0) >> 4);
	b = ((m_palette_ptr[offset + 1] & 0x0f) >> 0);

	m_palette->set_pen_color((offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

#ifdef UNUSED_FUNCTION
READ8_MEMBER(nbmj8900_state::palette_type2_r)
{
	return m_palette_ptr[offset];
}

WRITE8_MEMBER(nbmj8900_state::palette_type2_w)
{
	int r, g, b;

	m_palette_ptr[offset] = data;

	if (!(offset & 0x100)) return;

	offset &= 0x0ff;

	r = ((m_palette_ptr[offset + 0x000] & 0x0f) >> 0);
	g = ((m_palette_ptr[offset + 0x000] & 0xf0) >> 4);
	b = ((m_palette_ptr[offset + 0x100] & 0x0f) >> 0);

	m_palette->set_pen_color((offset & 0x0ff), pal4bit(r), pal4bit(g), pal4bit(b));
}

READ8_MEMBER(nbmj8900_state::palette_type3_r)
{
	return m_palette_ptr[offset];
}

WRITE8_MEMBER(nbmj8900_state::palette_type3_w)
{
	int r, g, b;

	m_palette_ptr[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_palette_ptr[offset + 1] & 0x0f) >> 0);
	g = ((m_palette_ptr[offset + 0] & 0xf0) >> 4);
	b = ((m_palette_ptr[offset + 0] & 0x0f) >> 0);

	m_palette->set_pen_color((offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}
#endif

WRITE8_MEMBER(nbmj8900_state::clutsel_w)
{
	m_clutsel = data;
}

READ8_MEMBER(nbmj8900_state::clut_r)
{
	return m_clut[offset];
}

WRITE8_MEMBER(nbmj8900_state::clut_w)
{
	m_clut[((m_clutsel & 0x7f) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(nbmj8900_state::blitter_w)
{
	switch (offset)
	{
		case 0x00:  m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 0x01:  m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 0x02:  m_blitter_destx = data; break;
		case 0x03:  m_blitter_desty = data; break;
		case 0x04:  m_blitter_sizex = data; break;
		case 0x05:  m_blitter_sizey = data;
					/* writing here also starts the blit */
					gfxdraw();
					break;
		case 0x06:  m_blitter_direction_x = (data & 0x01) ? 1 : 0;
					m_blitter_direction_y = (data & 0x02) ? 1 : 0;
					m_flipscreen = (data & 0x04) ? 1 : 0;
					m_dispflag = (data & 0x08) ? 0 : 1;
					if (m_gfxdraw_mode) vramflip(1);
					vramflip(0);
					break;
		case 0x07:  break;
	}
}

WRITE8_MEMBER(nbmj8900_state::scrolly_w)
{
	m_scrolly = data;
}

WRITE8_MEMBER(nbmj8900_state::vramsel_w)
{
	/* protection - not sure about this */
	m_nb1413m3->m_sndromrgntag = (data & 0x20) ? "protdata" : "voice";

	m_vram = data;
}

WRITE8_MEMBER(nbmj8900_state::romsel_w)
{
	m_gfxrom = (data & 0x0f);

	if ((0x20000 * m_gfxrom) > (memregion("gfx")->bytes() - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (memregion("gfx")->bytes() / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
void nbmj8900_state::vramflip(int vram)
{
	int x, y;
	unsigned char color1, color2;
	unsigned char *vidram;
	int width = m_screen->width();
	int height = m_screen->height();

	if (m_flipscreen == m_flipscreen_old) return;

	vidram = vram ? m_videoram1 : m_videoram0;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = vidram[(y * width) + x];
			color2 = vidram[((y ^ 0xff) * width) + (x ^ 0x1ff)];
			vidram[(y * width) + x] = color2;
			vidram[((y ^ 0xff) * width) + (x ^ 0x1ff)] = color1;
		}
	}

	m_flipscreen_old = m_flipscreen;
	m_screen_refresh = 1;
}


void nbmj8900_state::update_pixel0(int x, int y)
{
	UINT8 color = m_videoram0[(y * m_screen_width) + x];
	m_tmpbitmap0.pix16(y, x) = m_palette->pen(color);
}

void nbmj8900_state::update_pixel1(int x, int y)
{
	UINT8 color = m_videoram1[(y * m_screen_width) + x];
	m_tmpbitmap1.pix16(y, x) = m_palette->pen(color);
}

void nbmj8900_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER:
		m_nb1413m3->m_busyflag = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in nbmj8900_state::device_timer");
	}
}

void nbmj8900_state::gfxdraw()
{
	unsigned char *GFX = memregion("gfx")->base();

	int x, y;
	int dx1, dx2, dy1, dy2;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	unsigned char color, color1, color2;
	int gfxaddr;

	m_nb1413m3->m_busyctr = 0;

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

	gfxaddr = (m_gfxrom << 17) + (m_blitter_src_addr << 1);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (memregion("gfx")->bytes() - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr &= (memregion("gfx")->bytes() - 1);
			}

			color = GFX[gfxaddr++];

			// for hanamomo
			if ((m_nb1413m3->m_nb1413m3_type == NB1413M3_HANAMOMO) && ((gfxaddr >= 0x20000) && (gfxaddr < 0x28000)))
			{
				color |= ((color & 0x0f) << 4);
			}

			dx1 = (2 * x + 0) & 0x1ff;
			dx2 = (2 * x + 1) & 0x1ff;

			if (m_gfxdraw_mode)
			{
				// 2 layer type
				dy1 = y & 0xff;
				dy2 = (y + m_scrolly) & 0xff;
			}
			else
			{
				// 1 layer type
				dy1 = (y + m_scrolly) & 0xff;
				dy2 = 0;
			}

			if (!m_flipscreen)
			{
				dx1 ^= 0x1ff;
				dx2 ^= 0x1ff;
				dy1 ^= 0xff;
				dy2 ^= 0xff;
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

			color1 = m_clut[((m_clutsel & 0x7f) << 4) + color1];
			color2 = m_clut[((m_clutsel & 0x7f) << 4) + color2];

			if ((!m_gfxdraw_mode) || (m_vram & 0x01))
			{
				// layer 1
				if (color1 != 0xff)
				{
					m_videoram0[(dy1 * m_screen_width) + dx1] = color1;
					update_pixel0(dx1, dy1);
				}
				if (color2 != 0xff)
				{
					m_videoram0[(dy1 * m_screen_width) + dx2] = color2;
					update_pixel0(dx2, dy1);
				}
			}
			if (m_gfxdraw_mode && (m_vram & 0x02))
			{
				// layer 2
				if (m_vram & 0x08)
				{
					// transparent enable
					if (color1 != 0xff)
					{
						m_videoram1[(dy2 * m_screen_width) + dx1] = color1;
						update_pixel1(dx1, dy2);
					}
					if (color2 != 0xff)
					{
						m_videoram1[(dy2 * m_screen_width) + dx2] = color2;
						update_pixel1(dx2, dy2);
					}
				}
				else
				{
					// transparent disable
					m_videoram1[(dy2 * m_screen_width) + dx1] = color1;
					update_pixel1(dx1, dy2);
					m_videoram1[(dy2 * m_screen_width) + dx2] = color2;
					update_pixel1(dx2, dy2);
				}
			}

			m_nb1413m3->m_busyctr++;
		}
	}

	m_nb1413m3->m_busyflag = 0;
	m_blitter_timer->adjust(attotime::from_nsec(2500) * m_nb1413m3->m_busyctr);
}

/******************************************************************************


******************************************************************************/
void nbmj8900_state::video_start()
{
	m_screen_width = m_screen->width();
	m_screen_height = m_screen->height();

	m_blitter_timer = timer_alloc(TIMER_BLITTER);

	m_screen->register_screen_bitmap(m_tmpbitmap0);
	m_screen->register_screen_bitmap(m_tmpbitmap1);
	m_videoram0 = auto_alloc_array(machine(), UINT8, m_screen_width * m_screen_height);
	m_videoram1 = auto_alloc_array(machine(), UINT8, m_screen_width * m_screen_height);
	m_palette_ptr = auto_alloc_array(machine(), UINT8, 0x200);
	m_clut = auto_alloc_array(machine(), UINT8, 0x800);
	memset(m_videoram0, 0xff, (m_screen_width * m_screen_height * sizeof(UINT8)));
	memset(m_videoram1, 0xff, (m_screen_width * m_screen_height * sizeof(UINT8)));
//  m_palette->pen(0x07f) = 0xff;    /* palette_transparent_pen */
	m_gfxdraw_mode = 1;
	m_screen_refresh = 1;

	save_item(NAME(m_scrolly));
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_vram));
	save_item(NAME(m_gfxrom));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_clutsel));
	//save_item(NAME(m_gfxdraw_mode)); //always 1?
	save_pointer(NAME(m_videoram0), m_screen_width * m_screen_height);
	save_pointer(NAME(m_videoram1), m_screen_width * m_screen_height);
	save_pointer(NAME(m_palette_ptr), 0x200);
	save_pointer(NAME(m_clut), 0x800);
	save_item(NAME(m_flipscreen_old));
}

void nbmj8900_state::postload()
{
	m_screen_refresh = 1;
}

/******************************************************************************


******************************************************************************/
UINT32 nbmj8900_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	if (m_screen_refresh)
	{
		m_screen_refresh = 0;
		for (y = 0; y < m_screen_height; y++)
		{
			for (x = 0; x < m_screen_width; x++)
			{
				update_pixel0(x, y);
			}
		}
		if (m_gfxdraw_mode)
		{
			for (y = 0; y < m_screen_height; y++)
			{
				for (x = 0; x < m_screen_width; x++)
				{
					update_pixel1(x, y);
				}
			}
		}
	}

	if (m_dispflag)
	{
		int scrolly;
		if (!m_flipscreen) scrolly =   m_scrolly;
		else                      scrolly = (-m_scrolly) & 0xff;

		if (m_gfxdraw_mode)
		{
			copyscrollbitmap(bitmap, m_tmpbitmap0, 0, 0, 0, 0, cliprect);
			copyscrollbitmap_trans(bitmap, m_tmpbitmap1, 0, 0, 1, &scrolly, cliprect, 0xff);
		}
		else
		{
			copyscrollbitmap(bitmap, m_tmpbitmap0, 0, 0, 1, &scrolly, cliprect);
		}
	}
	else
	{
		bitmap.fill(0);
	}
	return 0;
}
