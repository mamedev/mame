// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Wolf Pack (prototype) video emulation

***************************************************************************/

#include "emu.h"
#include "includes/wolfpack.h"


PALETTE_INIT_MEMBER(wolfpack_state, wolfpack)
{
	int i;

	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(1, rgb_t(0xc1, 0xc1, 0xc1));
	palette.set_indirect_color(2, rgb_t(0x81, 0x81, 0x81));
	palette.set_indirect_color(3, rgb_t(0x48, 0x48, 0x48));

	for (i = 0; i < 4; i++)
	{
		rgb_t color = palette.indirect_color(i);

		palette.set_indirect_color(4 + i,
										rgb_t(color.r()   < 0xb8 ? color.r()   + 0x48 : 0xff,
												color.g() < 0xb8 ? color.g()  + 0x48 : 0xff,
												color.b()  < 0xb8 ? color.b()  + 0x48 : 0xff));
	}

	palette.set_pen_indirect(0x00, 0);
	palette.set_pen_indirect(0x01, 1);
	palette.set_pen_indirect(0x02, 1);
	palette.set_pen_indirect(0x03, 0);
	palette.set_pen_indirect(0x04, 0);
	palette.set_pen_indirect(0x05, 2);
	palette.set_pen_indirect(0x06, 0);
	palette.set_pen_indirect(0x07, 3);
	palette.set_pen_indirect(0x08, 4);
	palette.set_pen_indirect(0x09, 5);
	palette.set_pen_indirect(0x0a, 6);
	palette.set_pen_indirect(0x0b, 7);
}


WRITE8_MEMBER(wolfpack_state::wolfpack_ship_size_w)
{
	m_ship_size = data;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_video_invert_w)
{
	m_video_invert = data & 1;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_ship_reflect_w)
{
	m_ship_reflect = data & 1;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_pt_pos_select_w)
{
	m_pt_pos_select = data & 1;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_pt_horz_w)
{
	m_pt_horz = data;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_pt_pic_w)
{
	m_pt_pic = data & 0x3f;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_ship_h_w)
{
	m_ship_h = data;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_torpedo_pic_w)
{
	m_torpedo_pic = data;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_ship_h_precess_w)
{
	m_ship_h_precess = data & 0x3f;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_ship_pic_w)
{
	m_ship_pic = data & 0x0f;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_torpedo_h_w)
{
	m_torpedo_h = data;
}
WRITE8_MEMBER(wolfpack_state::wolfpack_torpedo_v_w)
{
	m_torpedo_v = data;
}


void wolfpack_state::video_start()
{
	UINT16 val = 0;

	int i;

	m_LFSR = auto_alloc_array(machine(), UINT8, 0x8000);

	m_screen->register_screen_bitmap(m_helper);

	for (i = 0; i < 0x8000; i++)
	{
		int bit = (val >> 0x0) ^ (val >> 0xe) ^ 1;

		val = (val << 1) | (bit & 1);

		m_LFSR[i] = (val & 0xc00) == 0xc00;
	}

	m_current_index = 0x80;
}


void wolfpack_state::draw_ship(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const UINT32 scaler[] =
	{
		0x00000, 0x00500, 0x00a00, 0x01000,
		0x01000, 0x01200, 0x01500, 0x01800,
		0x01800, 0x01d00, 0x02200, 0x02800,
		0x02800, 0x02800, 0x02800, 0x02800,
		0x02800, 0x03000, 0x03800, 0x04000,
		0x04000, 0x04500, 0x04a00, 0x05000,
		0x05000, 0x05500, 0x05a00, 0x06000,
		0x06000, 0x06a00, 0x07500, 0x08000,
		0x08000, 0x08a00, 0x09500, 0x0a000,
		0x0a000, 0x0b000, 0x0c000, 0x0d000,
		0x0d000, 0x0e000, 0x0f000, 0x10000,
		0x10000, 0x11a00, 0x13500, 0x15000,
		0x15000, 0x17500, 0x19a00, 0x1c000,
		0x1c000, 0x1ea00, 0x21500, 0x24000,
		0x24000, 0x26a00, 0x29500, 0x2c000,
		0x2c000, 0x2fa00, 0x33500, 0x37000
	};

	int chop = (scaler[m_ship_size >> 2] * m_ship_h_precess) >> 16;


		m_gfxdecode->gfx(1)->zoom_transpen(bitmap,cliprect,
		m_ship_pic,
		0,
		m_ship_reflect, 0,
		2 * (m_ship_h - chop),
		128,
		2 * scaler[m_ship_size >> 2], scaler[m_ship_size >> 2], 0);
}


void wolfpack_state::draw_torpedo(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	int x;
	int y;


		m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
		m_torpedo_pic,
		0,
		0, 0,
		2 * (244 - m_torpedo_h),
		224 - m_torpedo_v, 0);

	for (y = 16; y < 224 - m_torpedo_v; y++)
	{
		int x1;
		int x2;

		if (y % 16 == 1)
			count = (count - 1) & 7;

		x1 = 248 - m_torpedo_h - count;
		x2 = 248 - m_torpedo_h + count;

		for (x = 2 * x1; x < 2 * x2; x++)
			if (m_LFSR[(m_current_index + 0x300 * y + x) % 0x8000])
				bitmap.pix16(y, x) = 1;
	}
}


void wolfpack_state::draw_pt(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle rect = cliprect;

	if (!(m_pt_pic & 0x20))
		rect.min_x = 256;

	if (!(m_pt_pic & 0x10))
		rect.max_x = 255;


		m_gfxdecode->gfx(2)->transpen(bitmap,rect,
		m_pt_pic,
		0,
		0, 0,
		2 * m_pt_horz,
		m_pt_pos_select ? 0x70 : 0xA0, 0);


		m_gfxdecode->gfx(2)->transpen(bitmap,rect,
		m_pt_pic,
		0,
		0, 0,
		2 * m_pt_horz - 512,
		m_pt_pos_select ? 0x70 : 0xA0, 0);
}


void wolfpack_state::draw_water(palette_device &palette, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle rect = cliprect;

	int x;
	int y;

	if (rect.max_y > 127)
		rect.max_y = 127;

	for (y = rect.min_y; y <= rect.max_y; y++)
	{
		UINT16* p = &bitmap.pix16(y);

		for (x = rect.min_x; x <= rect.max_x; x++)
			p[x] = palette.pen_indirect(p[x]) | 0x08;
	}
}


UINT32 wolfpack_state::screen_update_wolfpack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	int j;

	UINT8 color = 0x48;
	if (m_ship_size & 0x10) color += 0x13;
	if (m_ship_size & 0x20) color += 0x22;
	if (m_ship_size & 0x40) color += 0x3a;
	if (m_ship_size & 0x80) color += 0x48;

	m_palette->set_indirect_color(3, rgb_t(color,color,color));
	m_palette->set_indirect_color(7, rgb_t(color < 0xb8 ? color + 0x48 : 0xff,
																			color < 0xb8 ? color + 0x48 : 0xff,
																			color < 0xb8 ? color + 0x48 : 0xff));

	bitmap.fill(m_video_invert, cliprect);

	for (i = 0; i < 8; i++)
		for (j = 0; j < 32; j++)
		{
			int code = m_alpha_num_ram[32 * i + j];


				m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
				code,
				m_video_invert,
				0, 0,
				16 * j,
				192 + 8 * i);
		}

	draw_pt(bitmap, cliprect);
	draw_ship(bitmap, cliprect);
	draw_torpedo(bitmap, cliprect);
	draw_water(*m_palette, bitmap, cliprect);
	return 0;
}


void wolfpack_state::screen_eof_wolfpack(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		int x;
		int y;

		m_helper.fill(0);

		draw_ship(m_helper, m_helper.cliprect());

		for (y = 128; y < 224 - m_torpedo_v; y++)
		{
			int x1 = 248 - m_torpedo_h - 1;
			int x2 = 248 - m_torpedo_h + 1;

			for (x = 2 * x1; x < 2 * x2; x++)
			{
				if (x < 0 || x >= m_helper.width())
					continue;
				if (y < 0 || y >= m_helper.height())
					continue;

				if (m_helper.pix16(y, x))
					m_collision = 1;
			}
		}

		m_current_index += 0x300 * 262;
	}
}
