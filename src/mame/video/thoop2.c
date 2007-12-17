/***************************************************************************

  Gaelco Type 1 Video Hardware Rev B

  The video hardware it's nearly identical to the previous
  revision but it can handle more tiles and more sprites

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "driver.h"

UINT16 *thoop2_vregs;
UINT16 *thoop2_videoram;
UINT16 *thoop2_spriteram;

static int sprite_count[5];
static int *sprite_table[5];
static tilemap *pant[2];


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- ------xx | code (high bits)
      0  | xxxxxxxx xxxxxx-- | code (low bits)
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | --xxxxxx -------- | not used
      1  | -x------ -------- | flip x
      1  | x------- -------- | flip y
*/

static TILE_GET_INFO( get_tile_info_thoop2_screen0 )
{
	int data = thoop2_videoram[tile_index << 1];
	int data2 = thoop2_videoram[(tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo->category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}


static TILE_GET_INFO( get_tile_info_thoop2_screen1 )
{
	int data = thoop2_videoram[(0x1000/2) + (tile_index << 1)];
	int data2 = thoop2_videoram[(0x1000/2) + (tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo->category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_HANDLER( thoop2_vram_w )
{
	COMBINE_DATA(&thoop2_videoram[offset]);
	tilemap_mark_tile_dirty(pant[offset >> 11],((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( thoop2 )
{
	int i;

	pant[0] = tilemap_create(get_tile_info_thoop2_screen0,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	pant[1] = tilemap_create(get_tile_info_thoop2_screen1,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);

	tilemap_set_transmask(pant[0],0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
	tilemap_set_transmask(pant[1],0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */

	for (i = 0; i < 5; i++){
		sprite_table[i] = auto_malloc(512*sizeof(int));
	}
}

/***************************************************************************

    Sprites

***************************************************************************/

static void thoop2_sort_sprites(void)
{
	int i;

	sprite_count[0] = 0;
	sprite_count[1] = 0;
	sprite_count[2] = 0;
	sprite_count[3] = 0;
	sprite_count[4] = 0;

	for (i = 3; i < (0x1000 - 6)/2; i += 4){
		int color = (thoop2_spriteram[i+2] & 0x7e00) >> 9;
		int priority = (thoop2_spriteram[i] & 0x3000) >> 12;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38){
			sprite_table[4][sprite_count[4]] = i;
			sprite_count[4]++;
		}

		/* save sprite number in the proper array for later */
		sprite_table[priority][sprite_count[priority]] = i;
		sprite_count[priority]++;
	}
}

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
      3  | -------- ------xx | sprite code (high bits)
      3  | xxxxxxxx xxxxxx-- | sprite code (low bits)
*/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri)
{
	int j, x, y, ex, ey;
	const gfx_element *gfx = machine->gfx[0];

	static int x_offset[2] = {0x0,0x2};
	static int y_offset[2] = {0x0,0x1};

	for (j = 0; j < sprite_count[pri]; j++){
		int i = sprite_table[pri][j];
		int sx = thoop2_spriteram[i+2] & 0x01ff;
		int sy = (240 - (thoop2_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = thoop2_spriteram[i+3];
		int color = (thoop2_spriteram[i+2] & 0x7e00) >> 9;
		int attr = (thoop2_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size;

		number |= ((number & 0x03) << 16);

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

				drawgfx(bitmap,gfx,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( thoop2 )
{
	/* set scroll registers */
	tilemap_set_scrolly(pant[0], 0, thoop2_vregs[0]);
	tilemap_set_scrollx(pant[0], 0, thoop2_vregs[1]+4);
	tilemap_set_scrolly(pant[1], 0, thoop2_vregs[2]);
	tilemap_set_scrollx(pant[1], 0, thoop2_vregs[3]);

	thoop2_sort_sprites();

	fillbitmap( bitmap, machine->pens[0], cliprect );

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 3,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 3,0);
	draw_sprites(machine, bitmap,cliprect,3);
	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 3,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 3,0);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 2,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 2,0);
	draw_sprites(machine, bitmap,cliprect,2);
	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 2,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 2,0);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 1,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 1,0);
	draw_sprites(machine, bitmap,cliprect,1);
	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 1,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 1,0);

	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER1 | 0,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER1 | 0,0);
	draw_sprites(machine, bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,pant[1],TILEMAP_DRAW_LAYER0 | 0,0);
	tilemap_draw(bitmap,cliprect,pant[0],TILEMAP_DRAW_LAYER0 | 0,0);

	draw_sprites(machine, bitmap,cliprect,4);
	return 0;
}
