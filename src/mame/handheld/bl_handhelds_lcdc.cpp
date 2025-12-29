// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "bl_handhelds_lcdc.h"

DEFINE_DEVICE_TYPE(BL_HANDHELDS_LCDC, bl_handhelds_lcdc_device, "blhandheldlcdc", "BaoBaoLong Handhelds LCD Controller")

bl_handhelds_lcdc_device::bl_handhelds_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BL_HANDHELDS_LCDC, tag, owner, clock)
{
}

u32 bl_handhelds_lcdc_device::render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 128; y++)
	{
		u32* dst = &bitmap.pix(y);

		for (int x = 0; x < 160; x++)
		{
			int count = (y * 0x200) + x;

			u16 dat = m_displaybuffer[(count * 2) + 1] | (m_displaybuffer[(count * 2) + 0] << 8);

			int b = ((dat >> 0) & 0x1f) << 3;
			int g = ((dat >> 5) & 0x3f) << 2;
			int r = ((dat >> 11) & 0x1f) << 3;

			dst[x] = (r << 16) | (g << 8) | (b << 0);
		}
	}

	return 0;
}


void bl_handhelds_lcdc_device::lcdc_command_w(u8 data)
{
	m_command = data;
	m_commandstep = 0;

	if (m_command == 0x2c)
	{
		m_posx = m_posminx << 1;
		m_posy = m_posminy;
	}
}

u8 bl_handhelds_lcdc_device::lcdc_data_r()
{
	return 0;
}

void bl_handhelds_lcdc_device::lcdc_data_w(u8 data)
{
	if (m_command == 0x2b)
	{
		switch (m_commandstep)
		{
		case 0: m_posminy = data << 8 | (m_posminy & 0xff); break;
		case 1: m_posminy = (m_posminy & 0xff00) | data; break;
		case 2: m_posmaxy = data << 8 | (m_posmaxy & 0xff); break;
		case 3: m_posmaxy = (m_posmaxy & 0xff00) | data; break;
		}
		m_commandstep++;
	}
	else if (m_command == 0x2a)
	{
		switch (m_commandstep)
		{
		case 0: m_posminx = data << 8 | (m_posminx & 0xff); break;
		case 1: m_posminx = (m_posminx & 0xff00) | data; break;
		case 2: m_posmaxx = data << 8 | (m_posmaxx & 0xff); break;
		case 3: m_posmaxx = (m_posmaxx & 0xff00) | data; break;
		}
		m_commandstep++;
	}
	else if (m_command == 0x2c)
	{
		m_displaybuffer[((m_posx + (m_posy * 0x400))) & 0x1ffff] = data;

		m_posx++;
		if (m_posx > ((m_posmaxx << 1) + 1))
		{
			m_posx = m_posminx << 1;
			m_posy++;

			if (m_posy > m_posmaxy)
			{
				m_posy = m_posminy;
			}
		}
	}
}

void bl_handhelds_lcdc_device::device_start()
{
	std::fill(std::begin(m_displaybuffer), std::end(m_displaybuffer), 0);
	m_posx = 0;
	m_posy = 0;
	m_posminx = 0;
	m_posmaxx = 0;
	m_posminy = 0;
	m_posmaxy = 0;
	m_command = 0;
	m_commandstep = 0;

	save_item(NAME(m_displaybuffer));
	save_item(NAME(m_posx));
	save_item(NAME(m_posy));
	save_item(NAME(m_posminx));
	save_item(NAME(m_posmaxx));
	save_item(NAME(m_posminy));
	save_item(NAME(m_posmaxy));
	save_item(NAME(m_command));
	save_item(NAME(m_commandstep));
}

void bl_handhelds_lcdc_device::device_reset()
{
}
