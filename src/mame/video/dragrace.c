/***************************************************************************

Atari Drag Race video emulation

***************************************************************************/

#include "driver.h"
#include "includes/dragrace.h"

UINT8* dragrace_playfield_ram;
UINT8* dragrace_position_ram;

static tilemap* bg_tilemap;


static TILE_GET_INFO( get_tile_info )
{
	UINT8 code = dragrace_playfield_ram[tile_index];

	int num = 0;
	int col = 0;

	num = code & 0x1f;

	if ((code & 0xc0) == 0x40)
	{
		num |= 0x20;
	}

	switch (code & 0xA0)
	{
	case 0x00:
		col = 0;
		break;
	case 0x20:
		col = 1;
		break;
	case 0x80:
		col = (code & 0x40) ? 1 : 0;
		break;
	case 0xA0:
		col = (code & 0x40) ? 3 : 2;
		break;
	}

	SET_TILE_INFO(((code & 0xA0) == 0x80) ? 1 : 0, num, col, 0);
}


VIDEO_START( dragrace )
{
	bg_tilemap = tilemap_create(
		get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 16, 16);
}


VIDEO_UPDATE( dragrace )
{
	int y;

	tilemap_mark_all_tiles_dirty(bg_tilemap);

	for (y = 0; y < 256; y += 4)
	{
		rectangle rect = *cliprect;

		int xl = dragrace_position_ram[y + 0] & 15;
		int xh = dragrace_position_ram[y + 1] & 15;
		int yl = dragrace_position_ram[y + 2] & 15;
		int yh = dragrace_position_ram[y + 3] & 15;

		tilemap_set_scrollx(bg_tilemap, 0, 16 * xh + xl - 8);
		tilemap_set_scrolly(bg_tilemap, 0, 16 * yh + yl);

		if (rect.min_y < y + 0) rect.min_y = y + 0;
		if (rect.max_y > y + 3) rect.max_y = y + 3;

		tilemap_draw(bitmap, &rect, bg_tilemap, 0, 0);
	}
	return 0;
}
