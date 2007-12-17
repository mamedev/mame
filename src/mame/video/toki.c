/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *background_layer,*foreground_layer,*text_layer;

UINT16 *toki_background1_videoram16;
UINT16 *toki_background2_videoram16;
UINT16 *toki_scrollram16;

/*************************************************************************
                    RASTER EFFECTS

Xscroll can be altered per scanline to create rowscroll effect.

(The driver does not implement rowscroll on the bootleg. It seems unlikely
the bootleggers would have been able to do this on their chipset. They
remapped the scroll registers, so obviously they were using a different
chip.

Why then would the old ghost of one of the remapped registers
still cause rowscroll? Probably the bootleggers simply didn't bother to
remove all the code writing the $a0000 area.)

*************************************************************************/

WRITE16_HANDLER( toki_control_w )
{
	COMBINE_DATA(&toki_scrollram16[offset]);

	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
}

VIDEO_EOF( toki )
{
	buffer_spriteram16_w(0,0,0);
}

VIDEO_EOF( tokib )
{
	buffer_spriteram16_w(0,0,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = videoram16[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_back_tile_info )
{
	int tile = toki_background1_videoram16[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	int tile = toki_background2_videoram16[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			3,
			tile,
			color,
			0);
}


/*************************************
 *
 *      Start/Stop
 *
 *************************************/

VIDEO_START( toki )
{
	text_layer       = tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,  8,8,32,32);
	background_layer = tilemap_create(get_back_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	foreground_layer = tilemap_create(get_fore_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);

	tilemap_set_transparent_pen(text_layer,15);
	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
}

/*************************************/

WRITE16_HANDLER( toki_foreground_videoram16_w )
{
	COMBINE_DATA(&videoram16[offset]);
	tilemap_mark_tile_dirty(text_layer,offset);
}

WRITE16_HANDLER( toki_background1_videoram16_w )
{
	COMBINE_DATA(&toki_background1_videoram16[offset]);
	tilemap_mark_tile_dirty(background_layer,offset);
}

WRITE16_HANDLER( toki_background2_videoram16_w )
{
	COMBINE_DATA(&toki_background2_videoram16[offset]);
	tilemap_mark_tile_dirty(foreground_layer,offset);
}

/***************************************************************************
                    SPRITES

    Original Spriteram
    ------------------

    It's not clear what purpose is served by marking tiles as being part of
    big sprites. (Big sprites in the attract abduction scene have all tiles
    marked as "first" unlike big sprites in-game.)

    We just ignore this top nibble (although perhaps in theory the bits
    enable X/Y offsets in the low byte).

    +0   x....... ........  sprite disable ??
      +0   .xx..... ........  tile is part of big sprite (4=first, 6=middle, 2=last)
    +0   .....x.. ........  ??? always set? (could be priority - see Bloodbro)
    +0   .......x ........  Flip x
    +0   ........ xxxx....  X offset: add (this * 16) to X coord
    +0   ........ ....xxxx  Y offset: add (this * 16) to Y coord

    +1   xxxx.... ........  Color bank
    +1   ....xxxx xxxxxxxx  Tile number (lo bits)
    +2   x....... ........  Tile number (hi bit)
    +2   .???.... ........  (set in not yet used entries)
    +2   .......x xxxxxxxx  X coordinate
    +3   .......x xxxxxxxx  Y coordinate

    f000 0000 f000 0000     entry not yet used: unless this is honored there
                            will be junk sprites in top left corner
    ffff ???? ???? ????     sprite marked as dead: unless this is honored
                            there will be junk sprites after floating monkey machine


    Bootleg Spriteram
    -----------------

    +0   .......x xxxxxxxx  Sprite Y coordinate
    +1   ...xxxxx xxxxxxxx  Sprite tile number
    +1   .x...... ........  Sprite flip x
    +2   xxxx.... ........  Sprite color bank
    +3   .......x xxxxxxxx  Sprite X coordinate

    f100 ???? ???? ????     dead / unused sprite ??
    ???? ???? 0000 ????     dead / unused sprite ??


***************************************************************************/


static void toki_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int x,y,xoffs,yoffs,tile,flipx,flipy,color,offs;
	UINT16 *sprite_word;

	for (offs = (spriteram_size/2)-4;offs >= 0;offs -= 4)
	{
		sprite_word = &buffered_spriteram16[offs];

		if ((sprite_word[2] != 0xf000) && (sprite_word[0] != 0xffff))
		{
			xoffs = (sprite_word[0] &0xf0);
			x = (sprite_word[2] + xoffs) & 0x1ff;
			if (x > 256)
				x -= 512;

			yoffs = (sprite_word[0] &0xf) << 4;
			y = (sprite_word[3] + yoffs) & 0x1ff;
			if (y > 256)
				y -= 512;

			color = sprite_word[1] >> 12;
			flipx   = sprite_word[0] & 0x100;
			flipy   = 0;
			tile    = (sprite_word[1] & 0xfff) + ((sprite_word[2] & 0x8000) >> 3);

			if (flip_screen) {
				x=240-x;
				y=240-y;
				if (flipx) flipx=0; else flipx=1;
				flipy=1;
			}

			drawgfx (bitmap,machine->gfx[1],
					tile,
					color,
					flipx,flipy,
					x,y,
					cliprect,TRANSPARENCY_PEN,15);
		}
	}
}


static void tokib_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int x,y,tile,flipx,color,offs;
	UINT16 *sprite_word;

	for (offs = 0;offs < spriteram_size / 2;offs += 4)
	{
		sprite_word = &buffered_spriteram16[offs];

		if (sprite_word[0] == 0xf100)
			break;
		if (sprite_word[2])
		{

			x = sprite_word[3] & 0x1ff;
			if (x > 256)
				x -= 512;

			y = sprite_word[0] & 0x1ff;
			if (y > 256)
				y = (512-y)+240;
			else
				y = 240-y;

			flipx   = sprite_word[1] & 0x4000;
			tile    = sprite_word[1] & 0x1fff;
			color   = sprite_word[2] >> 12;

			drawgfx (bitmap,machine->gfx[1],
					tile,
					color,
					flipx,0,
					x,y-1,
					cliprect,TRANSPARENCY_PEN,15);
		}
	}
}

/*************************************
 *
 *      Master update function
 *
 *************************************/

VIDEO_UPDATE( toki )
{
	int background_y_scroll,foreground_y_scroll,background_x_scroll,foreground_x_scroll;

	background_x_scroll=((toki_scrollram16[0x06] &0x7f) << 1)
								 |((toki_scrollram16[0x06] &0x80) >> 7)
								 |((toki_scrollram16[0x05] &0x10) << 4);
	background_y_scroll=((toki_scrollram16[0x0d]&0x10)<<4)+((toki_scrollram16[0x0e]&0x7f)<<1)+((toki_scrollram16[0x0e]&0x80)>>7);

	tilemap_set_scrollx( background_layer, 0, background_x_scroll );
	tilemap_set_scrolly( background_layer, 0, background_y_scroll );

	foreground_x_scroll= ((toki_scrollram16[0x16] &0x7f) << 1)
								 |((toki_scrollram16[0x16] &0x80) >> 7)
								 |((toki_scrollram16[0x15] &0x10) << 4);
	foreground_y_scroll=((toki_scrollram16[0x1d]&0x10)<<4)+((toki_scrollram16[0x1e]&0x7f)<<1)+((toki_scrollram16[0x1e]&0x80)>>7);

	tilemap_set_scrollx( foreground_layer, 0, foreground_x_scroll );
	tilemap_set_scrolly( foreground_layer, 0, foreground_y_scroll );

	flip_screen_set((toki_scrollram16[0x28]&0x8000)==0);

	if (toki_scrollram16[0x28]&0x100) {
		tilemap_draw(bitmap,cliprect,background_layer,TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect,foreground_layer,0,0);
	} else {
		tilemap_draw(bitmap,cliprect,foreground_layer,TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	}
	toki_draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,text_layer,0,0);
	return 0;
}

VIDEO_UPDATE( tokib )
{
	tilemap_set_scroll_rows(foreground_layer,1);
	tilemap_set_scroll_rows(background_layer,1);
	tilemap_set_scrolly( background_layer, 0, toki_scrollram16[0]+1 );
	tilemap_set_scrollx( background_layer, 0, toki_scrollram16[1]-0x103 );
	tilemap_set_scrolly( foreground_layer, 0, toki_scrollram16[2]+1 );
	tilemap_set_scrollx( foreground_layer, 0, toki_scrollram16[3]-0x101 );

	if (toki_scrollram16[3]&0x2000) {
		tilemap_draw(bitmap,cliprect,background_layer,TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect,foreground_layer,0,0);
	} else {
		tilemap_draw(bitmap,cliprect,foreground_layer,TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	}

	tokib_draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,text_layer,0,0);
	return 0;
}
