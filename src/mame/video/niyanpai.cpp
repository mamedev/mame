// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/

#include "emu.h"
#include "includes/niyanpai.h"

/******************************************************************************


******************************************************************************/
READ16_MEMBER(niyanpai_state::palette_r)
{
	return m_palette_ptr[offset];
}

WRITE16_MEMBER(niyanpai_state::palette_w)
{
	int r, g, b;
	int offs_h, offs_l;
	UINT16 oldword = m_palette_ptr[offset];
	UINT16 newword;

	COMBINE_DATA(&m_palette_ptr[offset]);
	newword = m_palette_ptr[offset];

	if (oldword != newword)
	{
		offs_h = (offset / 0x180);
		offs_l = (offset & 0x7f);

		if (ACCESSING_BITS_8_15)
		{
			r  = ((m_palette_ptr[(0x000 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			g  = ((m_palette_ptr[(0x080 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			b  = ((m_palette_ptr[(0x100 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);

			m_palette->set_pen_color(((offs_h << 8) + (offs_l << 1) + 0), rgb_t(r, g, b));
		}

		if (ACCESSING_BITS_0_7)
		{
			r  = ((m_palette_ptr[(0x000 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			g  = ((m_palette_ptr[(0x080 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			b  = ((m_palette_ptr[(0x100 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);

			m_palette->set_pen_color(((offs_h << 8) + (offs_l << 1) + 1), rgb_t(r, g, b));
		}
	}
}

/******************************************************************************


******************************************************************************/
int niyanpai_state::blitter_r(int vram, int offset)
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

void niyanpai_state::blitter_w(int vram, int offset, UINT8 data)
{
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
		case 0x02:  m_scrollx[vram] = (m_scrollx[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
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

void niyanpai_state::clutsel_w(int vram, UINT8 data)
{
	m_clutsel[vram] = data;
}

void niyanpai_state::clut_w(int vram, int offset, UINT8 data)
{
	m_clut[vram][((m_clutsel[vram] & 0xff) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
void niyanpai_state::vramflip(int vram)
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

	m_flipscreen_old[vram] = m_flipscreen[vram];
	m_screen_refresh = 1;
}

void niyanpai_state::update_pixel(int vram, int x, int y)
{
	UINT16 color = m_videoram[vram][(y * m_screen->width()) + x];
	m_tmpbitmap[vram].pix16(y, x) = color;
}

void niyanpai_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER:
		m_nb19010_busyflag = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in niyanpai_state::device_timer");
	}
}

void niyanpai_state::gfxdraw(int vram)
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

	if (m_clutmode[vram])
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

			if (m_clutmode[vram])
			{
				// clut256 mode

				if (m_clutsel[vram] & 0x80)
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

		//          m_videoworkram[vram][(dy * width) + dx1] += m_clut[vram][(m_clutsel[vram] * 0x10)];
		//          m_videoworkram[vram][(dy * width) + dx2] += m_clut[vram][(m_clutsel[vram] * 0x10)];
				}

				color1 = m_videoworkram[vram][(dy * width) + dx1];
				color2 = m_videoworkram[vram][(dy * width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = m_clut[vram][(m_clutsel[vram] * 0x10) + color1];
				color2 = m_clut[vram][(m_clutsel[vram] * 0x10) + color2];
			}

			color1 |= (0x0100 * vram);
			color2 |= (0x0100 * vram);

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

	if (m_clutmode[vram])
	{
		// NB22090 clut256 mode
		m_blitter_src_addr[vram] = gfxaddr;
	}

	m_nb19010_busyflag = 0;
	m_blitter_timer->adjust(attotime::from_nsec(1000 * m_nb19010_busyctr));
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(niyanpai_state::blitter_0_w){ blitter_w(0, offset, data); }
WRITE8_MEMBER(niyanpai_state::blitter_1_w){ blitter_w(1, offset, data); }
WRITE8_MEMBER(niyanpai_state::blitter_2_w){ blitter_w(2, offset, data); }

READ8_MEMBER(niyanpai_state::blitter_0_r){ return blitter_r(0, offset); }
READ8_MEMBER(niyanpai_state::blitter_1_r){ return blitter_r(1, offset); }
READ8_MEMBER(niyanpai_state::blitter_2_r){ return blitter_r(2, offset); }

WRITE8_MEMBER(niyanpai_state::clut_0_w){ clut_w(0, offset, data); }
WRITE8_MEMBER(niyanpai_state::clut_1_w){ clut_w(1, offset, data); }
WRITE8_MEMBER(niyanpai_state::clut_2_w){ clut_w(2, offset, data); }

WRITE8_MEMBER(niyanpai_state::clutsel_0_w){ clutsel_w(0, data); }
WRITE8_MEMBER(niyanpai_state::clutsel_1_w){ clutsel_w(1, data); }
WRITE8_MEMBER(niyanpai_state::clutsel_2_w){ clutsel_w(2, data); }

/******************************************************************************


******************************************************************************/
void niyanpai_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_screen->register_screen_bitmap(m_tmpbitmap[0]);
	m_screen->register_screen_bitmap(m_tmpbitmap[1]);
	m_screen->register_screen_bitmap(m_tmpbitmap[2]);
	m_videoram[0] = auto_alloc_array_clear(machine(), UINT16, width * height);
	m_videoram[1] = auto_alloc_array_clear(machine(), UINT16, width * height);
	m_videoram[2] = auto_alloc_array_clear(machine(), UINT16, width * height);
	m_videoworkram[0] = auto_alloc_array_clear(machine(), UINT16, width * height);
	m_videoworkram[1] = auto_alloc_array_clear(machine(), UINT16, width * height);
	m_videoworkram[2] = auto_alloc_array_clear(machine(), UINT16, width * height);
	m_palette_ptr = std::make_unique<UINT16[]>(0x480);
	m_clut[0] = std::make_unique<UINT8[]>(0x1000);
	m_clut[1] = std::make_unique<UINT8[]>(0x1000);
	m_clut[2] = std::make_unique<UINT8[]>(0x1000);
	m_nb19010_busyflag = 1;
	m_blitter_timer = timer_alloc(TIMER_BLITTER);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
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
	save_item(NAME(m_screen_refresh));
	save_item(NAME(m_nb19010_busyctr));
	save_item(NAME(m_nb19010_busyflag));
	save_item(NAME(m_flipscreen_old));
	save_pointer(NAME(m_palette_ptr.get()), 0x480);
	save_pointer(NAME(m_videoram[0]), width * height);
	save_pointer(NAME(m_videoram[1]), width * height);
	save_pointer(NAME(m_videoram[2]), width * height);
	save_pointer(NAME(m_videoworkram[0]), width * height);
	save_pointer(NAME(m_videoworkram[1]), width * height);
	save_pointer(NAME(m_videoworkram[2]), width * height);
	save_pointer(NAME(m_clut[0].get()), 0x1000);
	save_pointer(NAME(m_clut[1].get()), 0x1000);
	save_pointer(NAME(m_clut[2].get()), 0x1000);
	save_item(NAME(m_tmpbitmap[0]));
	save_item(NAME(m_tmpbitmap[1]));
	save_item(NAME(m_tmpbitmap[2]));
}

/******************************************************************************


******************************************************************************/
UINT32 niyanpai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	int x, y;
	int scrollx[3], scrolly[3];

	if (m_screen_refresh)
	{
		int width = screen.width();
		int height = screen.height();

		m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
			{
				update_pixel(0, x, y);
				update_pixel(1, x, y);
				update_pixel(2, x, y);
			}
	}

	for (i = 0; i < 3; i++)
	{
		if (m_flipscreen[i])
		{
			scrollx[i] = (((-m_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = (-m_scrolly[i]) & 0x1ff;
		}
		else
		{
			scrollx[i] = (((-m_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = m_scrolly[i] & 0x1ff;
		}
	}

	if (m_dispflag[0])
		copyscrollbitmap(bitmap, m_tmpbitmap[0], 1, &scrollx[0], 1, &scrolly[0], cliprect);
	else
		bitmap.fill(0x00ff);

	if (m_dispflag[1])
		copyscrollbitmap_trans(bitmap, m_tmpbitmap[1], 1, &scrollx[1], 1, &scrolly[1], cliprect, 0x01ff);

	if (m_dispflag[2])
		copyscrollbitmap_trans(bitmap, m_tmpbitmap[2], 1, &scrollx[2], 1, &scrolly[2], cliprect, 0x02ff);

	return 0;
}
