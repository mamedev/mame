// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    NVIDIA GoForce 4500

    (c) 2010 Tim Schuerewegen

*/

#include "emu.h"
#include "gf4500.h"

#define VERBOSE (0)
#include "logmacro.h"

#define GF4500_FRAMEBUF_OFFSET 0x20000


DEFINE_DEVICE_TYPE(GF4500, gf4500_device, "gf4500", "NVIDIA GoForce 4500")


gf4500_device::gf4500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GF4500, tag, owner, clock), m_data(nullptr), m_screen_x(0), m_screen_y(0), m_screen_x_max(0), m_screen_y_max(0), m_screen_x_min(0), m_screen_y_min(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gf4500_device::device_start()
{
	m_data = make_unique_clear<uint32_t[]>(0x140000/4);

	save_pointer(NAME(m_data), 0x140000/4);
	save_item(NAME(m_screen_x));
	save_item(NAME(m_screen_y));
	save_item(NAME(m_screen_x_max));
	save_item(NAME(m_screen_y_max));
	save_item(NAME(m_screen_x_min));
	save_item(NAME(m_screen_y_min));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gf4500_device::device_reset()
{
	m_screen_x = m_screen_y = 0;
	m_screen_x_max = m_screen_y_max = m_screen_x_min = m_screen_y_min = 0;
}


void gf4500_device::vram_write16( uint16_t data )
{
	if ((m_screen_x < m_screen_x_max) && (m_screen_y < m_screen_y_max))
	{
		uint16_t *vram = (uint16_t *)((uint8_t *)m_data.get() + GF4500_FRAMEBUF_OFFSET + (((m_screen_y_min + m_screen_y) * (320 + 1)) + (m_screen_x_min + m_screen_x)) * 2);
		*vram = data;
		m_screen_x++;
	}
}

static rgb_t gf4500_get_color_16( uint16_t data )
{
	uint8_t r = BIT(data, 11, 5) << 3;
	uint8_t g = BIT(data, 5, 6) << 2;
	uint8_t b = BIT(data, 0, 5) << 3;
	return rgb_t(r, g, b);
}

uint32_t gf4500_device::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *vram = (uint16_t *)(m_data.get() + GF4500_FRAMEBUF_OFFSET / 4);
	for (int y = 0; y < 240; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 320; x++)
		{
			*scanline++ = gf4500_get_color_16(*vram++);
		}
		vram += 1;
	}
	return 0;
}

uint32_t gf4500_device::read(offs_t offset)
{
	uint32_t data = m_data[offset];
	switch (offset)
	{
		case 0x4c / 4:
			data = 0x00145000;
			break;
	}
	if ((offset < (GF4500_FRAMEBUF_OFFSET / 4)) || (offset >= ((GF4500_FRAMEBUF_OFFSET + (321 * 240 * 2)) / 4)))
	{
		LOG("%s: (GFO) %08X -> %08X\n", machine().describe_context(), 0x34000000 + (offset << 2), data);
	}
	return data;
}

void gf4500_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_data[offset]);
	if ((offset < (GF4500_FRAMEBUF_OFFSET / 4)) || (offset >= ((GF4500_FRAMEBUF_OFFSET + (321 * 240 * 2)) / 4)))
	{
		LOG("%s: (GFO) %08X <- %08X\n", machine().describe_context(), 0x34000000 + (offset << 2), data);
	}
	switch (offset)
	{
		case 0x300 / 4 :
			m_screen_x = m_screen_y = 0;
			break;
		case 0x304 / 4 :
			m_screen_x_max = (data >>  0) & 0xFFFF;
			m_screen_y_max = (data >> 16) & 0xFFFF;
			if (m_screen_x_max & 1) m_screen_x_min++;
			//if (screen_y_max & 1) screen_y_min++;
			break;
		case 0x308 / 4 :
			m_screen_x_min = (data >>  0) & 0xFFFF;
			m_screen_y_min = (data >> 16) & 0xFFFF;
			if (m_screen_x_min & 1) m_screen_x_min--;
			//if (screen_y_min & 1) screen_y_min--;
			break;
	}
	if ((offset >= (0x200 / 4)) && (offset < (0x280 / 4)))
	{
// 'maincpu' (02996998): (GFO) 34000304 <- 00F00140
// 'maincpu' (029969A8): (GFO) 34000308 <- 00000000
// 'maincpu' (029969B4): (GFO) 34000324 <- 00000000
// 'maincpu' (029969C4): (GFO) 34000328 <- 40000282
// 'maincpu' (029969D4): (GFO) 34000300 <- 001022CC
//
// 'maincpu' (01DCC55C): (GFO) 34000024 -> 00000000
// 'maincpu' (02996A24): (GFO) 34000200 <- AE9FAE9F
//
// 'maincpu' (02996A24): (GFO) 3400027C <- AE9FAE9F
//
// 'maincpu' (01DCC55C): (GFO) 34000024 -> 00000000
// 'maincpu' (02996A24): (GFO) 34000200 <- AE9FAE9F
// ...
// 'maincpu' (02996A24): (GFO) 3400027C <- AE9FAE9F

		vram_write16((data >>  0) & 0xFFFF);
		vram_write16((data >> 16) & 0xFFFF);
		if (m_screen_x >= m_screen_x_max)
		{
			m_screen_x = 0;
			m_screen_y++;
		}
	}
}
