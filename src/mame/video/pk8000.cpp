// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#include "emu.h"
#include "includes/pk8000.h"

READ8_MEMBER(pk8000_base_state::pk8000_video_color_r)
{
	return m_pk8000_video_color;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_video_color_w)
{
	m_pk8000_video_color = data;
}

READ8_MEMBER(pk8000_base_state::pk8000_text_start_r)
{
	return m_pk8000_text_start;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_text_start_w)
{
	m_pk8000_text_start = data;
}

READ8_MEMBER(pk8000_base_state::pk8000_chargen_start_r)
{
	return m_pk8000_chargen_start;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_chargen_start_w)
{
	m_pk8000_chargen_start = data;
}

READ8_MEMBER(pk8000_base_state::pk8000_video_start_r)
{
	return m_pk8000_video_start;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_video_start_w)
{
	m_pk8000_video_start = data;
}

READ8_MEMBER(pk8000_base_state::pk8000_color_start_r)
{
	return m_pk8000_color_start;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_color_start_w)
{
	m_pk8000_color_start = data;
}

READ8_MEMBER(pk8000_base_state::pk8000_color_r)
{
	return m_pk8000_color[offset];
}

WRITE8_MEMBER(pk8000_base_state::pk8000_color_w)
{
	m_pk8000_color[offset] = data;
}

static const rgb_t pk8000_palette[16] = {
	rgb_t(0x00, 0x00, 0x00), // 0
	rgb_t(0x00, 0x00, 0x00), // 1
	rgb_t(0x00, 0xc0, 0x00), // 2
	rgb_t(0x00, 0xff, 0x00), // 3
	rgb_t(0x00, 0x00, 0xc0), // 4
	rgb_t(0x00, 0x00, 0xff), // 5
	rgb_t(0x00, 0xc0, 0xc0), // 6
	rgb_t(0x00, 0xff, 0xff), // 7
	rgb_t(0xc0, 0x00, 0x00), // 8
	rgb_t(0xff, 0x00, 0x00), // 9
	rgb_t(0xc0, 0xc0, 0x00), // A
	rgb_t(0xff, 0xff, 0x00), // B
	rgb_t(0xc0, 0x00, 0xc0), // C
	rgb_t(0xff, 0x00, 0xff), // D
	rgb_t(0xc0, 0xc0, 0xc0), // E
	rgb_t(0xff, 0xff, 0xff), // F
};

PALETTE_INIT_MEMBER(pk8000_base_state, pk8000)
{
	palette.set_pen_colors(0, pk8000_palette, ARRAY_LENGTH(pk8000_palette));
}

READ8_MEMBER(pk8000_base_state::pk8000_84_porta_r)
{
	return m_pk8000_video_mode;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_84_porta_w)
{
	m_pk8000_video_mode = data;
}

WRITE8_MEMBER(pk8000_base_state::pk8000_84_portc_w)
{
	m_pk8000_video_enable = BIT(data,4);
}

UINT32 pk8000_base_state::pk8000_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *videomem)
{
	int x,y,j,b;
	UINT16 offset = (m_pk8000_video_mode & 0xc0) << 8;
	rectangle my_rect;
	my_rect.set(0, 256+32-1, 0, 192+32-1);

	if (m_pk8000_video_enable) {
		bitmap.fill((m_pk8000_video_color >> 4) & 0x0f, my_rect);

		if (BIT(m_pk8000_video_mode,4)==0){
			// Text mode
			if (BIT(m_pk8000_video_mode,5)==0){
				// 32 columns
				for (y = 0; y < 24; y++)
				{
					for (x = 0; x < 32; x++)
					{
						UINT8 chr  = videomem[x +(y*32) + ((m_pk8000_text_start & 0x0f) << 10)+offset] ;
						UINT8 color= m_pk8000_color[chr>>3];
						for (j = 0; j < 8; j++) {
							UINT8 code = videomem[((chr<<3) + j) + ((m_pk8000_chargen_start & 0x0e) << 10)+offset];

							for (b = 0; b < 8; b++)
							{
								UINT8 col = (code >> b) & 0x01 ? (color & 0x0f) : ((color>>4) & 0x0f);
								bitmap.pix16((y*8)+j+16, x*8+(7-b)+16) =  col;
							}
						}
					}
				}
			} else {
				// 40 columns
				for (y = 0; y < 24; y++)
				{
					for (x = 0; x < 42; x++)
					{
						UINT8 chr = videomem[x +(y*64) + ((m_pk8000_text_start & 0x0e) << 10)+offset] ;
						for (j = 0; j < 8; j++) {
							UINT8 code = videomem[((chr<<3) + j) + ((m_pk8000_chargen_start  & 0x0e) << 10)+offset];
							for (b = 2; b < 8; b++)
							{
								UINT8 col = ((code >> b) & 0x01) ? (m_pk8000_video_color) & 0x0f : (m_pk8000_video_color>>4) & 0x0f;
								bitmap.pix16((y*8)+j+16, x*6+(7-b)+16+8) =  col;
							}
						}
					}
				}
			}
		} else {
			//Graphics
			for (y = 0; y < 24; y++)
			{
				UINT16 off_color = (((~m_pk8000_color_start) & 0x08) << 10)+offset + ((y>>3)<<11);
				UINT16 off_code  = (((~m_pk8000_video_start) & 0x08) << 10)+offset + ((y>>3)<<11);
				for (x = 0; x < 32; x++)
				{
					UINT8 chr  = videomem[x +(y*32) + ((m_pk8000_chargen_start & 0x0e) << 10)+offset] ;
					for (j = 0; j < 8; j++) {
						UINT8 color= videomem[((chr<<3) + j)+off_color];
						UINT8 code = videomem[((chr<<3) + j)+off_code];

						for (b = 0; b < 8; b++)
						{
							UINT8 col = (code >> b) & 0x01 ? (color & 0x0f) : ((color>>4) & 0x0f);
							bitmap.pix16((y*8)+j+16, x*8+(7-b)+16) =  col;
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
