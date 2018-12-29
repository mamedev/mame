// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Specialist video driver by Miodrag Milanovic

        15/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/special.h"


uint32_t special_state::screen_update_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = 0; x < 48; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			uint8_t const code = m_p_videoram[y + x*256];
			for (int b = 7; b >= 0; b--)
				bitmap.pix16(y, x*8+(7-b)) = BIT(code, b);
		}
	}
	return 0;
}

uint32_t special_state::screen_update_specialp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			uint8_t const code = m_p_videoram[y + x*256];
			for (int b = 7; b >= 0; b--)
				bitmap.pix16(y, x*8+(7-b)) = BIT(code, b);
		}
	}
	return 0;
}


static constexpr rgb_t specimx_pens[16] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0xaa }, // 1
	{ 0x00, 0xaa, 0x00 }, // 2
	{ 0x00, 0xaa, 0xaa }, // 3
	{ 0xaa, 0x00, 0x00 }, // 4
	{ 0xaa, 0x00, 0xaa }, // 5
	{ 0xaa, 0x55, 0x00 }, // 6
	{ 0xaa, 0xaa, 0xaa }, // 7
	{ 0x55, 0x55, 0x55 }, // 8
	{ 0x55, 0x55, 0xff }, // 9
	{ 0x55, 0xff, 0x55 }, // A
	{ 0x55, 0xff, 0xff }, // B
	{ 0xff, 0x55, 0x55 }, // C
	{ 0xff, 0x55, 0xff }, // D
	{ 0xff, 0xff, 0x55 }, // E
	{ 0xff, 0xff, 0xff }  // F
};

void special_state::specimx_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, specimx_pens);
}


VIDEO_START_MEMBER(special_state,specimx)
{
	m_specimx_colorram = std::make_unique<uint8_t[]>(0x3000);
}

uint32_t special_state::screen_update_specimx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = 0; x < 48; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			uint8_t const code = m_ram->pointer()[0x9000 + y + x*256];
			uint8_t const color = m_specimx_colorram[y + x*256];
			for (int b = 7; b >= 0; b--)
				bitmap.pix16(y, x*8+(7-b)) = (BIT(code, b) ? (color >> 4) : color) & 0x0f;
		}
	}
	return 0;
}

static constexpr rgb_t erik_pens[8] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0xff }, // 1
	{ 0xff, 0x00, 0x00 }, // 2
	{ 0xff, 0x00, 0xff }, // 3
	{ 0x00, 0xff, 0x00 }, // 4
	{ 0x00, 0xff, 0xff }, // 5
	{ 0xff, 0xff, 0x00 }, // 6
	{ 0xff, 0xff, 0xff }  // 7
};

void special_state::erik_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, erik_pens);
}


uint32_t special_state::screen_update_erik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t code1, code2, color1, color2;
	int y, x, b;
	uint8_t *erik_video_ram_p1, *erik_video_ram_p2;

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
