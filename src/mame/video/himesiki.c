/******************************************************************************

Himeshikibu (C) 1989 Hi-Soft

Video hardware
    driver by Uki

******************************************************************************/

#include "driver.h"
#include "video/generic.h"

UINT8 *himesiki_bg_ram;
static tilemap *himesiki_bg_tilemap;

static int himesiki_scrollx[2];
static int himesiki_flip;


static TILE_GET_INFO( get_bg_tile_info )
{
	int code = himesiki_bg_ram[tile_index*2] + himesiki_bg_ram[tile_index*2+1]*0x100 ;
	int col = code >> 12;

	code &= 0xfff;

	SET_TILE_INFO(0, code, col, 0);
}

VIDEO_START( himesiki )
{
	himesiki_bg_ram = auto_alloc_array(machine, UINT8, 0x1000);
	himesiki_bg_tilemap = tilemap_create( machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,32 );
}

WRITE8_HANDLER( himesiki_bg_ram_w )
{
	himesiki_bg_ram[offset] = data;
	tilemap_mark_tile_dirty( himesiki_bg_tilemap, offset / 2 );
}

WRITE8_HANDLER( himesiki_scrollx_w )
{
	himesiki_scrollx[offset] = data;
}

WRITE8_HANDLER( himesiki_flip_w )
{
	himesiki_flip = data & 0xc0;
	flip_screen_set(space->machine, himesiki_flip);

	if (data & 0x3f)
		logerror("p08_w %02x\n",data);
}

static void himesiki_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0x100; offs<0x160; offs+=4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 0] | (attr & 3) << 8;
		int x = spriteram[offs + 3] | (attr & 8) << 5;
		int y = spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int fx = attr & 4;
		int fy = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (himesiki_flip)
		{
			y = (y + 33) & 0xff;
			x = 224-x;
			fx ^= 4;
			fy = 1;
		}
		else
		{
			y = 257-y;
			if (y > 0xc0)
				y -= 0x100;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],code,col,fx,fy,x,y,15);
	}

	for (offs = 0; offs<0x100; offs+=4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 0] | (attr & 7) << 8;
		int x = spriteram[offs + 3] | (attr & 8) << 5;
		int y = spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int f = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (himesiki_flip)
		{
			y += 49;
			x = 240-x;
			f = 1;
		}
		else
			y = 257-y;

		y &= 0xff;
		if (y > 0xf0)
			y -= 0x100;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],code,col,f,f,x,y,15);
	}
}

VIDEO_UPDATE( himesiki )
{
	int x = -(himesiki_scrollx[0] << 8 | himesiki_scrollx[1]) & 0x1ff;
	tilemap_set_scrolldx( himesiki_bg_tilemap, x, x);

	tilemap_draw(bitmap, cliprect, himesiki_bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	himesiki_draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}
