// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi 2000/06/07 -

******************************************************************************/

#include "emu.h"
#include "pastelg.h"


// pastelg specific methods

uint16_t pastelg_state::blitter_src_addr_r()
{
	return m_blitter_src_addr;
}

void pastelg_state::romsel_w(uint8_t data)
{
	m_gfxbank = ((data & 0xc0) >> 6);
	m_palbank = ((data & 0x10) >> 4);
	m_nb1413m3->sndrombank1_w(data);

	if ((m_gfxbank << 16) >= m_blitter_rom.length())
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		// FIXME: this isn't a power-of-two size, subtracting 1 doesn't generate a valid mask
		m_gfxbank &= (m_blitter_rom.length() / 0x20000 - 1);
	}
}

// threeds specific methods

void threeds_state::romsel_w(uint8_t data)
{
	if (data & 0xfc) printf("%02x\n", data);
	m_gfxbank = (data & 0x3);
}

void threeds_state::output_w(uint8_t data)
{
	m_palbank = ((data & 0x10) >> 4);

}

uint8_t threeds_state::rom_readback_r()
{
	return m_blitter_rom[(m_blitter_src_addr | (m_gfxbank << 16)) & 0x3ffff];
}

// common methods

void pastelg_common_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[palette.entries()], 0);
		bit1 = BIT(color_prom[palette.entries()], 1);
		bit2 = BIT(color_prom[palette.entries()], 2);
		bit3 = BIT(color_prom[palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

void pastelg_common_state::blitter_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 1: m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 2: m_blitter_destx = data; break;
		case 3: m_blitter_desty = data; break;
		case 4: m_blitter_sizex = data; break;
		case 5: m_blitter_sizey = data;
				// writing here also starts the blit
				gfxdraw();
				break;
		case 6: m_blitter_direction_x = (data & 0x01) ? 1 : 0;
				m_blitter_direction_y = (data & 0x02) ? 1 : 0;
				m_flipscreen = (data & 0x04) ? 0 : 1;
				m_dispflag = (data & 0x08) ? 0 : 1;
				vramflip();
				break;
	}
}

void pastelg_common_state::vramflip()
{
	int width = m_screen->width();
	int height = m_screen->height();

	if (m_flipscreen == m_flipscreen_old) return;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			uint8_t color1 = m_videoram[(y * width) + x];
			uint8_t color2 = m_videoram[((y ^ 0xff) * width) + (x ^ 0xff)];
			m_videoram[(y * width) + x] = color2;
			m_videoram[((y ^ 0xff) * width) + (x ^ 0xff)] = color1;
		}
	}

	m_flipscreen_old = m_flipscreen;
}

void pastelg_common_state::blitter_timer_callback(s32 param)
{
	m_nb1413m3->busyflag_w(1);
}


void pastelg_common_state::gfxdraw()
{
	int width = m_screen->width();

	int sizex, sizey;
	int incx, incy;

	m_nb1413m3->m_busyctr = 0;

	int startx = m_blitter_destx + m_blitter_sizex;
	int starty = m_blitter_desty + m_blitter_sizey;


	if (m_blitter_direction_x)
	{
		if (m_blitter_sizex & 0x80) sizex = 0xff - m_blitter_sizex;
		else sizex = m_blitter_sizex;
		incx = 1;
	}
	else
	{
		sizex = m_blitter_sizex;
		incx = -1;
	}

	if (m_blitter_direction_y)
	{
		if (m_blitter_sizey & 0x80) sizey = 0xff - m_blitter_sizey;
		else sizey = m_blitter_sizey;
		incy = 1;
	}
	else
	{
		sizey = m_blitter_sizey;
		incy = -1;
	}

	int gfxaddr = (m_gfxbank << 16) + m_blitter_src_addr;

	int readflag = 0;

	int count = 0;
	int y = starty;

	for (int ctry = sizey; ctry >= 0; ctry--)
	{
		int x = startx;

		for (int ctrx = sizex; ctrx >= 0; ctrx--)
		{
			gfxaddr = (m_gfxbank << 16) + ((m_blitter_src_addr + count));

			if (gfxaddr >= m_blitter_rom.length())
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr = 0;
			}

			uint8_t color = m_blitter_rom[gfxaddr];

			int dx = x & 0xff;
			int dy = y & 0xff;

			if (m_flipscreen)
			{
				dx ^= 0xff;
				dy ^= 0xff;
			}

			if (!readflag)
			{
				// 1st, 3rd, 5th, ... read
				color = (color & 0x0f);
			}
			else
			{
				// 2nd, 4th, 6th, ... read
				color = (color & 0xf0) >> 4;
				count++;
			}

			readflag ^= 1;

			if (m_clut[color] & 0xf0)
			{
				if (color)
				{
					color = ((m_palbank * 0x10) + color);
					m_videoram[(dy * width) + dx] = color;
				}
			}
			else
			{
				if(m_clut[color] != 0)
				{
					color = ((m_palbank * 0x10) + m_clut[color]);
					m_videoram[(dy * width) + dx] = color;
				}
			}

			m_nb1413m3->m_busyctr++;
			x += incx;
		}

		y += incy;
	}

	m_nb1413m3->busyflag_w(0);
	m_blitter_timer->adjust(attotime::from_hz(400000) * m_nb1413m3->m_busyctr);
}

/******************************************************************************


******************************************************************************/
void pastelg_common_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_videoram = make_unique_clear<uint8_t[]>(width * height);

	m_blitter_timer = timer_alloc(FUNC(pastelg_state::blitter_timer_callback), this);

	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_palbank));
	save_pointer(NAME(m_videoram), width*height);
	save_item(NAME(m_flipscreen_old));

	m_palbank = 0;
}

/******************************************************************************


******************************************************************************/
uint32_t pastelg_common_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_dispflag)
	{
		int width = screen.width();
		int height = screen.height();

		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				bitmap.pix(y, x) = m_videoram[(y * width) + x];
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}
