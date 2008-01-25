/***************************************************************************

Atari Tank 8 video emulation

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "includes/tank8.h"


UINT8* tank8_video_ram;
UINT8* tank8_pos_h_ram;
UINT8* tank8_pos_v_ram;
UINT8* tank8_pos_d_ram;

static tilemap* tilemap1;
static tilemap* tilemap2;

static mame_bitmap* helper1;
static mame_bitmap* helper2;
static mame_bitmap* helper3;



WRITE8_HANDLER( tank8_video_ram_w )
{
	tank8_video_ram[offset] = data;
	tilemap_mark_tile_dirty(tilemap1, offset);
	tilemap_mark_tile_dirty(tilemap2, offset);
}



static TILE_GET_INFO( tank8_get_tile_info1 )
{
	UINT8 code = tank8_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) == 0x28)
	{
		color = 9; /* walls & mines */
	}
	else
	{
		if (tile_index & 0x010)
		{
			color |= 1;
		}
		if (code & 0x80)
		{
			color |= 2;
		}
		if (tile_index & 0x200)
		{
			color |= 4;
		}
	}

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


static TILE_GET_INFO( tank8_get_tile_info2 )
{
	UINT8 code = tank8_video_ram[tile_index];

	int color = 8;

	if ((code & 0x38) == 0x28)
	{
		if ((code & 7) != 3)
		{
			color = 1; /* walls */
		}
		else
		{
			color = 2; /* mines */
		}
	}

	SET_TILE_INFO(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


VIDEO_START( tank8 )
{
	helper1 = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	helper2 = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	helper3 = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

	tilemap1 = tilemap_create(tank8_get_tile_info1, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 32, 32);
	tilemap2 = tilemap_create(tank8_get_tile_info2, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 32, 32);

	/* VBLANK starts on scanline #256 and ends on scanline #24 */

	tilemap_set_scrolly(tilemap1, 0, 2 * 24);
	tilemap_set_scrolly(tilemap2, 0, 2 * 24);
}


static int get_x_pos(int n)
{
	return 498 - tank8_pos_h_ram[n] - 2 * (tank8_pos_d_ram[n] & 128); /* ? */
}


static int get_y_pos(int n)
{
	return 2 * tank8_pos_v_ram[n] - 62;
}


static void draw_sprites(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		UINT8 code = ~tank8_pos_d_ram[i];

		int x = get_x_pos(i);
		int y = get_y_pos(i);

		drawgfx(bitmap, machine->gfx[(code & 0x04) ? 2 : 3],
			code & 0x03,
			i,
			code & 0x10,
			code & 0x08,
			x,
			y,
			cliprect, TRANSPARENCY_PEN, 0);
	}
}


static void draw_bullets(mame_bitmap* bitmap, const rectangle* cliprect)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		rectangle rect;

		int x = get_x_pos(8 + i);
		int y = get_y_pos(8 + i);

		x -= 4; /* ? */

		rect.min_x = x;
		rect.min_y = y;
		rect.max_x = rect.min_x + 3;
		rect.max_y = rect.min_y + 4;

		if (rect.min_x < cliprect->min_x)
			rect.min_x = cliprect->min_x;
		if (rect.min_y < cliprect->min_y)
			rect.min_y = cliprect->min_y;
		if (rect.max_x > cliprect->max_x)
			rect.max_x = cliprect->max_x;
		if (rect.max_y > cliprect->max_y)
			rect.max_y = cliprect->max_y;

		fillbitmap(bitmap, i, &rect);
	}
}


static TIMER_CALLBACK( tank8_collision_callback )
{
	tank8_set_collision(param);
}


VIDEO_UPDATE( tank8 )
{
	tilemap_draw(bitmap, cliprect, tilemap1, 0, 0);

	draw_sprites(machine, bitmap, cliprect);
	draw_bullets(bitmap, cliprect);
	return 0;
}


VIDEO_EOF( tank8 )
{
	const rectangle* clip = &machine->screen[0].visarea;

	int x;
	int y;

	tilemap_draw(helper1, clip, tilemap2, 0, 0);

	fillbitmap(helper2, 8, clip);
	fillbitmap(helper3, 8, clip);

	draw_sprites(machine, helper2, clip);
	draw_bullets(helper3, clip);

	for (y = clip->min_y; y <= clip->max_y; y++)
	{
		int state = 0;

		const UINT16* p1 = BITMAP_ADDR16(helper1, y, 0);
		const UINT16* p2 = BITMAP_ADDR16(helper2, y, 0);
		const UINT16* p3 = BITMAP_ADDR16(helper3, y, 0);

		if (y % 2 != cpu_getcurrentframe() % 2)
		{
			continue; /* video display is interlaced */
		}

		for (x = clip->min_x; x <= clip->max_x; x++)
		{
			UINT8 index;

			if (p1[x] == 8)
			{
				state = 0; continue; /* neither wall nor mine */
			}
			if (p2[x] == 8 && p3[x] == 8)
			{
				state = 0; continue; /* neither tank nor bullet */
			}
			if (p3[x] != 8 && p1[x] == 2)
			{
				state = 0; continue; /* bullets cannot hit mines */
			}

			if (state)
			{
				continue;
			}

			if (p3[x] != 8)
			{
				index = p3[x] | 0x18;

				if (1)
				{
					index |= 0x20;
				}
				if (0)
				{
					index |= 0x40;
				}
				if (1)
				{
					index |= 0x80;
				}
			}
			else
			{
				index = p2[x] | 0x10;

				if (p1[x] == 1)
				{
					index |= 0x20;
				}
				if (y - get_y_pos(p2[x]) >= 8)
				{
					index |= 0x40; /* collision on bottom side */
				}
				if (x - get_x_pos(p2[x]) >= 8)
				{
					index |= 0x80; /* collision on right side */
				}
			}

			timer_set(video_screen_get_time_until_pos(0, y, x), NULL, index, tank8_collision_callback);

			state = 1;
		}
	}
}
