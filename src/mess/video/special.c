// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Specialist video driver by Miodrag Milanovic

        15/03/2008 Preliminary driver.

****************************************************************************/


#include "includes/special.h"


VIDEO_START_MEMBER(special_state,special)
{
	m_palette->set_pen_color(0,rgb_t::black); /* black */
	m_palette->set_pen_color(1,rgb_t::white); /* white */
}

UINT32 special_state::screen_update_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code;
	int y, x, b;

	for (x = 0; x < 48; x++)
	{
		for (y = 0; y < 256; y++)
		{
			code = m_p_videoram[y + x*256];
			for (b = 7; b >= 0; b--)
				bitmap.pix16(y, x*8+(7-b)) =  (code >> b) & 0x01;
		}
	}
	return 0;
}
VIDEO_START_MEMBER(special_state,specialp)
{
}

UINT32 special_state::screen_update_specialp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code;
	int y, x, b;

	for (x = 0; x < 64; x++)
	{
		for (y = 0; y < 256; y++)
		{
			code = m_p_videoram[y + x*256];
			for (b = 7; b >= 0; b--)
				bitmap.pix16(y, x*8+(7-b)) =  (code >> b) & 0x01;
		}
	}
	return 0;
}


static const rgb_t specimx_palette[16] = {
	rgb_t(0x00, 0x00, 0x00), // 0
	rgb_t(0x00, 0x00, 0xaa), // 1
	rgb_t(0x00, 0xaa, 0x00), // 2
	rgb_t(0x00, 0xaa, 0xaa), // 3
	rgb_t(0xaa, 0x00, 0x00), // 4
	rgb_t(0xaa, 0x00, 0xaa), // 5
	rgb_t(0xaa, 0x55, 0x00), // 6
	rgb_t(0xaa, 0xaa, 0xaa), // 7
	rgb_t(0x55, 0x55, 0x55), // 8
	rgb_t(0x55, 0x55, 0xff), // 9
	rgb_t(0x55, 0xff, 0x55), // A
	rgb_t(0x55, 0xff, 0xff), // B
	rgb_t(0xff, 0x55, 0x55), // C
	rgb_t(0xff, 0x55, 0xff), // D
	rgb_t(0xff, 0xff, 0x55), // E
	rgb_t(0xff, 0xff, 0xff)  // F
};

PALETTE_INIT_MEMBER(special_state,specimx)
{
	palette.set_pen_colors(0, specimx_palette, ARRAY_LENGTH(specimx_palette));
}


VIDEO_START_MEMBER(special_state,specimx)
{
	m_specimx_colorram = auto_alloc_array(machine(), UINT8, 0x3000);
}

UINT32 special_state::screen_update_specimx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code, color;
	int y, x, b;

	for (x = 0; x < 48; x++)
	{
		for (y = 0; y < 256; y++)
		{
			code = m_ram->pointer()[0x9000 + y + x*256];
			color = m_specimx_colorram[y + x*256];
			for (b = 7; b >= 0; b--)
				bitmap.pix16(y, x*8+(7-b)) =  ((code >> b) & 0x01)==0 ? color & 0x0f : (color >> 4)& 0x0f ;
		}
	}
	return 0;
}

static const rgb_t erik_palette[8] = {
	rgb_t(0x00, 0x00, 0x00), // 0
	rgb_t(0x00, 0x00, 0xff), // 1
	rgb_t(0xff, 0x00, 0x00), // 2
	rgb_t(0xff, 0x00, 0xff), // 3
	rgb_t(0x00, 0xff, 0x00), // 4
	rgb_t(0x00, 0xff, 0xff), // 5
	rgb_t(0xff, 0xff, 0x00), // 6
	rgb_t(0xff, 0xff, 0xff)  // 7
};

PALETTE_INIT_MEMBER(special_state,erik)
{
	palette.set_pen_colors(0, erik_palette, ARRAY_LENGTH(erik_palette));
}


VIDEO_START_MEMBER(special_state,erik)
{
}

UINT32 special_state::screen_update_erik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code1, code2, color1, color2;
	int y, x, b;
	UINT8 *erik_video_ram_p1, *erik_video_ram_p2;

	erik_video_ram_p1 = m_ram->pointer() + 0x9000;
	erik_video_ram_p2 = m_ram->pointer() + 0xd000;

	for (x = 0; x < 48; x++)
	{
		for (y = 0; y < 256; y++)
		{
			code1  = erik_video_ram_p1[y + x*256];
			code2  = erik_video_ram_p2[y + x*256];

			for (b = 7; b >= 0; b--)
			{
				color1 = ((code1 >> b) & 0x01)==0 ? m_erik_background : m_erik_color_1;
				color2 = ((code2 >> b) & 0x01)==0 ? m_erik_background : m_erik_color_2;
				bitmap.pix16(y, x*8+(7-b)) =  color1 | color2;
			}
		}
	}
	return 0;
}
