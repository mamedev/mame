// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -
    Special thanks to Tatsuyuki Satoh

******************************************************************************/

#include "emu.h"
#include "includes/nbmj9195.h"

/******************************************************************************


******************************************************************************/

WRITE8_MEMBER(nbmj9195_state::palette_w)
{
	m_palette_ptr[offset] = data;

	if (offset & 1)
	{
		offset &= 0x1fe;

		int r = ((m_palette_ptr[offset + 0] & 0x0f) >> 0);
		int g = ((m_palette_ptr[offset + 0] & 0xf0) >> 4);
		int b = ((m_palette_ptr[offset + 1] & 0x0f) >> 0);

		m_palette->set_pen_color((offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

WRITE8_MEMBER(nbmj9195_state::nb22090_palette_w)
{
	int r, g, b;
	int offs_h, offs_l;

	m_palette_ptr[offset] = data;

	offs_h = (offset / 0x0300);
	offs_l = (offset & 0x00ff);

	r = m_palette_ptr[(0x000 + (offs_h * 0x300) + offs_l)];
	g = m_palette_ptr[(0x100 + (offs_h * 0x300) + offs_l)];
	b = m_palette_ptr[(0x200 + (offs_h * 0x300) + offs_l)];

	m_palette->set_pen_color(((offs_h * 0x100) + offs_l), rgb_t(r, g, b));
}

/******************************************************************************


******************************************************************************/
int nbmj9195_state::blitter_r(int offset, int vram)
{
	int ret;
	UINT8 *GFXROM = memregion("gfx1")->base();

	switch (offset)
	{
		case 0x00:  ret = 0xfe | ((m_nb19010_busyflag & 0x01) ^ 0x01); break;    // NB19010 Busy Flag
		case 0x01:  ret = GFXROM[m_blitter_src_addr[vram]]; break;           // NB19010 GFX-ROM Read
		default:    ret = 0xff; break;
	}

	return ret;
}

void nbmj9195_state::blitter_w(int offset, int data, int vram)
{
	int new_line;

	switch (offset)
	{
		case 0x00:  m_blitter_direction_x[vram] = (data & 0x01) ? 1 : 0;
					m_blitter_direction_y[vram] = (data & 0x02) ? 1 : 0;
					m_clutmode[vram] = (data & 0x04) ? 1 : 0;
				//  if (data & 0x08) popmessage("Unknown GFX Flag!! (0x08)");
					m_transparency[vram] = (data & 0x10) ? 1 : 0;
				//  if (data & 0x20) popmessage("Unknown GFX Flag!! (0x20)");
					m_flipscreen[vram] = (data & 0x40) ? 0 : 1;
					m_dispflag[vram] = (data & 0x80) ? 1 : 0;
					vramflip(vram);
					break;
		case 0x01:  m_scrollx[vram] = (m_scrollx[vram] & 0x0100) | data; break;
		case 0x02:  m_scrollx[vram] = (m_scrollx[vram] & 0x00ff) | ((data << 8) & 0x0100);
					new_line = m_screen->vpos();
					if (m_flipscreen[vram])
					{
						for ( ; m_scanline[vram] < new_line; m_scanline[vram]++)
							m_scrollx_raster[vram][m_scanline[vram]] = (((-m_scrollx[vram]) - 0x4e)  & 0x1ff) << 1;
					}
					else
					{
						for ( ; m_scanline[vram] < new_line; m_scanline[vram]++)
							m_scrollx_raster[vram][(m_scanline[vram] ^ 0x1ff)] = (((-m_scrollx[vram]) - 0x4e)  & 0x1ff) << 1;
					}
					break;
		case 0x03:  m_scrolly[vram] = (m_scrolly[vram] & 0x0100) | data; break;
		case 0x04:  m_scrolly[vram] = (m_scrolly[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x05:  m_blitter_src_addr[vram] = (m_blitter_src_addr[vram] & 0xffff00) | data; break;
		case 0x06:  m_blitter_src_addr[vram] = (m_blitter_src_addr[vram] & 0xff00ff) | (data << 8); break;
		case 0x07:  m_blitter_src_addr[vram] = (m_blitter_src_addr[vram] & 0x00ffff) | (data << 16); break;
		case 0x08:  m_blitter_sizex[vram] = data; break;
		case 0x09:  m_blitter_sizey[vram] = data; break;
		case 0x0a:  m_blitter_destx[vram] = (m_blitter_destx[vram]  & 0xff00) | data; break;
		case 0x0b:  m_blitter_destx[vram] = (m_blitter_destx[vram]  & 0x00ff) | (data << 8); break;
		case 0x0c:  m_blitter_desty[vram] = (m_blitter_desty[vram]  & 0xff00) | data; break;
		case 0x0d:  m_blitter_desty[vram] = (m_blitter_desty[vram]  & 0x00ff) | (data << 8);
					gfxdraw(vram);
					break;
		default:    break;
	}
}

WRITE8_MEMBER(nbmj9195_state::clutsel_w)
{
	m_clutsel = data;
}

void nbmj9195_state::clut_w(int offset, int data, int vram)
{
	m_clut[vram][((m_clutsel & 0xff) * 0x10) + (offset & 0x0f)] = data;
}

WRITE8_MEMBER(nbmj9195_state::gfxflag2_w)
{
	m_gfxflag2 = data;
}

/******************************************************************************


******************************************************************************/
void nbmj9195_state::vramflip(int vram)
{
	int x, y;
	UINT16 color1, color2;
	int width = m_screen->width();
	int height = m_screen->height();

	if (m_flipscreen[vram] == m_flipscreen_old[vram]) return;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = m_videoram[vram][(y * width) + x];
			color2 = m_videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			m_videoram[vram][(y * width) + x] = color2;
			m_videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	if (m_gfxdraw_mode == 2)
	{
		for (y = 0; y < (height / 2); y++)
		{
			for (x = 0; x < width; x++)
			{
				color1 = m_videoworkram[vram][(y * width) + x];
				color2 = m_videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
				m_videoworkram[vram][(y * width) + x] = color2;
				m_videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
			}
		}
	}

	m_flipscreen_old[vram] = m_flipscreen[vram];
	m_screen_refresh = 1;
}

void nbmj9195_state::update_pixel(int vram, int x, int y)
{
	UINT16 color = m_videoram[vram][(y * m_screen->width()) + x];
	m_tmpbitmap[vram].pix16(y, x) = color;
}

void nbmj9195_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER:
		m_nb19010_busyflag = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in nbmj9195_state::device_timer");
	}
}

void nbmj9195_state::gfxdraw(int vram)
{
	UINT8 *GFX = memregion("gfx1")->base();
	int width = m_screen->width();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT16 color, color1, color2;
	int gfxaddr, gfxlen;

	m_nb19010_busyctr = 0;

	if ((m_gfxdraw_mode == 2) && (m_clutmode[vram]))
	{
		// NB22090 clut256 mode
		m_blitter_sizex[vram] = GFX[((m_blitter_src_addr[vram] + 0) & 0x00ffffff)];
		m_blitter_sizey[vram] = GFX[((m_blitter_src_addr[vram] + 1) & 0x00ffffff)];
	}

	if (m_blitter_direction_x[vram])
	{
		startx = m_blitter_destx[vram];
		sizex = m_blitter_sizex[vram];
		skipx = 1;
	}
	else
	{
		startx = m_blitter_destx[vram] + m_blitter_sizex[vram];
		sizex = m_blitter_sizex[vram];
		skipx = -1;
	}

	if (m_blitter_direction_y[vram])
	{
		starty = m_blitter_desty[vram];
		sizey = m_blitter_sizey[vram];
		skipy = 1;
	}
	else
	{
		starty = m_blitter_desty[vram] + m_blitter_sizey[vram];
		sizey = m_blitter_sizey[vram];
		skipy = -1;
	}

	gfxlen = memregion("gfx1")->bytes();
	gfxaddr = ((m_blitter_src_addr[vram] + 2) & 0x00ffffff);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d", gfxaddr, startx, starty, sizex,sizey);
				logerror("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d\n", gfxaddr, startx, starty, sizex,sizey);
#endif
				gfxaddr &= (gfxlen - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x3ff;
			dx2 = (2 * x + 1) & 0x3ff;
			dy = y & 0x1ff;

			if (!m_flipscreen[vram])
			{
				dx1 ^= 0x3ff;
				dx2 ^= 0x3ff;
				dy ^= 0x1ff;
			}

			if (m_blitter_direction_x[vram])
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

			if ((m_gfxdraw_mode == 2) && (m_clutmode[vram]))
			{
				// clut256 mode

				if (m_gfxflag2 & 0xc0)
				{
					// clut256 mode 1st(low)
					m_videoworkram[vram][(dy * width) + dx1] &= 0x00f0;
					m_videoworkram[vram][(dy * width) + dx1] |= color1 & 0x0f;
					m_videoworkram[vram][(dy * width) + dx2] &= 0x00f0;
					m_videoworkram[vram][(dy * width) + dx2] |= color2 & 0x0f;

					continue;
				}
				else
				{
					// clut256 mode 2nd(high)
					m_videoworkram[vram][(dy * width) + dx1] &= 0x000f;
					m_videoworkram[vram][(dy * width) + dx1] |= (color1 & 0x0f) << 4;
					m_videoworkram[vram][(dy * width) + dx2] &= 0x000f;
					m_videoworkram[vram][(dy * width) + dx2] |= (color2 & 0x0f) << 4;

					m_videoworkram[vram][(dy * width) + dx1] += m_clut[vram][(m_clutsel * 0x10)];
					m_videoworkram[vram][(dy * width) + dx2] += m_clut[vram][(m_clutsel * 0x10)];
				}

				color1 = m_videoworkram[vram][(dy * width) + dx1];
				color2 = m_videoworkram[vram][(dy * width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = m_clut[vram][(m_clutsel * 0x10) + color1];
				color2 = m_clut[vram][(m_clutsel * 0x10) + color2];
			}

			if (m_gfxdraw_mode == 2)
			{
				color1 |= (0x0100 * vram);
				color2 |= (0x0100 * vram);
			}

			if (((color1 & 0x00ff) != 0x00ff) || (!m_transparency[vram]))
			{
				m_videoram[vram][(dy * width) + dx1] = color1;
				update_pixel(vram, dx1, dy);
			}
			if (((color2 & 0x00ff) != 0x00ff) || (!m_transparency[vram]))
			{
				m_videoram[vram][(dy * width) + dx2] = color2;
				update_pixel(vram, dx2, dy);
			}

			m_nb19010_busyctr++;
		}
	}

	if ((m_gfxdraw_mode == 2) && (m_clutmode[vram]))
	{
		// NB22090 clut256 mode
		m_blitter_src_addr[vram] = gfxaddr;
	}

	m_nb19010_busyflag = 0;

	/* 1650ns per count */
	m_blitter_timer->adjust(attotime::from_nsec(m_nb19010_busyctr * 1650));
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(nbmj9195_state::blitter_0_w){ blitter_w(offset, data, 0); }
WRITE8_MEMBER(nbmj9195_state::blitter_1_w){ blitter_w(offset, data, 1); }

READ8_MEMBER(nbmj9195_state::blitter_0_r){ return blitter_r(offset, 0); }
READ8_MEMBER(nbmj9195_state::blitter_1_r){ return blitter_r(offset, 1); }

WRITE8_MEMBER(nbmj9195_state::clut_0_w){ clut_w(offset, data, 0); }
WRITE8_MEMBER(nbmj9195_state::clut_1_w){ clut_w(offset, data, 1); }

/******************************************************************************


******************************************************************************/
VIDEO_START_MEMBER(nbmj9195_state,_1layer)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_blitter_timer = timer_alloc(TIMER_BLITTER);

	m_screen->register_screen_bitmap(m_tmpbitmap[0]);
	m_videoram[0] = make_unique_clear<UINT16[]>(width * height);
	m_clut[0] = std::make_unique<UINT8[]>(0x1000);
	m_scanline[0] = m_scanline[1] = SCANLINE_MIN;
	m_nb19010_busyflag = 1;
	m_gfxdraw_mode = 0;

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_scrollx_raster));
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_clutmode));
	save_item(NAME(m_transparency));
	save_item(NAME(m_clutsel));
	save_item(NAME(m_gfxflag2));
	save_item(NAME(m_gfxdraw_mode));
	save_item(NAME(m_nb19010_busyctr));
	save_item(NAME(m_nb19010_busyflag));
	save_pointer(NAME(m_videoram[0].get()), width * height);
	save_pointer(NAME(m_clut[0].get()), 0x1000);
	save_item(NAME(m_flipscreen_old));
	machine().save().register_postload(save_prepost_delegate(FUNC(nbmj9195_state::postload), this));
}

void nbmj9195_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_blitter_timer = timer_alloc(TIMER_BLITTER);

	m_screen->register_screen_bitmap(m_tmpbitmap[0]);
	m_screen->register_screen_bitmap(m_tmpbitmap[1]);
	m_videoram[0] = make_unique_clear<UINT16[]>(width * height);
	m_videoram[1] = make_unique_clear<UINT16[]>(width * height);
	m_clut[0] = std::make_unique<UINT8[]>(0x1000);
	m_clut[1] = std::make_unique<UINT8[]>(0x1000);
	m_scanline[0] = m_scanline[1] = SCANLINE_MIN;
	m_nb19010_busyflag = 1;
	m_gfxdraw_mode = 1;
	m_screen_refresh = 1;

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_scrollx_raster));
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_clutmode));
	save_item(NAME(m_transparency));
	save_item(NAME(m_clutsel));
	save_item(NAME(m_gfxflag2));
	save_item(NAME(m_gfxdraw_mode));
	save_item(NAME(m_nb19010_busyctr));
	save_item(NAME(m_nb19010_busyflag));
	save_pointer(NAME(m_videoram[0].get()), width * height);
	save_pointer(NAME(m_videoram[1].get()), width * height);
	save_pointer(NAME(m_clut[0].get()), 0x1000);
	save_pointer(NAME(m_clut[1].get()), 0x1000);
	save_item(NAME(m_flipscreen_old));
	machine().save().register_postload(save_prepost_delegate(FUNC(nbmj9195_state::postload), this));
}

void nbmj9195_state::postload()
{
	m_screen_refresh = 1;
}

VIDEO_START_MEMBER(nbmj9195_state,nb22090)
{
	nbmj9195_state::video_start();

	int width = m_screen->width();
	int height = m_screen->height();

	m_videoworkram[0] = make_unique_clear<UINT16[]>(width * height);
	m_videoworkram[1] = make_unique_clear<UINT16[]>(width * height);

	save_pointer(NAME(m_videoworkram[0].get()), width * height);
	save_pointer(NAME(m_videoworkram[1].get()), width * height);

	m_gfxdraw_mode = 2;
}

/******************************************************************************


******************************************************************************/
UINT32 nbmj9195_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	int x, y;
	int scrolly[2];

	if (m_screen_refresh)
	{
		int width = screen.width();
		int height = screen.height();

		m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
			{
				update_pixel(0, x, y);

				if (m_gfxdraw_mode)
					update_pixel(1, x, y);
			}
	}

	for (i = 0; i < 2; i++)
	{
		if (m_flipscreen[i])
		{
			for ( ; m_scanline[i] < SCANLINE_MAX; m_scanline[i]++)
			{
				m_scrollx_raster[i][m_scanline[i]] = (((-m_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			}
			scrolly[i] = (-m_scrolly[i]) & 0x1ff;
		}
		else
		{
			for ( ; m_scanline[i] < SCANLINE_MAX; m_scanline[i]++)
			{
				m_scrollx_raster[i][(m_scanline[i] ^ 0x1ff)] = (((-m_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			}
			scrolly[i] = m_scrolly[i] & 0x1ff;
		}
		m_scanline[i] = SCANLINE_MIN;
	}

	if (m_dispflag[0])
		// nbmj9195 1layer
		copyscrollbitmap(bitmap, m_tmpbitmap[0], SCANLINE_MAX, m_scrollx_raster[0], 1, &scrolly[0], cliprect);
	else
		bitmap.fill(0x0ff);

	if (m_dispflag[1])
	{
		if (m_gfxdraw_mode == 1)
			// nbmj9195 2layer
			copyscrollbitmap_trans(bitmap, m_tmpbitmap[1], SCANLINE_MAX, m_scrollx_raster[1], 1, &scrolly[1], cliprect, 0x0ff);

		if (m_gfxdraw_mode == 2)
			// nbmj9195 nb22090 2layer
			copyscrollbitmap_trans(bitmap, m_tmpbitmap[1], SCANLINE_MAX, m_scrollx_raster[1], 1, &scrolly[1], cliprect, 0x1ff);
	}
	return 0;
}
