#include "driver.h"
#include "video/taitoic.h"


static tilemap *fg_tilemap;
UINT16 *darius_fg_ram;

struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};
static struct tempsprite *spritelist;


/***************************************************************************/

INLINE void actual_get_fg_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index, UINT16 *ram,int gfxnum)
{
	UINT16 code = (ram[tile_index + 0x2000] & 0x7ff);
	UINT16 attr = ram[tile_index];

	SET_TILE_INFO(
			gfxnum,
			code,
			((attr & 0xff) << 2),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	actual_get_fg_tile_info(machine, tileinfo, tile_index, darius_fg_ram, 2);
}

/***************************************************************************/

VIDEO_START( darius )
{
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,128,64);

	spritelist = auto_alloc_array(machine, struct tempsprite, 0x800);

	/* (chips, gfxnum, x_offs, y_offs, y_invert, opaque, dblwidth) */
	PC080SN_vh_start(machine,1,1,-16,8,0,1,1);

	tilemap_set_transparent_pen(fg_tilemap,0);
}

/***************************************************************************/

WRITE16_HANDLER( darius_fg_layer_w )
{
	COMBINE_DATA(&darius_fg_ram[offset]);
	if (offset < 0x4000)
		tilemap_mark_tile_dirty(fg_tilemap,(offset & 0x1fff));
}

/***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int primask, int x_offs, int y_offs)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs,curx,cury;
	UINT16 code,data,sx,sy;
	UINT8 flipx,flipy,color,priority;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
       while processing sprite ram and then draw them all at the end */
	struct tempsprite *sprite_ptr = spritelist;

	for (offs = machine->generic.spriteram_size/2-4; offs >= 0; offs -= 4)
	{
		code = spriteram16[offs+2] &0x1fff;

		if (code)
		{
			data = spriteram16[offs];
			sy = (256-data) & 0x1ff;

			data = spriteram16[offs+1];
			sx = data & 0x3ff;

			data = spriteram16[offs+2];
			flipx = ((data & 0x4000) >> 14);
			flipy = ((data & 0x8000) >> 15);

			data = spriteram16[offs+3];
			priority = (data &0x80) >> 7;  // 0 = low
			if (priority != primask) continue;
			color = (data & 0x7f);

			curx = sx - x_offs;
			cury = sy + y_offs;

			if (curx > 900) curx -= 1024;
 			if (cury > 400) cury -= 512;

			sprite_ptr->code = code;
			sprite_ptr->color = color;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
					sprite_ptr->code,
					sprite_ptr->color,
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,0);
		}
	}
}



VIDEO_UPDATE( darius )
{
	int xoffs = 0;
	const device_config *left_screen   = devtag_get_device(screen->machine, "lscreen");
	const device_config *middle_screen = devtag_get_device(screen->machine, "mscreen");
	const device_config *right_screen  = devtag_get_device(screen->machine, "rscreen");

	if (screen == left_screen)
		xoffs = 36*8*0;
	else if (screen == middle_screen)
		xoffs = 36*8*1;
	else if (screen == right_screen)
		xoffs = 36*8*2;

	PC080SN_tilemap_update();

	// draw bottom layer(always active)
 	PC080SN_tilemap_draw_offset(bitmap,cliprect,0,0,TILEMAP_DRAW_OPAQUE,0,-xoffs,0);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen->machine,bitmap,cliprect,0,xoffs,-8); // draw sprites with priority 0 which are under the mid layer

	// draw middle layer
	PC080SN_tilemap_draw_offset(bitmap,cliprect,0,1,0,0,-xoffs,0);

	draw_sprites(screen->machine,bitmap,cliprect,1,xoffs,-8); // draw sprites with priority 1 which are over the mid layer

	/* top(text) layer is in fixed position */
	tilemap_set_scrollx(fg_tilemap,0,0+xoffs);
	tilemap_set_scrolly(fg_tilemap,0,-8);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
