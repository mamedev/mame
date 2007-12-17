/***************************************************************************

    Atari Sprint 2 video emulation

***************************************************************************/

#include "driver.h"
#include "includes/sprint2.h"

UINT8* sprint2_video_ram;

static tilemap* bg_tilemap;
static mame_bitmap* helper;

static int collision[2];


static TILE_GET_INFO( get_tile_info )
{
	UINT8 code = sprint2_video_ram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, code >> 7, 0);
}


VIDEO_START( sprint2 )
{
	helper = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

	bg_tilemap = tilemap_create(get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 8, 32, 32);
}


READ8_HANDLER( sprint2_collision1_r )
{
	return collision[0];
}
READ8_HANDLER( sprint2_collision2_r )
{
	return collision[1];
}


WRITE8_HANDLER( sprint2_collision_reset1_w )
{
	collision[0] = 0;
}
WRITE8_HANDLER( sprint2_collision_reset2_w )
{
	collision[1] = 0;
}


WRITE8_HANDLER( sprint2_video_ram_w )
{
	sprint2_video_ram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


static UINT8 collision_check(rectangle* rect)
{
	UINT8 data = 0;

	int x;
	int y;

	for (y = rect->min_y; y <= rect->max_y; y++)
	{
		for (x = rect->min_x; x <= rect->max_x; x++)
		{
			pen_t a = *BITMAP_ADDR16(helper, y, x);

			if (a == 0)
			{
				data |= 0x40;
			}
			if (a == 3)
			{
				data |= 0x80;
			}
		}
	}

	return data;
}


static int get_sprite_code(int n)
{
	return sprint2_video_ram[0x398 + 2 * n + 1] >> 3;
}
static int get_sprite_x(int n)
{
	return 2 * (248 - sprint2_video_ram[0x390 + 1 * n]);
}
static int get_sprite_y(int n)
{
	return 1 * (248 - sprint2_video_ram[0x398 + 2 * n]);
}


VIDEO_UPDATE( sprint2 )
{
	int i;

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* draw the sprites */

	for (i = 0; i < 4; i++)
	{
		drawgfx(bitmap, machine->gfx[1],
			get_sprite_code(i),
			i,
			0, 0,
			get_sprite_x(i),
			get_sprite_y(i),
			cliprect, TRANSPARENCY_PEN, 0);
	}
	return 0;
}


VIDEO_EOF( sprint2 )
{
	int i;
	int j;

	/*
     * Collisions are detected for both player cars:
     *
     * D7 => collision w/ white playfield
     * D6 => collision w/ black playfield or another car
     *
     */

	for (i = 0; i < 2; i++)
	{
		rectangle rect;

		rect.min_x = get_sprite_x(i);
		rect.min_y = get_sprite_y(i);
		rect.max_x = get_sprite_x(i) + machine->gfx[1]->width - 1;
		rect.max_y = get_sprite_y(i) + machine->gfx[1]->height - 1;

		if (rect.min_x < machine->screen[0].visarea.min_x)
			rect.min_x = machine->screen[0].visarea.min_x;
		if (rect.min_y < machine->screen[0].visarea.min_y)
			rect.min_y = machine->screen[0].visarea.min_y;
		if (rect.max_x > machine->screen[0].visarea.max_x)
			rect.max_x = machine->screen[0].visarea.max_x;
		if (rect.max_y > machine->screen[0].visarea.max_y)
			rect.max_y = machine->screen[0].visarea.max_y;

		/* check for sprite-tilemap collisions */

		tilemap_draw(helper, &rect, bg_tilemap, 0, 0);

		drawgfx(helper, machine->gfx[1],
			get_sprite_code(i),
			0,
			0, 0,
			get_sprite_x(i),
			get_sprite_y(i),
			&rect, TRANSPARENCY_PEN, 1);

		collision[i] |= collision_check(&rect);

		/* check for sprite-sprite collisions */

		for (j = 0; j < 4; j++)
		{
			if (j != i)
			{
				drawgfx(helper, machine->gfx[1],
					get_sprite_code(j),
					1,
					0, 0,
					get_sprite_x(j),
					get_sprite_y(j),
					&rect, TRANSPARENCY_PEN, 0);
			}
		}

		drawgfx(helper, machine->gfx[1],
			get_sprite_code(i),
			0,
			0, 0,
			get_sprite_x(i),
			get_sprite_y(i),
			&rect, TRANSPARENCY_PEN, 1);

		collision[i] |= collision_check(&rect);
	}
}
