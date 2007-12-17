/***************************************************************************

  World Rally Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "driver.h"

UINT16 *wrally_spriteram;
UINT16 *wrally_vregs;
UINT16 *wrally_videoram;

static tilemap *pant[2];

/* from machine/wrally.c */
extern UINT16 *wrally_encr_table[2];

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (64*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | --xxxxxx xxxxxxxx | code
      0  | xx------ -------- | not used?
      1  | xxxxxxxx xxxxxxxx | unknown

      preliminary
*/


static TILE_GET_INFO( get_tile_info_wrally_screen0 )
{
	int data = wrally_videoram[tile_index << 1];
	int data2 = wrally_videoram[(tile_index << 1) + 1];
	int code = data & 0x3fff;

	SET_TILE_INFO(0, code, data2 & 0x1f, TILE_FLIPXY((data2 >> 5) & 0x00));
}

static TILE_GET_INFO( get_tile_info_wrally_screen1 )
{
	int data = wrally_videoram[(0x2000/2) + (tile_index << 1)];
	int data2 = wrally_videoram[(0x2000/2) + (tile_index << 1) + 1];
	int code = data & 0x3fff;

	SET_TILE_INFO(0, code, data2 & 0x1f, TILE_FLIPXY((data2 >> 5) & 0x00));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_HANDLER( wrally_vram_w )
{
	wrally_videoram[offset] = wrally_encr_table[offset & 0x01][data];

	tilemap_mark_tile_dirty(pant[(offset & 0x1fff) >> 12], ((offset << 1) & 0x1fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( wrally )
{
	pant[0] = tilemap_create(get_tile_info_wrally_screen0,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64,32);
	pant[1] = tilemap_create(get_tile_info_wrally_screen1,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64,32);

	tilemap_set_transparent_pen(pant[0],0);
	tilemap_set_transparent_pen(pant[1],0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | unknown
      2  | ------xx xxxxxxxx | x position
      2  | --xxxx-- -------- | sprite color (low 4 bits)
      2  | xx------ -------- | unknown
      3  | --xxxxxx xxxxxxxx | sprite code
      3  | xx------ -------- | not used?

      preliminary
*/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int i, x, y, ex, ey;
	const gfx_element *gfx = machine->gfx[0];

	static int x_offset[2] = {0x0,0x2};
	static int y_offset[2] = {0x0,0x1};

	for (i = 3; i < (0x1000 - 6)/2; i += 4){
		int sx = wrally_spriteram[i+2] & 0x03ff;
		int sy = (240 - (wrally_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = wrally_spriteram[i+3] & 0x3fff;
		int color = ((wrally_spriteram[i+2] & 0xfc00) >> 10);
		int attr = (wrally_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size = 1;

		color = (color & 0x0f);

		for (y = 0; y < spr_size; y++){
			for (x = 0; x < spr_size; x++){

				ex = xflip ? (spr_size-1-x) : x;
				ey = yflip ? (spr_size-1-y) : y;

				drawgfx(bitmap,gfx,number + x_offset[ex] + y_offset[ey],
						0x20 + color,xflip,yflip,
						sx-0x0f+x*16,sy+y*16,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( wrally )
{
	/* set scroll registers */
	tilemap_set_scrolly(pant[0], 0, wrally_vregs[0]);
	tilemap_set_scrollx(pant[0], 0, wrally_vregs[1]);
	tilemap_set_scrolly(pant[1], 0, wrally_vregs[2]);
	tilemap_set_scrollx(pant[1], 0, wrally_vregs[3]);

	fillbitmap( bitmap, machine->pens[0], cliprect );

	tilemap_draw(bitmap,cliprect,pant[1],0,0);
	tilemap_draw(bitmap,cliprect,pant[0],0,0);
	draw_sprites(machine, bitmap,cliprect);
	return 0;
}
