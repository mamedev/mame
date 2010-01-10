/***************************************************************************

Atari Sky Raider video emulation

***************************************************************************/

#include "emu.h"

int skyraid_scroll;

UINT8* skyraid_alpha_num_ram;
UINT8* skyraid_pos_ram;
UINT8* skyraid_obj_ram;

static bitmap_t *helper;


VIDEO_START( skyraid )
{
	helper = auto_bitmap_alloc(machine, 128, 240, video_screen_get_format(machine->primary_screen));
}


static void draw_text(running_machine *machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	const UINT8* p = skyraid_alpha_num_ram;

	int i;

	for (i = 0; i < 4; i++)
	{
		int x;
		int y;

		y = 136 + 16 * (i ^ 1);

		for (x = 0; x < bitmap->width; x += 16)
			drawgfx_transpen(bitmap, cliprect, machine->gfx[0], *p++, 0, 0, 0,	x, y, 0);
	}
}


static void draw_terrain(running_machine *machine, bitmap_t* bitmap, const rectangle *cliprect)
{
	const UINT8* p = memory_region(machine, "user1");

	int x;
	int y;

	for (y = 0; y < bitmap->height; y++)
	{
		int offset = (16 * skyraid_scroll + 16 * ((y + 1) / 2)) & 0x7FF;

		x = 0;

		while (x < bitmap->width)
		{
			UINT8 val = p[offset++];

			int color = val / 32;
			int count = val % 32;

			rectangle r;

			r.min_y = y;
			r.min_x = x;
			r.max_y = y + 1;
			r.max_x = x + 31 - count;

			bitmap_fill(bitmap, &r, color);

			x += 32 - count;
		}
	}
}


static void draw_sprites(running_machine *machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		int code = skyraid_obj_ram[8 + 2 * i + 0] & 15;
		int flag = skyraid_obj_ram[8 + 2 * i + 1] & 15;
		int vert = skyraid_pos_ram[8 + 2 * i + 0];
		int horz = skyraid_pos_ram[8 + 2 * i + 1];

		vert -= 31;

		if (flag & 1)
			drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
				code ^ 15, code >> 3, 0, 0,
				horz / 2, vert, 2);
	}
}


static void draw_missiles(running_machine *machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	int i;

	/* hardware is restricted to one sprite per scanline */

	for (i = 0; i < 4; i++)
	{
		int code = skyraid_obj_ram[2 * i + 0] & 15;
		int vert = skyraid_pos_ram[2 * i + 0];
		int horz = skyraid_pos_ram[2 * i + 1];

		vert -= 15;
		horz -= 31;

		drawgfx_transpen(bitmap, cliprect, machine->gfx[2],
			code ^ 15, 0, 0, 0,
			horz / 2, vert, 0);
	}
}


static void draw_trapezoid(running_machine *machine, bitmap_t* dst, bitmap_t* src)
{
	const UINT8* p = memory_region(machine, "user2");

	int x;
	int y;

	for (y = 0; y < dst->height; y++)
	{
		UINT16* pSrc = BITMAP_ADDR16(src, y, 0);
		UINT16* pDst = BITMAP_ADDR16(dst, y, 0);

		int x1 = 0x000 + p[(y & ~1) + 0];
		int x2 = 0x100 + p[(y & ~1) + 1];

		for (x = x1; x < x2; x++)
			pDst[x] = pSrc[128 * (x - x1) / (x2 - x1)];
	}
}


VIDEO_UPDATE( skyraid )
{
	bitmap_fill(bitmap, cliprect, 0);

	draw_terrain(screen->machine, helper, NULL);
	draw_sprites(screen->machine, helper, NULL);
	draw_missiles(screen->machine, helper, NULL);
	draw_trapezoid(screen->machine, bitmap, helper);
	draw_text(screen->machine, bitmap, cliprect);
	return 0;
}
