// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    NVIDIA GoForce 4500

    (c) 2010 Tim Schuerewegen

*/

#include "emu.h"
#include "video/gf4500.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( device_t &device, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		device.logerror("%s: %s", device.machine().describe_context(), buf);
	}
}

#define BIT(x,n) (((x)>>(n))&1)
#define BITS(x,m,n) (((x)>>(n))&(((UINT32)1<<((m)-(n)+1))-1))

#define GF4500_FRAMEBUF_OFFSET 0x20000


const device_type GF4500 = &device_creator<gf4500_device>;


gf4500_device::gf4500_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, GF4500, "NVIDIA GoForce 4500", tag, owner, clock, "gf4500", __FILE__), m_data(nullptr), m_screen_x(0), m_screen_y(0), m_screen_x_max(0), m_screen_y_max(0), m_screen_x_min(0), m_screen_y_min(0)
			{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gf4500_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gf4500_device::device_start()
{
	m_data = auto_alloc_array_clear(machine(), UINT32, 0x140000/4);

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


void gf4500_device::vram_write16( UINT16 data )
{
	if ((m_screen_x < m_screen_x_max) && (m_screen_y < m_screen_y_max))
	{
		UINT16 *vram = (UINT16 *)((UINT8 *)m_data + GF4500_FRAMEBUF_OFFSET + (((m_screen_y_min + m_screen_y) * (320 + 1)) + (m_screen_x_min + m_screen_x)) * 2);
		*vram = data;
		m_screen_x++;
	}
}

static rgb_t gf4500_get_color_16( UINT16 data )
{
	UINT8 r, g, b;
	r = BITS(data, 15, 11) << 3;
	g = BITS(data, 10, 5) << 2;
	b = BITS(data, 4, 0) << 3;
	return rgb_t(r, g, b);
}

UINT32 gf4500_device::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT16 *vram = (UINT16 *)(m_data + GF4500_FRAMEBUF_OFFSET / 4);
	int x, y;
	for (y = 0; y < 240; y++)
	{
		UINT32 *scanline = &bitmap.pix32(y);
		for (x = 0; x < 320; x++)
		{
			*scanline++ = gf4500_get_color_16(*vram++);
		}
		vram += 1;
	}
	return 0;
}

READ32_MEMBER( gf4500_device::read )
{
	UINT32 data = m_data[offset];
	switch (offset)
	{
		case 0x4c / 4:
			data = 0x00145000;
			break;
	}
	if ((offset < (GF4500_FRAMEBUF_OFFSET / 4)) || (offset >= ((GF4500_FRAMEBUF_OFFSET + (321 * 240 * 2)) / 4)))
	{
		verboselog( *this, 9, "(GFO) %08X -> %08X\n", 0x34000000 + (offset << 2), data);
	}
	return data;
}

WRITE32_MEMBER( gf4500_device::write )
{
	COMBINE_DATA(&m_data[offset]);
	if ((offset < (GF4500_FRAMEBUF_OFFSET / 4)) || (offset >= ((GF4500_FRAMEBUF_OFFSET + (321 * 240 * 2)) / 4)))
	{
		verboselog( *this, 9, "(GFO) %08X <- %08X\n", 0x34000000 + (offset << 2), data);
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
