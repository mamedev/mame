/***************************************************************************

Atari Wolf Pack (prototype) video emulation

***************************************************************************/

#include "emu.h"
#include "includes/wolfpack.h"


PALETTE_INIT( wolfpack )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 8);

	colortable_palette_set_color(machine.colortable, 0, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(machine.colortable, 1, MAKE_RGB(0xc1, 0xc1, 0xc1));
	colortable_palette_set_color(machine.colortable, 2, MAKE_RGB(0x81, 0x81, 0x81));
	colortable_palette_set_color(machine.colortable, 3, MAKE_RGB(0x48, 0x48, 0x48));

	for (i = 0; i < 4; i++)
	{
		rgb_t color = colortable_palette_get_color(machine.colortable, i);

		colortable_palette_set_color(machine.colortable, 4 + i,
									 MAKE_RGB(RGB_RED(color)   < 0xb8 ? RGB_RED(color)   + 0x48 : 0xff,
											  RGB_GREEN(color) < 0xb8 ? RGB_GREEN(color) + 0x48 : 0xff,
											  RGB_BLUE(color)  < 0xb8 ? RGB_BLUE(color)  + 0x48 : 0xff));
	}

	colortable_entry_set_value(machine.colortable, 0x00, 0);
	colortable_entry_set_value(machine.colortable, 0x01, 1);
	colortable_entry_set_value(machine.colortable, 0x02, 1);
	colortable_entry_set_value(machine.colortable, 0x03, 0);
	colortable_entry_set_value(machine.colortable, 0x04, 0);
	colortable_entry_set_value(machine.colortable, 0x05, 2);
	colortable_entry_set_value(machine.colortable, 0x06, 0);
	colortable_entry_set_value(machine.colortable, 0x07, 3);
	colortable_entry_set_value(machine.colortable, 0x08, 4);
	colortable_entry_set_value(machine.colortable, 0x09, 5);
	colortable_entry_set_value(machine.colortable, 0x0a, 6);
	colortable_entry_set_value(machine.colortable, 0x0b, 7);
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


VIDEO_START( wolfpack )
{
	wolfpack_state *state = machine.driver_data<wolfpack_state>();
	UINT16 val = 0;

	int i;

	state->m_LFSR = auto_alloc_array(machine, UINT8, 0x8000);

	machine.primary_screen->register_screen_bitmap(state->m_helper);

	for (i = 0; i < 0x8000; i++)
	{
		int bit = (val >> 0x0) ^ (val >> 0xe) ^ 1;

		val = (val << 1) | (bit & 1);

		state->m_LFSR[i] = (val & 0xc00) == 0xc00;
	}

	state->m_current_index = 0x80;
}


static void draw_ship(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	wolfpack_state *state = machine.driver_data<wolfpack_state>();
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

	int chop = (scaler[state->m_ship_size >> 2] * state->m_ship_h_precess) >> 16;

	drawgfxzoom_transpen(bitmap, cliprect,
		machine.gfx[1],
		state->m_ship_pic,
		0,
		state->m_ship_reflect, 0,
		2 * (state->m_ship_h - chop),
		128,
		2 * scaler[state->m_ship_size >> 2], scaler[state->m_ship_size >> 2], 0);
}


static void draw_torpedo(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	wolfpack_state *state = machine.driver_data<wolfpack_state>();
	int count = 0;

	int x;
	int y;

	drawgfx_transpen(bitmap, cliprect,
		machine.gfx[3],
		state->m_torpedo_pic,
		0,
		0, 0,
		2 * (244 - state->m_torpedo_h),
		224 - state->m_torpedo_v, 0);

	for (y = 16; y < 224 - state->m_torpedo_v; y++)
	{
		int x1;
		int x2;

		if (y % 16 == 1)
			count = (count - 1) & 7;

		x1 = 248 - state->m_torpedo_h - count;
		x2 = 248 - state->m_torpedo_h + count;

		for (x = 2 * x1; x < 2 * x2; x++)
			if (state->m_LFSR[(state->m_current_index + 0x300 * y + x) % 0x8000])
				bitmap.pix16(y, x) = 1;
	}
}


static void draw_pt(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	wolfpack_state *state = machine.driver_data<wolfpack_state>();
	rectangle rect = cliprect;

	if (!(state->m_pt_pic & 0x20))
		rect.min_x = 256;

	if (!(state->m_pt_pic & 0x10))
		rect.max_x = 255;

	drawgfx_transpen(bitmap, rect,
		machine.gfx[2],
		state->m_pt_pic,
		0,
		0, 0,
		2 * state->m_pt_horz,
		state->m_pt_pos_select ? 0x70 : 0xA0, 0);

	drawgfx_transpen(bitmap, rect,
		machine.gfx[2],
		state->m_pt_pic,
		0,
		0, 0,
		2 * state->m_pt_horz - 512,
		state->m_pt_pos_select ? 0x70 : 0xA0, 0);
}


static void draw_water(colortable_t *colortable, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
			p[x] = colortable_entry_get_value(colortable, p[x]) | 0x08;
	}
}


SCREEN_UPDATE_IND16( wolfpack )
{
	wolfpack_state *state = screen.machine().driver_data<wolfpack_state>();
	int i;
	int j;

	UINT8 color = 0x48;
	if (state->m_ship_size & 0x10) color += 0x13;
	if (state->m_ship_size & 0x20) color += 0x22;
	if (state->m_ship_size & 0x40) color += 0x3a;
	if (state->m_ship_size & 0x80) color += 0x48;

	colortable_palette_set_color(screen.machine().colortable, 3, MAKE_RGB(color,color,color));
	colortable_palette_set_color(screen.machine().colortable, 7, MAKE_RGB(color < 0xb8 ? color + 0x48 : 0xff,
																		  color < 0xb8 ? color + 0x48 : 0xff,
																		  color < 0xb8 ? color + 0x48 : 0xff));

	bitmap.fill(state->m_video_invert, cliprect);

	for (i = 0; i < 8; i++)
		for (j = 0; j < 32; j++)
		{
			int code = state->m_alpha_num_ram[32 * i + j];

			drawgfx_opaque(bitmap, cliprect,
				screen.machine().gfx[0],
				code,
				state->m_video_invert,
				0, 0,
				16 * j,
				192 + 8 * i);
		}

	draw_pt(screen.machine(), bitmap, cliprect);
	draw_ship(screen.machine(), bitmap, cliprect);
	draw_torpedo(screen.machine(), bitmap, cliprect);
	draw_water(screen.machine().colortable, bitmap, cliprect);
	return 0;
}


SCREEN_VBLANK( wolfpack )
{
	// rising edge
	if (vblank_on)
	{
		wolfpack_state *state = screen.machine().driver_data<wolfpack_state>();

		int x;
		int y;

		state->m_helper.fill(0);

		draw_ship(screen.machine(), state->m_helper, state->m_helper.cliprect());

		for (y = 128; y < 224 - state->m_torpedo_v; y++)
		{
			int x1 = 248 - state->m_torpedo_h - 1;
			int x2 = 248 - state->m_torpedo_h + 1;

			for (x = 2 * x1; x < 2 * x2; x++)
			{
				if (x < 0 || x >= state->m_helper.width())
					continue;
				if (y < 0 || y >= state->m_helper.height())
					continue;

				if (state->m_helper.pix16(y, x))
					state->m_collision = 1;
			}
		}

		state->m_current_index += 0x300 * 262;
	}
}
