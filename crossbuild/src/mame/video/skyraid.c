/***************************************************************************

Atari Sky Raider video emulation

***************************************************************************/

#include "driver.h"

int skyraid_scroll;

UINT8* skyraid_alpha_num_ram;
UINT8* skyraid_pos_ram;
UINT8* skyraid_obj_ram;

static mame_bitmap *helper;


VIDEO_START( skyraid )
{
	helper = auto_bitmap_alloc(128, 240, machine->screen[0].format);
}


static void draw_text(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	const UINT8* p = skyraid_alpha_num_ram;

	int i;

	for (i = 0; i < 4; i++)
	{
		int x;
		int y;

		y = 136 + 16 * (i ^ 1);

		for (x = 0; x < bitmap->width; x += 16)
		{
			drawgfx(bitmap, machine->gfx[0], *p++, 0, 0, 0,
				x, y, cliprect, TRANSPARENCY_PEN, 0);
		}
	}
}


static void draw_terrain(running_machine *machine, mame_bitmap* bitmap, const rectangle *cliprect)
{
	const UINT8* p = memory_region(REGION_USER1);

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

			fillbitmap(bitmap, machine->pens[color], &r);

			x += 32 - count;
		}
	}
}


static void draw_sprites(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
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
		{
			drawgfx(bitmap, machine->gfx[1],
				code ^ 15, code >> 3, 0, 0,
				horz / 2, vert, cliprect, TRANSPARENCY_PEN, 2);
		}
	}
}


static void draw_missiles(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
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

		drawgfx(bitmap, machine->gfx[2],
			code ^ 15, 0, 0, 0,
			horz / 2, vert, cliprect, TRANSPARENCY_PEN, 0);
	}
}


static void draw_trapezoid(mame_bitmap* dst, mame_bitmap* src)
{
	const UINT8* p = memory_region(REGION_USER2);

	int x;
	int y;

	for (y = 0; y < dst->height; y++)
	{
		UINT16* pSrc = BITMAP_ADDR16(src, y, 0);
		UINT16* pDst = BITMAP_ADDR16(dst, y, 0);

		int x1 = 0x000 + p[(y & ~1) + 0];
		int x2 = 0x100 + p[(y & ~1) + 1];

		for (x = x1; x < x2; x++)
		{
			pDst[x] = pSrc[128 * (x - x1) / (x2 - x1)];
		}
	}
}


VIDEO_UPDATE( skyraid )
{
	fillbitmap(bitmap, machine->pens[0], cliprect);

	draw_terrain(machine, helper, cliprect);
	draw_sprites(machine, helper, cliprect);
	draw_missiles(machine, helper, cliprect);
	draw_trapezoid(bitmap, helper);
	draw_text(machine, bitmap, cliprect);
	return 0;
}
