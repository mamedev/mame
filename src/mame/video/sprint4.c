/***************************************************************************

Atari Sprint 4 video emulation

***************************************************************************/

#include "driver.h"
#include "audio/sprint4.h"

static tilemap* playfield;

static mame_bitmap* helper;

int sprint4_collision[4];


PALETTE_INIT( sprint4 )
{
	palette_set_color(machine, 0, MAKE_RGB(0x00, 0x00, 0x00)); /* black  */
	palette_set_color(machine, 1, MAKE_RGB(0xfc, 0xdf, 0x80)); /* peach  */
	palette_set_color(machine, 2, MAKE_RGB(0xf0, 0x00, 0xf0)); /* violet */
	palette_set_color(machine, 3, MAKE_RGB(0x00, 0xf0, 0x0f)); /* green  */
	palette_set_color(machine, 4, MAKE_RGB(0x30, 0x4f, 0xff)); /* blue   */
	palette_set_color(machine, 5, MAKE_RGB(0xff, 0xff, 0xff)); /* white  */

	colortable[0] = 0;
	colortable[2] = 0;
	colortable[4] = 0;
	colortable[6] = 0;
	colortable[8] = 0;

	colortable[1] = 1;
	colortable[3] = 2;
	colortable[5] = 3;
	colortable[7] = 4;
	colortable[9] = 5;
}


static TILE_GET_INFO( sprint4_tile_info )
{
	UINT8 code = videoram[tile_index];

	if ((code & 0x30) == 0x30)
	{
		SET_TILE_INFO(0, code & ~0x40, (code >> 6) ^ 3, 0);
	}
	else
	{
		SET_TILE_INFO(0, code, 4, 0);
	}
}


VIDEO_START( sprint4 )
{
	helper = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

	playfield = tilemap_create(sprint4_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}


VIDEO_UPDATE( sprint4 )
{
	int i;

	tilemap_draw(bitmap, cliprect, playfield, 0, 0);

	for (i = 0; i < 4; i++)
	{
		int bank = 0;

		UINT8 horz = videoram[0x390 + 2 * i + 0];
		UINT8 attr = videoram[0x390 + 2 * i + 1];
		UINT8 vert = videoram[0x398 + 2 * i + 0];
		UINT8 code = videoram[0x398 + 2 * i + 1];

		if (i & 1)
		{
			bank = 32;
		}

		drawgfx(bitmap, machine->gfx[1],
			(code >> 3) | bank,
			(attr & 0x80) ? 4 : i,
			0, 0,
			horz - 15,
			vert - 15,
			cliprect, TRANSPARENCY_PEN, 0);
	}
	return 0;
}


VIDEO_EOF( sprint4 )
{
	UINT16 BG = machine->remapped_colortable[machine->gfx[0]->color_base];

	int i;

	/* check for sprite-playfield collisions */

	for (i = 0; i < 4; i++)
	{
		rectangle rect;

		int x;
		int y;

		int bank = 0;

		UINT8 horz = videoram[0x390 + 2 * i + 0];
		UINT8 vert = videoram[0x398 + 2 * i + 0];
		UINT8 code = videoram[0x398 + 2 * i + 1];

		rect.min_x = horz - 15;
		rect.min_y = vert - 15;
		rect.max_x = horz - 15 + machine->gfx[1]->width - 1;
		rect.max_y = vert - 15 + machine->gfx[1]->height - 1;

		sect_rect(&rect, &machine->screen[0].visarea);

		tilemap_draw(helper, &rect, playfield, 0, 0);

		if (i & 1)
		{
			bank = 32;
		}

		drawgfx(helper, machine->gfx[1],
			(code >> 3) | bank,
			4,
			0, 0,
			horz - 15,
			vert - 15,
			&rect, TRANSPARENCY_PEN, 1);

		for (y = rect.min_y; y <= rect.max_y; y++)
		{
			for (x = rect.min_x; x <= rect.max_x; x++)
			{
				if (*BITMAP_ADDR16(helper, y, x) != BG)
				{
					sprint4_collision[i] = 1;
				}
			}
		}
	}

	/* update sound status */

	discrete_sound_w(SPRINT4_MOTOR_DATA_1, videoram[0x391] & 15);
	discrete_sound_w(SPRINT4_MOTOR_DATA_2, videoram[0x393] & 15);
	discrete_sound_w(SPRINT4_MOTOR_DATA_3, videoram[0x395] & 15);
	discrete_sound_w(SPRINT4_MOTOR_DATA_4, videoram[0x397] & 15);
}


WRITE8_HANDLER( sprint4_video_ram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(playfield, offset);
}
