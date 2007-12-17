/***************************************************************************

    Atari Basketball hardware

***************************************************************************/

#include "driver.h"
#include "bsktball.h"

UINT8 *bsktball_motion;

static tilemap *bg_tilemap;

WRITE8_HANDLER( bsktball_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = videoram[tile_index];
	int code = ((attr & 0x0f) << 2) | ((attr & 0x30) >> 4);
	int color = (attr & 0x40) >> 6;
	int flags = (attr & 0x80) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( bsktball )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine,  mame_bitmap *bitmap, const rectangle *cliprect)
{
	int motion;

	for (motion=0;motion<16;motion++)
	{
		int pic = bsktball_motion[motion*4];
		int sy = 28*8 - bsktball_motion[motion*4 + 1];
		int sx = bsktball_motion[motion*4 + 2];
		int color = bsktball_motion[motion*4 + 3];
		int flipx = (pic & 0x80) >> 7;

		pic = (pic & 0x3F);
        color = (color & 0x3F);

        drawgfx(bitmap,machine->gfx[1],
            pic, color,
			flipx,0,sx,sy,
			cliprect,TRANSPARENCY_PEN,0);
	}
}

VIDEO_UPDATE( bsktball )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
