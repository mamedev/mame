/***************************************************************************

  Gaelco Type 1 Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "driver.h"

UINT16 *gaelco_vregs;
UINT16 *gaelco_videoram;
UINT16 *gaelco_spriteram;

tilemap *gaelco_tilemap[2];
#define pant gaelco_tilemap


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -------x | flip x
      0  | -------- ------x- | flip y
      0  | xxxxxxxx xxxxxx-- | code
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | xxxxxxxx -------- | not used
*/

static TILE_GET_INFO( get_tile_info_gaelco_screen0 )
{
	int data = gaelco_videoram[tile_index << 1];
	int data2 = gaelco_videoram[(tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2);

	tileinfo->category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}


static TILE_GET_INFO( get_tile_info_gaelco_screen1 )
{
	int data = gaelco_videoram[(0x1000/2) + (tile_index << 1)];
	int data2 = gaelco_videoram[(0x1000/2) + (tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2);

	tileinfo->category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_HANDLER( gaelco_vram_w )
{
	COMBINE_DATA(&gaelco_videoram[offset]);
	tilemap_mark_tile_dirty(pant[offset >> 11],((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( bigkarnk )
{
	pant[0] = tilemap_create(get_tile_info_gaelco_screen0,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	pant[1] = tilemap_create(get_tile_info_gaelco_screen1,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);

	tilemap_set_transmask(pant[0],0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
	tilemap_set_transmask(pant[1],0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
}

VIDEO_START( maniacsq )
{
	pant[0] = tilemap_create(get_tile_info_gaelco_screen0,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	pant[1] = tilemap_create(get_tile_info_gaelco_screen1,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);

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
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (8x8 cuadrant)
      3  | xxxxxxxx xxxxxx-- | sprite code
*/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int i, x, y, ex, ey;
	const gfx_element *gfx = machine->gfx[0];

	static int x_offset[2] = {0x0,0x2};
	static int y_offset[2] = {0x0,0x1};

	for (i = 0x800 - 4 - 1; i >= 3; i -= 4){
		int sx = gaelco_spriteram[i+2] & 0x01ff;
		int sy = (240 - (gaelco_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = gaelco_spriteram[i+3];
		int color = (gaelco_spriteram[i+2] & 0x7e00) >> 9;
		int attr = (gaelco_spriteram[i] & 0xfe00) >> 9;
		int priority = (gaelco_spriteram[i] & 0x3000) >> 12;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size, pri_mask;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38){
			priority = 4;
		}

		switch( priority )
		{
			case 0: pri_mask = 0xff00; break;
			case 1: pri_mask = 0xff00|0xf0f0; break;
			case 2: pri_mask = 0xff00|0xf0f0|0xcccc; break;
			case 3: pri_mask = 0xff00|0xf0f0|0xcccc|0xaaaa; break;
			default:
			case 4: pri_mask = 0; break;
		}

		if (attr & 0x04){
			spr_size = 1;
		}
		else{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++){
			for (x = 0; x < spr_size; x++){

				ex = xflip ? (spr_size-1-x) : x;
				ey = yflip ? (spr_size-1-y) : y;

				pdrawgfx(bitmap,gfx,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,
						cliprect,TRANSPARENCY_PEN,0,pri_mask);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( maniacsq )
{
	/* set scroll registers */
	tilemap_set_scrolly(pant[0], 0, gaelco_vregs[0]);
	tilemap_set_scrollx(pant[0], 0, gaelco_vregs[1]+4);
	tilemap_set_scrolly(pant[1], 0, gaelco_vregs[2]);
	tilemap_set_scrollx(pant[1], 0, gaelco_vregs[3]);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap( bitmap, machine->pens[0], cliprect );

	tilemap_draw(bitmap,cliprect,pant[1],3,0);
	tilemap_draw(bitmap,cliprect,pant[0],3,0);

	tilemap_draw(bitmap,cliprect,pant[1],2,1);
	tilemap_draw(bitmap,cliprect,pant[0],2,1);

	tilemap_draw(bitmap,cliprect,pant[1],1,2);
	tilemap_draw(bitmap,cliprect,pant[0],1,2);

	tilemap_draw(bitmap,cliprect,pant[1],0,4);
	tilemap_draw(bitmap,cliprect,pant[0],0,4);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( bigkarnk )
{
	/* set scroll registers */
	tilemap_set_scrolly(pant[0], 0, gaelco_vregs[0]);
	tilemap_set_scrollx(pant[0], 0, gaelco_vregs[1]+4);
	tilemap_set_scrolly(pant[1], 0, gaelco_vregs[2]);
	tilemap_set_scrollx(pant[1], 0, gaelco_vregs[3]);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap( bitmap, machine->pens[0], cliprect );

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 3,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 3,0);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 3,1);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 3,1);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 2,1);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 2,1);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 2,2);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 2,2);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 1,2);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 1,2);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 1,4);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 1,4);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 0,4);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 0,4);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 0,8);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 0,8);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
