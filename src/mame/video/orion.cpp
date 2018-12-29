// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Orion video driver by Miodrag Milanovic

        22/04/2008 Orion Pro added
        02/04/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/orion.h"

uint32_t orion_state::screen_update_orion128(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t code1,code2,code3,code4,color,val;
	int y, x,b;
	int orionproshift = (m_orion128_video_mode & 0x10) ? 1 : 0;
	int part1addr = (3-((m_orion128_video_page & 3) | orionproshift)) * 0x4000;
	int part2addr = part1addr + 0x10000;
	int video_mode = m_orion128_video_mode & m_video_mode_mask;
	uint8_t *ram = m_ram->pointer();

	for (x = 0; x < m_orion128_video_width; x++)
	{
		for (y = 0; y < 256; y++)
		{
			code1 = ram[part1addr + y + x*256];
			code2 = ram[part2addr + y + x*256];
			code3 = ram[part1addr + y + x*256 + 0x4000];
			code4 = ram[part2addr + y + x*256 + 0x4000];
			if ((video_mode==14) || (video_mode==15)) {
				code2 = m_orionpro_pseudo_color;
			}
			color = 0;
			for (b = 7; b >= 0; b--)
			{
				switch(m_orion128_video_mode & m_video_mode_mask) {
					case 0 : color = ((code1 >> b) & 0x01) ? 10 : 0; break;
					case 1 : color = ((code1 >> b) & 0x01) ? 17 : 16; break;
					case 4 : val = (((code1 >> b) & 0x01) << 1) + ((code2 >> b) & 0x01);
								switch(val) {
								case 0 : color = 0; break; // black
								case 1 : color = 4; break; // red
								case 2 : color = 2; break; // green
								case 3 : color = 1; break; // blue
								}
								break;
					case 5 : val = (((code1 >> b) & 0x01) << 1) + ((code2 >> b) & 0x01);
								switch(val) {
								case 0 : color = 7; break; // white
								case 1 : color = 4; break; // red
								case 2 : color = 2; break; // green
								case 3 : color = 1; break; // blue
								}
								break;
					case 6 :
					case 7 :
					case 14 :
					case 15 :
								color = ((code1 >> b) & 0x01) ? (code2 & 0x0f) : (code2 >> 4); break;

					default:
						switch(m_orion128_video_mode & m_video_mode_mask & 20) {
							case 16 :
										color = (((code1 >> b) & 0x01) << 2) + (((code3 >> b) & 0x01) << 1) + ((code2 >> b) & 0x01);
										break;
							case 20 :
										color = (((code1 >> b) & 0x01) << 2) + (((code3 >> b) & 0x01) << 1) + ((code2 >> b) & 0x01);
										if ((((code4 >> b) & 0x01)==1) && (color!=0)) {
										color += 8;
										}
										break;
						}
				}
				bitmap.pix16(y, x*8+(7-b)) = color;
			}
		}
	}

	return 0;
}

static constexpr rgb_t orion128_pens[18] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0xc0 }, // 1
	{ 0x00, 0xc0, 0x00 }, // 2
	{ 0x00, 0xc0, 0xc0 }, // 3
	{ 0xc0, 0x00, 0x00 }, // 4
	{ 0xc0, 0x00, 0xc0 }, // 5
	{ 0xc0, 0xc0, 0x00 }, // 6
	{ 0xc0, 0xc0, 0xc0 }, // 7
	{ 0x80, 0x80, 0x80 }, // 8
	{ 0x00, 0x00, 0xff }, // 9
	{ 0x00, 0xff, 0x00 }, // A
	{ 0x00, 0xff, 0xff }, // B
	{ 0xff, 0x00, 0x00 }, // C
	{ 0xff, 0x00, 0xff }, // D
	{ 0xff, 0xff, 0x00 }, // E
	{ 0xff, 0xff, 0xff }, // F
	{ 0xc8, 0xb4, 0x28 }, // 10
	{ 0x32, 0xfa, 0xfa }  // 11
};

void orion_state::orion128_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, orion128_pens);
}
