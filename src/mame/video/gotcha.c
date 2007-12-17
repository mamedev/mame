#include "driver.h"


UINT16 *gotcha_fgvideoram,*gotcha_bgvideoram;

static tilemap *fg_tilemap,*bg_tilemap;
static int banksel,gfxbank[4];


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( gotcha_tilemap_scan )
{
	return (col & 0x1f) | (row << 5) | ((col & 0x20) << 5);
}

INLINE void get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *vram,int color_offs)
{
	UINT16 data = vram[tile_index];
	int code = (data & 0x3ff) | (gfxbank[(data & 0x0c00) >> 10] << 10);

	SET_TILE_INFO(0,code,(data >> 12) + color_offs,0);
}

static TILE_GET_INFO( fg_get_tile_info ) { get_tile_info(machine,tileinfo,tile_index,gotcha_fgvideoram, 0); }
static TILE_GET_INFO( bg_get_tile_info ) { get_tile_info(machine,tileinfo,tile_index,gotcha_bgvideoram,16); }



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gotcha )
{
	fg_tilemap = tilemap_create(fg_get_tile_info,gotcha_tilemap_scan,TILEMAP_TYPE_PEN,16,16,64,32);
	bg_tilemap = tilemap_create(bg_get_tile_info,gotcha_tilemap_scan,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(fg_tilemap,-1,0);
	tilemap_set_scrolldx(bg_tilemap,-5,0);
}


WRITE16_HANDLER( gotcha_fgvideoram_w )
{
	COMBINE_DATA(&gotcha_fgvideoram[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE16_HANDLER( gotcha_bgvideoram_w )
{
	COMBINE_DATA(&gotcha_bgvideoram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE16_HANDLER( gotcha_gfxbank_select_w )
{
	if (ACCESSING_MSB)
		banksel = (data & 0x0300) >> 8;
}

WRITE16_HANDLER( gotcha_gfxbank_w )
{
	if (ACCESSING_MSB)
	{
		if (gfxbank[banksel] != ((data & 0x0f00) >> 8))
		{
			gfxbank[banksel] = (data & 0x0f00) >> 8;
			tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
		}
	}
}

WRITE16_HANDLER( gotcha_scroll_w )
{
	static UINT16 scroll[4];
	COMBINE_DATA(&scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(fg_tilemap,0,scroll[0]); break;
		case 1: tilemap_set_scrolly(fg_tilemap,0,scroll[1]); break;
		case 2: tilemap_set_scrollx(bg_tilemap,0,scroll[2]); break;
		case 3: tilemap_set_scrolly(bg_tilemap,0,scroll[3]); break;
	}
}




static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size/2;offs += 4)
	{
		int sx,sy,code,color,flipx,flipy,height,y;

		sx = spriteram16[offs + 2];
		sy = spriteram16[offs + 0];
		code = spriteram16[offs + 1];
		color = spriteram16[offs + 2] >> 9;
		height = 1 << ((spriteram16[offs + 0] & 0x0600) >> 9);
		flipx = spriteram16[offs + 0] & 0x2000;
		flipy = spriteram16[offs + 0] & 0x4000;

		for (y = 0;y < height;y++)
		{
			drawgfx(bitmap,machine->gfx[1],
					code + (flipy ? height-1 - y : y),
					color,
					flipx,flipy,
					0x140-5 - ((sx + 0x10) & 0x1ff),0x100+1 - ((sy + 0x10 * (height - y)) & 0x1ff),
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}


VIDEO_UPDATE( gotcha )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	draw_sprites(machine, bitmap,cliprect);
	return 0;
}
