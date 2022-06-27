// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#include "emu.h"
#include "pk8000_v.h"

uint8_t pk8000_base_state::video_color_r()
{
	return m_video_color;
}

void pk8000_base_state::video_color_w(uint8_t data)
{
	m_video_color = data;
}

uint8_t pk8000_base_state::text_start_r()
{
	return m_text_start;
}

void pk8000_base_state::text_start_w(uint8_t data)
{
	m_text_start = data;
}

uint8_t pk8000_base_state::chargen_start_r()
{
	return m_chargen_start;
}

void pk8000_base_state::chargen_start_w(uint8_t data)
{
	m_chargen_start = data;
}

uint8_t pk8000_base_state::video_start_r()
{
	return m_video_start;
}

void pk8000_base_state::video_start_w(uint8_t data)
{
	m_video_start = data;
}

uint8_t pk8000_base_state::color_start_r()
{
	return m_color_start;
}

void pk8000_base_state::color_start_w(uint8_t data)
{
	m_color_start = data;
}

uint8_t pk8000_base_state::color_r(offs_t offset)
{
	return m_color[offset];
}

void pk8000_base_state::color_w(offs_t offset, uint8_t data)
{
	m_color[offset] = data;
}

static constexpr rgb_t pk8000_pens[16] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0x00 }, // 1
	{ 0x00, 0xc0, 0x00 }, // 2
	{ 0x00, 0xff, 0x00 }, // 3
	{ 0x00, 0x00, 0xc0 }, // 4
	{ 0x00, 0x00, 0xff }, // 5
	{ 0x00, 0xc0, 0xc0 }, // 6
	{ 0x00, 0xff, 0xff }, // 7
	{ 0xc0, 0x00, 0x00 }, // 8
	{ 0xff, 0x00, 0x00 }, // 9
	{ 0xc0, 0xc0, 0x00 }, // A
	{ 0xff, 0xff, 0x00 }, // B
	{ 0xc0, 0x00, 0xc0 }, // C
	{ 0xff, 0x00, 0xff }, // D
	{ 0xc0, 0xc0, 0xc0 }, // E
	{ 0xff, 0xff, 0xff }  // F
};

void pk8000_base_state::pk8000_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, pk8000_pens);
}

uint8_t pk8000_base_state::_84_porta_r()
{
	return m_video_mode;
}

void pk8000_base_state::_84_porta_w(uint8_t data)
{
	m_video_mode = data;
}

void pk8000_base_state::_84_portc_w(uint8_t data)
{
	m_video_enable = BIT(data, 4);
}

uint32_t pk8000_base_state::video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *videomem)
{
	uint16_t offset = (m_video_mode & 0xc0) << 8;
	rectangle my_rect;
	my_rect.set(0, 256+32-1, 0, 192+32-1);

	if (m_video_enable) {
		bitmap.fill((m_video_color >> 4) & 0x0f, my_rect);

		if (BIT(m_video_mode,4)==0){
			// Text mode
			if (BIT(m_video_mode,5)==0){
				// 32 columns
				for (int y = 0; y < 24; y++)
				{
					for (int x = 0; x < 32; x++)
					{
						uint8_t chr  = videomem[x +(y*32) + ((m_text_start & 0x0f) << 10)+offset] ;
						uint8_t color= m_color[chr>>3];
						for (int j = 0; j < 8; j++) {
							uint8_t code = videomem[((chr<<3) + j) + ((m_chargen_start & 0x0e) << 10)+offset];

							for (int b = 0; b < 8; b++)
							{
								uint8_t col = (code >> b) & 0x01 ? (color & 0x0f) : ((color>>4) & 0x0f);
								bitmap.pix((y*8)+j+16, x*8+(7-b)+16) =  col;
							}
						}
					}
				}
			} else {
				// 40 columns
				for (int y = 0; y < 24; y++)
				{
					for (int x = 0; x < 42; x++)
					{
						uint8_t chr = videomem[x +(y*64) + ((m_text_start & 0x0e) << 10)+offset] ;
						for (int j = 0; j < 8; j++) {
							uint8_t code = videomem[((chr<<3) + j) + ((m_chargen_start  & 0x0e) << 10)+offset];
							for (int b = 2; b < 8; b++)
							{
								uint8_t col = ((code >> b) & 0x01) ? (m_video_color) & 0x0f : (m_video_color>>4) & 0x0f;
								bitmap.pix((y*8)+j+16, x*6+(7-b)+16+8) =  col;
							}
						}
					}
				}
			}
		} else {
			//Graphics
			for (int y = 0; y < 24; y++)
			{
				uint16_t off_color = (((~m_color_start) & 0x08) << 10)+offset + ((y>>3)<<11);
				uint16_t off_code  = (((~m_video_start) & 0x08) << 10)+offset + ((y>>3)<<11);
				for (int x = 0; x < 32; x++)
				{
					uint8_t chr  = videomem[x +(y*32) + ((m_chargen_start & 0x0e) << 10)+offset] ;
					for (int j = 0; j < 8; j++) {
						uint8_t color= videomem[((chr<<3) + j)+off_color];
						uint8_t code = videomem[((chr<<3) + j)+off_code];

						for (int b = 0; b < 8; b++)
						{
							uint8_t col = (code >> b) & 0x01 ? (color & 0x0f) : ((color>>4) & 0x0f);
							bitmap.pix((y*8)+j+16, x*8+(7-b)+16) =  col;
						}
					}
				}
			}
		}
	} else {
		// Disabled video
		bitmap.fill(0, my_rect);
	}
	return 0;
}
