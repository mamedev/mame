/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "driver.h"
#include "includes/nitedrvr.h"

UINT8 *nitedrvr_hvc;

static tilemap_t *bg_tilemap;

WRITE8_HANDLER( nitedrvr_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( nitedrvr_hvc_w )
{
	nitedrvr_hvc[offset & 0x3f] = data;

	if ((offset & 0x30) == 0x30)
		watchdog_reset_w(space, 0, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = machine->generic.videoram.u8[tile_index] & 0x3f;

	SET_TILE_INFO(0, code, 0, 0);
}



VIDEO_START( nitedrvr )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_box(bitmap_t *bitmap, int bx, int by, int ex, int ey )
{
	int x, y;

	for (y = by; y < ey; y++)
	{
		for (x = bx; x < ex; x++)
			if ((y < 256) && (x < 256))
				*BITMAP_ADDR16(bitmap, y, x) = 1;
	}

	return;
}

static void draw_roadway(bitmap_t *bitmap)
{
	int roadway;

	for (roadway = 0; roadway < 16; roadway++)
	{
		int bx, by, ex, ey;

		bx = nitedrvr_hvc[roadway];
		by = nitedrvr_hvc[roadway + 16];
		ex = bx + ((nitedrvr_hvc[roadway + 32] & 0xf0) >> 4);
		ey = by + (16 - (nitedrvr_hvc[roadway + 32] & 0x0f));

		draw_box(bitmap, bx, by, ex, ey);
	}
}

VIDEO_UPDATE( nitedrvr )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_roadway(bitmap);
	return 0;
}
