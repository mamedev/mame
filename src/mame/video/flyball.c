/***************************************************************************

Atari Flyball video emulation

***************************************************************************/

#include "driver.h"

static tilemap* flyball_tilemap;

UINT8 flyball_pitcher_vert;
UINT8 flyball_pitcher_horz;
UINT8 flyball_pitcher_pic;
UINT8 flyball_ball_vert;
UINT8 flyball_ball_horz;

UINT8* flyball_playfield_ram;


static TILEMAP_MAPPER( flyball_get_memory_offset )
{
	if (col == 0)
	{
		col = num_cols;
	}

	return num_cols * (num_rows - row) - col;
}


static TILE_GET_INFO( flyball_get_tile_info )
{
	UINT8 data = flyball_playfield_ram[tile_index];

	int flags =
		((data & 0x40) ? TILE_FLIPX : 0) |
		((data & 0x80) ? TILE_FLIPY : 0);

	int code = data & 63;

	if ((flags & TILE_FLIPX) && (flags & TILE_FLIPY))
	{
		code += 64;
	}

	SET_TILE_INFO(0, code, 0, flags);
}


VIDEO_START( flyball )
{
	flyball_tilemap = tilemap_create(flyball_get_tile_info,
		flyball_get_memory_offset, TILEMAP_TYPE_PEN, 8, 16, 32, 16);
}


VIDEO_UPDATE( flyball )
{
	int pitcherx = flyball_pitcher_horz;
	int pitchery = flyball_pitcher_vert - 31;

	int ballx = flyball_ball_horz - 1;
	int bally = flyball_ball_vert - 17;

	int x;
	int y;

	tilemap_mark_all_tiles_dirty(flyball_tilemap);

	/* draw playfield */

	tilemap_draw(bitmap, cliprect, flyball_tilemap, 0, 0);

	/* draw pitcher */

	drawgfx(bitmap, machine->gfx[1], flyball_pitcher_pic ^ 0xf,
		0, 1, 0, pitcherx, pitchery, cliprect, TRANSPARENCY_PEN, 1);

	/* draw ball */

	for (y = bally; y < bally + 2; y++)
	{
		for (x = ballx; x < ballx + 2; x++)
		{
			if (x >= machine->screen[0].visarea.min_x &&
			    x <= machine->screen[0].visarea.max_x &&
			    y >= machine->screen[0].visarea.min_y &&
			    y <= machine->screen[0].visarea.max_y)
			{
				*BITMAP_ADDR16(bitmap, y, x) = machine->pens[1];
			}
		}
	}
	return 0;
}
