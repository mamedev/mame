// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Taito Super Speed Race video emulation

***************************************************************************/

#include "emu.h"
#include "sspeedr.h"


void sspeedr_state::driver_horz_w(uint8_t data)
{
	m_driver_horz = (m_driver_horz & 0x100) | data;
}


void sspeedr_state::driver_horz_2_w(uint8_t data)
{
	m_driver_horz = (m_driver_horz & 0xff) | ((data & 1) << 8);
}


void sspeedr_state::driver_vert_w(uint8_t data)
{
	m_driver_vert = data;
}


void sspeedr_state::driver_pic_w(uint8_t data)
{
	m_driver_pic = data & 0x1f;
}


void sspeedr_state::drones_horz_w(uint8_t data)
{
	m_drones_horz = (m_drones_horz & 0x100) | data;
}


void sspeedr_state::drones_horz_2_w(uint8_t data)
{
	m_drones_horz = (m_drones_horz & 0xff) | ((data & 1) << 8);
}


void sspeedr_state::drones_mask_w(uint8_t data)
{
	m_drones_mask = data & 0x3f;
}


void sspeedr_state::drones_vert_w(offs_t offset, uint8_t data)
{
	m_drones_vert[offset] = data;
}


void sspeedr_state::track_horz_w(uint8_t data)
{
	m_track_horz = (m_track_horz & 0x100) | data;
}


void sspeedr_state::track_horz_2_w(uint8_t data)
{
	m_track_horz = (m_track_horz & 0xff) | ((data & 1) << 8);
}


void sspeedr_state::track_vert_w(offs_t offset, uint8_t data)
{
	m_track_vert[offset] = data & 0x7f;
}


void sspeedr_state::track_ice_w(uint8_t data)
{
	m_track_ice = data & 0x07;
}


void sspeedr_state::draw_track(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		unsigned counter_x = x + m_track_horz + 0x50;

		int flag = 0;

		if (m_track_ice & 2)
		{
			flag = 1;
		}
		else if (m_track_ice & 4)
		{
			if (m_track_ice & 1)
			{
				flag = (counter_x <= 0x1ff);
			}
			else
			{
				flag = (counter_x >= 0x200);
			}
		}

		if (counter_x >= 0x200)
		{
			counter_x -= 0x1c8;
		}

		int y = cliprect.min_y;

		// upper landscape
		for (; y < m_track_vert[0] && y <= cliprect.max_y; y++)
		{
			unsigned counter_y = y - m_track_vert[0];

			int offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				bitmap.pix(y, x) = m_track[offset] >> 4;
			}
			else
			{
				bitmap.pix(y, x) = m_track[offset] & 0xf;
			}
		}

		// street
		for (; y < 128 + m_track_vert[1] && y <= cliprect.max_y; y++)
		{
			bitmap.pix(y, x) = flag ? 15 : 0;
		}

		// lower landscape
		for (; y <= cliprect.max_y; y++)
		{
			unsigned counter_y = y - m_track_vert[1];

			int offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				bitmap.pix(y, x) = m_track[offset] >> 4;
			}
			else
			{
				bitmap.pix(y, x) = m_track[offset] & 0xf;
			}
		}
	}
}


void sspeedr_state::draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const uint8_t code[6] =
	{
		0xf, 0x4, 0x3, 0x9, 0x7, 0xc
	};

	for (int i = 0; i < 6; i++)
	{
		if ((m_drones_mask >> i) & 1)
		{
			continue;
		}

		int x = (code[i] << 5) - m_drones_horz - 0x50;

		if (x <= -32)
		{
			x += 0x1c8;
		}

		int y = 0xf0 - m_drones_vert[i >> 1];

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code[i] ^ m_toggle,
				0,
				0, 0,
				x,
				y, 0);
	}
}


void sspeedr_state::draw_driver(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!(m_driver_pic & 0x10))
	{
		return;
	}

	int x = 0x1e0 - m_driver_horz - 0x50;

	if (x <= -32)
	{
		x += 0x1c8;
	}

	int y = 0xf0 - m_driver_vert;

	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			m_driver_pic,
			0,
			0, 0,
			x,
			y, 0);
}


void sspeedr_state::video_start()
{
	m_toggle = 0;

	save_item(NAME(m_toggle));
	save_item(NAME(m_driver_horz));
	save_item(NAME(m_driver_vert));
	save_item(NAME(m_driver_pic));
	save_item(NAME(m_drones_horz));
	save_item(NAME(m_drones_vert));
	save_item(NAME(m_drones_mask));
	save_item(NAME(m_track_horz));
	save_item(NAME(m_track_vert));
	save_item(NAME(m_track_ice));
}


uint32_t sspeedr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_track(bitmap, cliprect);
	draw_drones(bitmap, cliprect);
	draw_driver(bitmap, cliprect);
	return 0;
}


void sspeedr_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_toggle ^= 1;
	}
}
