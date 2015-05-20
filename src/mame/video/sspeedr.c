// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Taito Super Speed Race video emulation

***************************************************************************/

#include "emu.h"
#include "includes/sspeedr.h"


WRITE8_MEMBER(sspeedr_state::sspeedr_driver_horz_w)
{
	m_driver_horz = (m_driver_horz & 0x100) | data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_driver_horz_2_w)
{
	m_driver_horz = (m_driver_horz & 0xff) | ((data & 1) << 8);
}


WRITE8_MEMBER(sspeedr_state::sspeedr_driver_vert_w)
{
	m_driver_vert = data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_driver_pic_w)
{
	m_driver_pic = data & 0x1f;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_drones_horz_w)
{
	m_drones_horz = (m_drones_horz & 0x100) | data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_drones_horz_2_w)
{
	m_drones_horz = (m_drones_horz & 0xff) | ((data & 1) << 8);
}


WRITE8_MEMBER(sspeedr_state::sspeedr_drones_mask_w)
{
	m_drones_mask = data & 0x3f;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_drones_vert_w)
{
	m_drones_vert[offset] = data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_track_horz_w)
{
	m_track_horz = (m_track_horz & 0x100) | data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_track_horz_2_w)
{
	m_track_horz = (m_track_horz & 0xff) | ((data & 1) << 8);
}


WRITE8_MEMBER(sspeedr_state::sspeedr_track_vert_w)
{
	m_track_vert[offset] = data & 0x7f;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_track_ice_w)
{
	m_track_ice = data & 0x07;
}


void sspeedr_state::draw_track(bitmap_ind16 &bitmap)
{
	const UINT8* p = memregion("gfx3")->base();

	int x;
	int y;

	for (x = 0; x < 376; x++)
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

		y = 0;

		/* upper landscape */

		for (; y < m_track_vert[0]; y++)
		{
			unsigned counter_y = y - m_track_vert[0];

			int offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				bitmap.pix16(y, x) = p[offset] / 16;
			}
			else
			{
				bitmap.pix16(y, x) = p[offset] % 16;
			}
		}

		/* street */

		for (; y < 128 + m_track_vert[1]; y++)
		{
			bitmap.pix16(y, x) = flag ? 15 : 0;
		}

		/* lower landscape */

		for (; y < 248; y++)
		{
			unsigned counter_y = y - m_track_vert[1];

			int offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				bitmap.pix16(y, x) = p[offset] / 16;
			}
			else
			{
				bitmap.pix16(y, x) = p[offset] % 16;
			}
		}
	}
}


void sspeedr_state::draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const UINT8 code[6] =
	{
		0xf, 0x4, 0x3, 0x9, 0x7, 0xc
	};

	int i;

	for (i = 0; i < 6; i++)
	{
		int x;
		int y;

		if ((m_drones_mask >> i) & 1)
		{
			continue;
		}

		x = (code[i] << 5) - m_drones_horz - 0x50;

		if (x <= -32)
		{
			x += 0x1c8;
		}

		y = 0xf0 - m_drones_vert[i >> 1];


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
	int x;
	int y;

	if (!(m_driver_pic & 0x10))
	{
		return;
	}

	x = 0x1e0 - m_driver_horz - 0x50;

	if (x <= -32)
	{
		x += 0x1c8;
	}

	y = 0xf0 - m_driver_vert;


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
}


UINT32 sspeedr_state::screen_update_sspeedr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_track(bitmap);
	draw_drones(bitmap, cliprect);
	draw_driver(bitmap, cliprect);
	return 0;
}


void sspeedr_state::screen_eof_sspeedr(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_toggle ^= 1;
	}
}
