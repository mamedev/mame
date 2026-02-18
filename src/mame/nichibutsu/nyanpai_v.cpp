// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi 2000/12/23 -

******************************************************************************/

#include "emu.h"
#include "nyanpai.h"

/******************************************************************************


******************************************************************************/

void nyanpai_state::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 oldword = m_paletteram[offset];
	u16 newword;

	newword = COMBINE_DATA(&m_paletteram[offset]);

	if (oldword != newword)
	{
		const int offs_h = (offset / 0x180);
		const int offs_l = (offset & 0x7f);

		if (ACCESSING_BITS_8_15)
		{
			const int r = ((m_paletteram[(0x000 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			const int g = ((m_paletteram[(0x080 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			const int b = ((m_paletteram[(0x100 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);

			m_palette->set_pen_color(((offs_h << 8) + (offs_l << 1) + 0), rgb_t(r, g, b));
		}

		if (ACCESSING_BITS_0_7)
		{
			const int r = ((m_paletteram[(0x000 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			const int g = ((m_paletteram[(0x080 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			const int b = ((m_paletteram[(0x100 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);

			m_palette->set_pen_color(((offs_h << 8) + (offs_l << 1) + 1), rgb_t(r, g, b));
		}
	}
}

/******************************************************************************


******************************************************************************/
void nyanpai_state::vramflip(nyanpai_layer &layer)
{
	const int width = m_screen->width();
	const int height = m_screen->height();

	if (layer.m_flipscreen == layer.m_flipscreen_old) return;

	for (int y = 0; y < (height / 2); y++)
	{
		for (int x = 0; x < width; x++)
		{
			const u16 color1 = layer.m_videoram[(y * width) + x];
			const u16 color2 = layer.m_videoram[((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			layer.m_videoram[(y * width) + x] = color2;
			layer.m_videoram[((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	for (int y = 0; y < (height / 2); y++)
	{
		for (int x = 0; x < width; x++)
		{
			const u16 color1 = layer.m_videoworkram[(y * width) + x];
			const u16 color2 = layer.m_videoworkram[((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			layer.m_videoworkram[(y * width) + x] = color2;
			layer.m_videoworkram[((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	layer.m_flipscreen_old = layer.m_flipscreen;
	m_screen_refresh = 1;
}

void nyanpai_state::update_pixel(int vram, int x, int y)
{
	m_layer[vram].m_tmpbitmap.pix(y, x) = m_layer[vram].m_videoram[(y * m_screen->width()) + x];
}

TIMER_CALLBACK_MEMBER(nyanpai_state::clear_busy_flag)
{
	m_nb19010_busyflag = 1;
}

void nyanpai_state::gfxdraw(int vram)
{
	nyanpai_layer &layer = m_layer[vram];
	const int width = m_screen->width();

	m_nb19010_busyctr = 0;

	if (layer.m_clutmode)
	{
		// NB22090 clut256 mode
		layer.m_blitter_sizex = m_gfx[((layer.m_blitter_src_addr + 0) & 0x00ffffff)];
		layer.m_blitter_sizey = m_gfx[((layer.m_blitter_src_addr + 1) & 0x00ffffff)];
	}

	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	if (layer.m_blitter_direction_x)
	{
		startx = layer.m_blitter_destx;
		sizex = layer.m_blitter_sizex;
		skipx = 1;
	}
	else
	{
		startx = layer.m_blitter_destx + layer.m_blitter_sizex;
		sizex = layer.m_blitter_sizex;
		skipx = -1;
	}

	if (layer.m_blitter_direction_y)
	{
		starty = layer.m_blitter_desty;
		sizey = layer.m_blitter_sizey;
		skipy = 1;
	}
	else
	{
		starty = layer.m_blitter_desty + layer.m_blitter_sizey;
		sizey = layer.m_blitter_sizey;
		skipy = -1;
	}

	const size_t gfxlen = m_gfx.bytes();
	u32 gfxaddr = ((layer.m_blitter_src_addr + 2) & 0x00ffffff);

	for (int y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (int x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d", gfxaddr, startx, starty, sizex,sizey);
				logerror("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d\n", gfxaddr, startx, starty, sizex,sizey);
#endif
				gfxaddr &= (gfxlen - 1);
			}

			const u8 color = m_gfx[gfxaddr++];

			int dx1 = (2 * x + 0) & 0x3ff;
			int dx2 = (2 * x + 1) & 0x3ff;
			int dy = y & 0x1ff;

			if (!layer.m_flipscreen)
			{
				dx1 ^= 0x3ff;
				dx2 ^= 0x3ff;
				dy ^= 0x1ff;
			}

			u16 color1, color2;
			if (layer.m_blitter_direction_x)
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

			if (layer.m_clutmode)
			{
				// clut256 mode

				if (layer.m_clutsel & 0x80)
				{
					// clut256 mode 1st(low)
					layer.m_videoworkram[(dy * width) + dx1] &= 0x00f0;
					layer.m_videoworkram[(dy * width) + dx1] |= color1 & 0x0f;
					layer.m_videoworkram[(dy * width) + dx2] &= 0x00f0;
					layer.m_videoworkram[(dy * width) + dx2] |= color2 & 0x0f;

					continue;
				}
				else
				{
					// clut256 mode 2nd(high)
					layer.m_videoworkram[(dy * width) + dx1] &= 0x000f;
					layer.m_videoworkram[(dy * width) + dx1] |= (color1 & 0x0f) << 4;
					layer.m_videoworkram[(dy * width) + dx2] &= 0x000f;
					layer.m_videoworkram[(dy * width) + dx2] |= (color2 & 0x0f) << 4;

		//          layer.m_videoworkram[(dy * width) + dx1] += layer.m_clut[(layer.m_clutsel * 0x10)];
		//          layer.m_videoworkram[(dy * width) + dx2] += layer.m_clut[(layer.m_clutsel * 0x10)];
				}

				color1 = layer.m_videoworkram[(dy * width) + dx1];
				color2 = layer.m_videoworkram[(dy * width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = layer.m_clut[(layer.m_clutsel * 0x10) + color1];
				color2 = layer.m_clut[(layer.m_clutsel * 0x10) + color2];
			}

			color1 |= (0x0100 * vram);
			color2 |= (0x0100 * vram);

			if (((color1 & 0x00ff) != 0x00ff) || (!layer.m_transparency))
			{
				layer.m_videoram[(dy * width) + dx1] = color1;
				update_pixel(vram, dx1, dy);
			}
			if (((color2 & 0x00ff) != 0x00ff) || (!layer.m_transparency))
			{
				layer.m_videoram[(dy * width) + dx2] = color2;
				update_pixel(vram, dx2, dy);
			}

			m_nb19010_busyctr++;
		}
	}

	if (layer.m_clutmode)
	{
		// NB22090 clut256 mode
		layer.m_blitter_src_addr = gfxaddr;
	}

	m_nb19010_busyflag = 0;
	m_blitter_timer->adjust(attotime::from_nsec(1000 * m_nb19010_busyctr));
}

/******************************************************************************


******************************************************************************/
void nyanpai_state::video_start()
{
	const int width = m_screen->width();
	const int height = m_screen->height();

	for (int i = 0; i < 3; i++)
	{
		nyanpai_layer &layer = m_layer[i];
		m_screen->register_screen_bitmap(layer.m_tmpbitmap);
		layer.m_videoram = make_unique_clear<u16[]>(width * height);
		layer.m_videoworkram = make_unique_clear<u16[]>(width * height);
		layer.m_clut = std::make_unique<u8[]>(0x1000);

		save_pointer(NAME(layer.m_videoram), width * height, i);
		save_pointer(NAME(layer.m_videoworkram), width * height, i);
		save_pointer(NAME(layer.m_clut), 0x1000, i);
		save_item(NAME(layer.m_tmpbitmap), i);
	}
	m_nb19010_busyflag = 1;
	m_blitter_timer = timer_alloc(FUNC(nyanpai_state::clear_busy_flag), this);

	save_item(STRUCT_MEMBER(m_layer, m_scrollx));
	save_item(STRUCT_MEMBER(m_layer, m_scrolly));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_destx));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_desty));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_sizex));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_sizey));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_src_addr));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_direction_x));
	save_item(STRUCT_MEMBER(m_layer, m_blitter_direction_y));
	save_item(STRUCT_MEMBER(m_layer, m_dispflag));
	save_item(STRUCT_MEMBER(m_layer, m_flipscreen));
	save_item(STRUCT_MEMBER(m_layer, m_clutmode));
	save_item(STRUCT_MEMBER(m_layer, m_transparency));
	save_item(STRUCT_MEMBER(m_layer, m_clutsel));
	save_item(STRUCT_MEMBER(m_layer, m_flipscreen_old));
	save_item(NAME(m_screen_refresh));
	save_item(NAME(m_nb19010_busyctr));
	save_item(NAME(m_nb19010_busyflag));
}

/******************************************************************************


******************************************************************************/
u32 nyanpai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int scrollx[3]{}, scrolly[3]{};

	if (m_screen_refresh)
	{
		const int width = screen.width();
		const int height = screen.height();

		m_screen_refresh = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				update_pixel(0, x, y);
				update_pixel(1, x, y);
				update_pixel(2, x, y);
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		nyanpai_layer &layer = m_layer[i];
		if (layer.m_flipscreen)
		{
			scrollx[i] = (((-layer.m_scrollx) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = (-layer.m_scrolly) & 0x1ff;
		}
		else
		{
			scrollx[i] = (((-layer.m_scrollx) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = layer.m_scrolly & 0x1ff;
		}
	}

	if (m_layer[0].m_dispflag)
		copyscrollbitmap(bitmap, m_layer[0].m_tmpbitmap, 1, &scrollx[0], 1, &scrolly[0], cliprect);
	else
		bitmap.fill(0x00ff);

	if (m_layer[1].m_dispflag)
		copyscrollbitmap_trans(bitmap, m_layer[1].m_tmpbitmap, 1, &scrollx[1], 1, &scrolly[1], cliprect, 0x01ff);

	if (m_layer[2].m_dispflag)
		copyscrollbitmap_trans(bitmap, m_layer[2].m_tmpbitmap, 1, &scrollx[2], 1, &scrolly[2], cliprect, 0x02ff);

	return 0;
}
