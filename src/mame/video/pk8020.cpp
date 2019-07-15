// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "includes/pk8020.h"

void pk8020_state::video_start()
{
	m_color = 0;
	m_text_attr = 0;

	save_item(NAME(m_color));
	save_item(NAME(m_video_page));
	save_item(NAME(m_wide));
	save_item(NAME(m_font));
	save_item(NAME(m_attr));
	save_item(NAME(m_text_attr));
	save_item(NAME(m_video_page_access));
}

void pk8020_state::color_w(uint8_t data)
{
	// Color
	m_color = data;
}

void pk8020_state::palette_w(uint8_t data)
{
	// Palette set
	uint8_t number = data & 0x0f;
	uint8_t color = data >> 4;
	uint8_t i = (color & 0x08) ?  0x3F : 0;
	uint8_t r = ((color & 0x04) ? 0xC0 : 0) + i;
	uint8_t g = ((color & 0x02) ? 0xC0 : 0) + i;
	uint8_t b = ((color & 0x01) ? 0xC0 : 0) + i;
	m_palette->set_pen_color( number, rgb_t(r,g,b) );
}

void pk8020_state::video_page_w(uint8_t data)
{
	m_video_page_access =(data>>6) & 3;
	m_attr = (data >> 4) & 3;
	m_wide = (data >> 3) & 1;
	m_font = (data >> 2) & 1;
	m_video_page = (data & 3);
}

uint8_t pk8020_state::text_r(offs_t offset)
{
	if (m_attr == 3) m_text_attr=m_ram->pointer()[0x40400+offset];
	return m_ram->pointer()[0x40000+offset];
}

void pk8020_state::text_w(offs_t offset, uint8_t data)
{
	uint8_t *ram = m_ram->pointer();
	ram[0x40000+offset] = data;
	switch (m_attr) {
		case 0: break;
		case 1: ram[0x40400+offset]=0x01;break;
		case 2: ram[0x40400+offset]=0x00;break;
		case 3: ram[0x40400+offset]=m_text_attr;break;
	}
}

uint8_t pk8020_state::gzu_r(offs_t offset)
{
	uint8_t *addr = m_ram->pointer() + 0x10000 + (m_video_page_access * 0xC000);
	uint8_t p0 = addr[offset];
	uint8_t p1 = addr[offset + 0x4000];
	uint8_t p2 = addr[offset + 0x8000];
	uint8_t retVal = 0;
	if(m_color & 0x80) {
		// Color mode
		if (!(m_color & 0x10)) {
			p0 ^= 0xff;
		}
		if (!(m_color & 0x20)) {
			p1 ^= 0xff;
		}
		if (!(m_color & 0x40)) {
			p2 ^= 0xff;
		}
		retVal = (p0 & p1 & p2) ^ 0xff;
	} else {
		// Plane mode
		if (m_color & 0x10) {
			retVal |= p0;
		}
		if (m_color & 0x20) {
			retVal |= p1;
		}
		if (m_color & 0x40) {
			retVal |= p2;
		}
	}
	return retVal;
}

void pk8020_state::gzu_w(offs_t offset, uint8_t data)
{
	uint8_t *addr = m_ram->pointer() + 0x10000 + (m_video_page_access * 0xC000);
	uint8_t *plane_0 = addr;
	uint8_t *plane_1 = addr + 0x4000;
	uint8_t *plane_2 = addr + 0x8000;

	if(m_color & 0x80)
	{
		// Color mode
		plane_0[offset] = (plane_0[offset] & ~data) | ((m_color & 2) ? data : 0);
		plane_1[offset] = (plane_1[offset] & ~data) | ((m_color & 4) ? data : 0);
		plane_2[offset] = (plane_2[offset] & ~data) | ((m_color & 8) ? data : 0);
	} else {
		// Plane mode
		uint8_t mask = (m_color & 1) ? data : 0;
		if (!(m_color & 0x02)) {
			plane_0[offset] = (plane_0[offset] & ~data) | mask;
		}
		if (!(m_color & 0x04)) {
			plane_1[offset] = (plane_1[offset] & ~data) | mask;
		}
		if (!(m_color & 0x08)) {
			plane_2[offset] = (plane_2[offset] & ~data) | mask;
		}
	}
}

uint32_t pk8020_state::screen_update_pk8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *ram = m_ram->pointer();

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			uint8_t chr = ram[x +(y*64) + 0x40000];
			uint8_t attr= ram[x +(y*64) + 0x40400];
			for (int j = 0; j < 16; j++)
			{
				uint32_t addr = 0x10000 + x + ((y*16+j)*64) + (m_video_page * 0xC000);
				uint8_t code1 = ram[addr];
				uint8_t code2 = ram[addr + 0x4000];
				uint8_t code3 = ram[addr + 0x8000];
				uint8_t code4 = m_region_gfx1[((chr<<4) + j) + (m_font*0x1000)];
				if (attr) code4 ^= 0xff;
				for (int b = 0; b < 8; b++)
				{
					uint8_t col = (((code4 >> b) & 0x01) ? 0x08 : 0x00);
					col |= (((code3 >> b) & 0x01) ? 0x04 : 0x00);
					col |= (((code2 >> b) & 0x01) ? 0x02 : 0x00);
					col |= (((code1 >> b) & 0x01) ? 0x01 : 0x00);
					bitmap.pix16((y*16)+j, x*8+(7-b)) =  col;
				}
			}
		}
	}
	return 0;
}

void pk8020_state::pk8020_palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
		palette.set_pen_color(i, rgb_t(i * 0x10, i * 0x10, i * 0x10)); // FIXME: if this is supposed to be a 4-bit ramp it should be 0x11, not 0x10
}
