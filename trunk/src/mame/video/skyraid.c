/***************************************************************************

Atari Sky Raider video emulation

***************************************************************************/

#include "emu.h"
#include "includes/skyraid.h"


VIDEO_START( skyraid )
{
	skyraid_state *state = machine.driver_data<skyraid_state>();

	state->m_helper.allocate(128, 240);
}


static void draw_text(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	skyraid_state *state = machine.driver_data<skyraid_state>();
	const UINT8* p = state->m_alpha_num_ram;

	int i;

	for (i = 0; i < 4; i++)
	{
		int x;
		int y;

		y = 136 + 16 * (i ^ 1);

		for (x = 0; x < bitmap.width(); x += 16)
			drawgfx_transpen(bitmap, cliprect, machine.gfx[0], *p++, 0, 0, 0,	x, y, 0);
	}
}


static void draw_terrain(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	skyraid_state *state = machine.driver_data<skyraid_state>();
	const UINT8* p = state->memregion("user1")->base();

	int x;
	int y;

	for (y = 0; y < bitmap.height(); y++)
	{
		int offset = (16 * state->m_scroll + 16 * ((y + 1) / 2)) & 0x7FF;

		x = 0;

		while (x < bitmap.width())
		{
			UINT8 val = p[offset++];

			int color = val / 32;
			int count = val % 32;

			rectangle r(x, x + 31 - count, y, y+ 1);

			bitmap.fill(color, r);

			x += 32 - count;
		}
	}
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	skyraid_state *state = machine.driver_data<skyraid_state>();
	int i;

	for (i = 0; i < 4; i++)
	{
		int code = state->m_obj_ram[8 + 2 * i + 0] & 15;
		int flag = state->m_obj_ram[8 + 2 * i + 1] & 15;
		int vert = state->m_pos_ram[8 + 2 * i + 0];
		int horz = state->m_pos_ram[8 + 2 * i + 1];

		vert -= 31;

		if (flag & 1)
			drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
				code ^ 15, code >> 3, 0, 0,
				horz / 2, vert, 2);
	}
}


static void draw_missiles(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	skyraid_state *state = machine.driver_data<skyraid_state>();
	int i;

	/* hardware is restricted to one sprite per scanline */

	for (i = 0; i < 4; i++)
	{
		int code = state->m_obj_ram[2 * i + 0] & 15;
		int vert = state->m_pos_ram[2 * i + 0];
		int horz = state->m_pos_ram[2 * i + 1];

		vert -= 15;
		horz -= 31;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[2],
			code ^ 15, 0, 0, 0,
			horz / 2, vert, 0);
	}
}


static void draw_trapezoid(running_machine &machine, bitmap_ind16& dst, bitmap_ind16& src)
{
	const UINT8* p = machine.root_device().memregion("user2")->base();

	int x;
	int y;

	for (y = 0; y < dst.height(); y++)
	{
		UINT16* pSrc = &src.pix16(y);
		UINT16* pDst = &dst.pix16(y);

		int x1 = 0x000 + p[(y & ~1) + 0];
		int x2 = 0x100 + p[(y & ~1) + 1];

		for (x = x1; x < x2; x++)
			pDst[x] = pSrc[128 * (x - x1) / (x2 - x1)];
	}
}


SCREEN_UPDATE_IND16( skyraid )
{
	skyraid_state *state = screen.machine().driver_data<skyraid_state>();

	bitmap.fill(0, cliprect);

	rectangle helper_clip = cliprect;
	helper_clip &= state->m_helper.cliprect();

	draw_terrain(screen.machine(), state->m_helper, helper_clip);
	draw_sprites(screen.machine(), state->m_helper, helper_clip);
	draw_missiles(screen.machine(), state->m_helper, helper_clip);
	draw_trapezoid(screen.machine(), bitmap, state->m_helper);
	draw_text(screen.machine(), bitmap, cliprect);
	return 0;
}
