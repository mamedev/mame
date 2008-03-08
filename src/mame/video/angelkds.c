/* video/angelkds.c - see drivers/angelkds.c for more info */

/* graphical issues

enable / disable tilemap bits might be wrong

*/

#include "driver.h"
#include "deprecat.h"


static tilemap *tx_tilemap,*bgbot_tilemap ,*bgtop_tilemap;

UINT8 *angelkds_txvideoram, *angelkds_bgbotvideoram, *angelkds_bgtopvideoram;

/*** Text Layer Tilemap

*/

static int angelkds_txbank;


static TILE_GET_INFO( get_tx_tile_info )
{
	int tileno;

	tileno = angelkds_txvideoram[tile_index] + (angelkds_txbank * 0x100);

	SET_TILE_INFO(0,tileno,0,0);
}

WRITE8_HANDLER( angelkds_txvideoram_w )
{
	angelkds_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

WRITE8_HANDLER( angelkds_txbank_write )
{

if (angelkds_txbank != data)
	{
	angelkds_txbank = data;
	tilemap_mark_all_tiles_dirty (tx_tilemap);
	};

}

/*** Top Half Background Tilemap

*/

static int angelkds_bgtopbank;


static TILE_GET_INFO( get_bgtop_tile_info )
{
	int tileno;

	tileno = angelkds_bgtopvideoram[tile_index];

	tileno += angelkds_bgtopbank*0x100 ;
	SET_TILE_INFO(1,tileno,0,0);
}

WRITE8_HANDLER( angelkds_bgtopvideoram_w )
{
	angelkds_bgtopvideoram[offset] = data;
	tilemap_mark_tile_dirty(bgtop_tilemap,offset);
}

WRITE8_HANDLER( angelkds_bgtopbank_write )
{

if (angelkds_bgtopbank != data)
	{
	angelkds_bgtopbank = data;
	tilemap_mark_all_tiles_dirty (bgtop_tilemap);
	};

}

WRITE8_HANDLER( angelkds_bgtopscroll_write )
{
	tilemap_set_scrollx(bgtop_tilemap, 0, data );
}

/*** Bottom Half Background Tilemap

*/

static int angelkds_bgbotbank;

static TILE_GET_INFO( get_bgbot_tile_info )
{
	int tileno;

	tileno = angelkds_bgbotvideoram[tile_index];

	tileno += angelkds_bgbotbank * 0x100 ;
	SET_TILE_INFO(2,tileno,1,0);
}

WRITE8_HANDLER( angelkds_bgbotvideoram_w )
{
	angelkds_bgbotvideoram[offset] = data;
	tilemap_mark_tile_dirty(bgbot_tilemap,offset);
}


WRITE8_HANDLER( angelkds_bgbotbank_write )
{

if (angelkds_bgbotbank != data)
	{
	angelkds_bgbotbank = data;
	tilemap_mark_all_tiles_dirty (bgbot_tilemap);
	};

}

WRITE8_HANDLER( angelkds_bgbotscroll_write )
{
	tilemap_set_scrollx(bgbot_tilemap, 0, data );
}

static UINT8 angelkds_layer_ctrl;

WRITE8_HANDLER( angelkds_layer_ctrl_write )
{
	angelkds_layer_ctrl = data;
}

/*** Sprites

the sprites are similar to the tilemaps in the sense that there is
a split down the middle of the screen

*/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int enable_n)
{
	const UINT8 *source = spriteram+0x100-4;
	const UINT8 *finish = spriteram;
	const gfx_element *gfx = machine->gfx[3];

	while( source>=finish )
	{
	/*

    nnnn nnnn - EeFf B?cc - yyyy yyyy - xxxx xxxx

    n = sprite number
    E = Sprite Enabled in Top Half of Screen
    e = Sprite Enabled in Bottom Half of Screen
    F = Flip Y
    f = Flip X
    B = Tile Bank
    ? = unknown, nothing / unused? recheck
    c = color
    y = Y position
    x = X position

    */


	UINT16 tile_no = source[0];
	UINT8 attr = source[1];
	UINT8 ypos = source[2];
	UINT8 xpos = source[3];

	UINT8 enable = attr & 0xc0;
	UINT8 flipx = (attr & 0x10) >> 4;
	UINT8 flipy = (attr & 0x20) >> 5;
	UINT8 bank = attr & 0x08;
	UINT8 color = attr & 0x03;

	if (bank) tile_no +=0x100;

	ypos= 0xff-ypos;

	if (enable & enable_n)
	{
			drawgfx(
					bitmap,
					gfx,
					tile_no,
					color*4,
					flipx,flipy,
					xpos,ypos,
					cliprect,
					TRANSPARENCY_PEN,15
					);
			/* wraparound */
			if (xpos > 240)
				drawgfx(
						bitmap,
						gfx,
						tile_no,
						color*4,
						flipx,flipy,
						xpos-256,ypos,
						cliprect,
						TRANSPARENCY_PEN,15
						);
			/* wraparound */
			if (ypos > 240)
			{
				drawgfx(
						bitmap,
						gfx,
						tile_no,
						color*4,
						flipx,flipy,
						xpos,ypos-256,
						cliprect,
						TRANSPARENCY_PEN,15
						);
				/* wraparound */
				if (xpos > 240)
							drawgfx(
									bitmap,
									gfx,
									tile_no,
									color*4,
									flipx,flipy,
									xpos-256,ypos-256,
									cliprect,
									TRANSPARENCY_PEN,15
									);
			}

	}

	source-=0x04;

	}

}


/*** Palette Handling

 4 bits of Red, 4 bits of Green, 4 bits of Blue

*/

WRITE8_HANDLER( angelkds_paletteram_w )
{
	int no;


	paletteram[offset] = data;

	no=offset & 0xff;
	palette_set_color_rgb(Machine,no,pal4bit(paletteram[no]),pal4bit(paletteram[no]>>4),pal4bit(paletteram[no+0x100]));
}

/*** Video Start & Update

*/

VIDEO_START( angelkds )
{

	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows, 8, 8,32,32);
	tilemap_set_transparent_pen(tx_tilemap,0);

	bgbot_tilemap = tilemap_create(get_bgbot_tile_info,tilemap_scan_rows, 8, 8,32,32);
	tilemap_set_transparent_pen(bgbot_tilemap,15);

	bgtop_tilemap = tilemap_create(get_bgtop_tile_info,tilemap_scan_rows, 8, 8,32,32);
	tilemap_set_transparent_pen(bgtop_tilemap,15);
}

/* enable bits are uncertain */

VIDEO_UPDATE( angelkds )
{
	rectangle clip;

	fillbitmap(bitmap,0x3f,cliprect); /* is there a register controling the colour?, we currently use the last colour of the tx palette */

	/* draw top of screen */
	clip.min_x = 8*0;
	clip.max_x = 8*16-1;
	clip.min_y = screen->machine->screen[0].visarea.min_y;
	clip.max_y = screen->machine->screen[0].visarea.max_y;
	if ((angelkds_layer_ctrl & 0x80) == 0x00) tilemap_draw(bitmap,&clip,bgtop_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,&clip, 0x80);
	if ((angelkds_layer_ctrl & 0x20) == 0x00) tilemap_draw(bitmap,&clip,tx_tilemap,0,0);

	/* draw bottom of screen */
	clip.min_x = 8*16;
	clip.max_x = 8*32-1;
	clip.min_y = screen->machine->screen[0].visarea.min_y;
	clip.max_y = screen->machine->screen[0].visarea.max_y;
	if ((angelkds_layer_ctrl & 0x40) == 0x00) tilemap_draw(bitmap,&clip,bgbot_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,&clip, 0x40);
	if ((angelkds_layer_ctrl & 0x20) == 0x00) tilemap_draw(bitmap,&clip,tx_tilemap,0,0);
	return 0;
}
