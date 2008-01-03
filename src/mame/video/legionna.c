/***************************************************************************

    Legionnaire / Heated Barrel video hardware (derived from D-Con)

***************************************************************************/

#include "driver.h"

UINT16 *legionna_back_data,*legionna_fore_data,*legionna_mid_data,*legionna_scrollram16,*legionna_textram;

static tilemap *background_layer,*foreground_layer,*midground_layer,*text_layer;
//static int legionna_enable;

/******************************************************************************/

static UINT16 back_gfx_bank = 0,fore_gfx_bank = 0,mid_gfx_bank = 0;
UINT8 sdgndmrb_pri_n;

void heatbrl_setgfxbank(UINT16 data)
{
	back_gfx_bank = (data &0x4000) >> 2;
}

/*xxx- --- ---- ---- banking*/
void denjinmk_setgfxbank(UINT16 data)
{
	fore_gfx_bank = (data &0x2000) >> 1;//???
	back_gfx_bank = (data &0x4000) >> 2;
	mid_gfx_bank  = (data &0x8000) >> 3;//???
}

#ifdef UNUSED_FUNCTION
WRITE16_HANDLER( legionna_control_w )
{
	if (ACCESSING_LSB)
	{
		legionna_enable=data;
		if ((legionna_enable&4)==4)
			tilemap_set_enable(foreground_layer,0);
		else
			tilemap_set_enable(foreground_layer,1);

		if ((legionna_enable&2)==2)
			tilemap_set_enable(midground_layer,0);
		else
			tilemap_set_enable(midground_layer,1);

		if ((legionna_enable&1)==1)
			tilemap_set_enable(background_layer,0);
		else
			tilemap_set_enable(background_layer,1);
	}
}
#endif

WRITE16_HANDLER( legionna_background_w )
{
	COMBINE_DATA(&legionna_back_data[offset]);
	tilemap_mark_tile_dirty(background_layer,offset);
}

WRITE16_HANDLER( legionna_midground_w )
{
	COMBINE_DATA(&legionna_mid_data[offset]);
	tilemap_mark_tile_dirty(midground_layer,offset);
}

WRITE16_HANDLER( legionna_foreground_w )
{
	COMBINE_DATA(&legionna_fore_data[offset]);
	tilemap_mark_tile_dirty(foreground_layer,offset);
}

WRITE16_HANDLER( legionna_text_w )
{
	COMBINE_DATA(&legionna_textram[offset]);
	tilemap_mark_tile_dirty(text_layer,offset);
}

static TILE_GET_INFO( get_back_tile_info )
{
	int tile=legionna_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;
	tile |= back_gfx_bank;		/* Heatbrl uses banking */

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	int tile=legionna_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(5,tile,color,0);
}

static TILE_GET_INFO( get_mid_tile_info_denji )
{
	int tile=legionna_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;
	tile |= mid_gfx_bank;

	SET_TILE_INFO(5,tile,color,0);
}

static TILE_GET_INFO( get_mid_tile_info_cupsoc )
{
	int tile=legionna_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	tile |= 0x1000;

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info )	/* this is giving bad tiles... */
{
	int tile=legionna_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	// legionnaire tile numbers / gfx set wrong, see screen after coin insertion
	tile &= 0xfff;

	SET_TILE_INFO(4,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info_denji )
{
	int tile=legionna_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;
	tile |= fore_gfx_bank;

	SET_TILE_INFO(4,tile,color,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = legionna_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(0,tile,color,0);
}

VIDEO_START( legionna )
{
	background_layer = tilemap_create(get_back_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	foreground_layer = tilemap_create(get_fore_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	midground_layer =  tilemap_create(get_mid_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	text_layer =       tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,  8,8,64,32);

	legionna_scrollram16 = auto_malloc(0x60);

	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(midground_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,15);
}

VIDEO_START( denjinmk )
{
	background_layer = tilemap_create(get_back_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	foreground_layer = tilemap_create(get_fore_tile_info_denji,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	midground_layer =  tilemap_create(get_mid_tile_info_denji, tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	text_layer =       tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,  8,8,64,32);

	legionna_scrollram16 = auto_malloc(0x60);

	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(midground_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,7);//?
}

VIDEO_START( cupsoc )
{
	background_layer = tilemap_create(get_back_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	foreground_layer = tilemap_create(get_fore_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	midground_layer =  tilemap_create(get_mid_tile_info_cupsoc, tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	text_layer =       tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,  8,8,64,32);

	legionna_scrollram16 = auto_malloc(0x60);

	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(midground_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,15);
}


/*************************************************************************

    Legionnaire Spriteram (similar to Dcon)
    ---------------------

    It has "big sprites" created by setting width or height >0. Tile
    numbers are read consecutively.

    +0   x....... ........  Sprite enable
    +0   .x...... ........  Flip x
    +0   ..x..... ........  Flip y ???
    +0   ...xxx.. ........  Width: do this many tiles horizontally
    +0   ......xx x.......  Height: do this many tiles vertically
    +0   ........ .x......  Tile bank,used in Denjin Makai
    +0   ........ ..xxxxxx  Color bank

    +1   xx...... ........  Priority? (1=high?)
    +1   ..xxxxxx xxxxxxxx  Tile number
    +2   xxxxxxxx xxxxxxxx  X coordinate (signed)
    +3   xxxxxxxx xxxxxxxx  Y coordinate (signed)

*************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int pri)
{
	int offs,fx,fy,x,y,color,sprite,cur_pri;
	int dx,dy,ax,ay;

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		UINT16 data = spriteram16[offs];
		if (!(data &0x8000)) continue;

		cur_pri = (spriteram16[offs+1] & 0xc000) >> 14;
		if (cur_pri!=pri) continue;

		sprite = spriteram16[offs+1];

		sprite &= 0x3fff;
		if(data & 0x40) sprite |= 0x4000;//tile banking,used in Denjin Makai

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		if (x &0x8000)	x = -(0x200-(x &0x1ff));
		else	x &= 0x1ff;
		if (y &0x8000)	y = -(0x200-(y &0x1ff));
		else	y &= 0x1ff;

		color = (data &0x3f) + 0x40;
		fx =  (data &0x4000) >> 14;
		fy =  (data &0x2000) >> 13;
		dy = ((data &0x0380) >> 7)  + 1;
		dx = ((data &0x1c00) >> 10) + 1;

		if (!fx)
		{
			if(!fy)
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx(bitmap,machine->gfx[3],
						sprite++,
						color,fx,fy,x+ax*16,y+ay*16,
						cliprect,TRANSPARENCY_PEN,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx(bitmap,machine->gfx[3],
						sprite++,
						color,fx,fy,x+ax*16,y+(dy-ay-1)*16,
						cliprect,TRANSPARENCY_PEN,15);
					}
			}
		}
		else
		{
			if(!fy)
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx(bitmap,machine->gfx[3],
						sprite++,
						color,fx,fy,x+(dx-ax-1)*16,y+ay*16,
						cliprect,TRANSPARENCY_PEN,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx(bitmap,machine->gfx[3],
						sprite++,
						color,fx,fy,x+(dx-ax-1)*16,y+(dy-ay-1)*16,
						cliprect,TRANSPARENCY_PEN,15);
					}
			}
		}
	}
}

#define LAYER_DB 0

VIDEO_UPDATE( legionna )
{
#if LAYER_DB
	static int dislayer[5];	/* Layer toggles to help get the layers correct */
#endif

#if LAYER_DB
if (input_code_pressed_once (KEYCODE_Z))
	{
		dislayer[0] ^= 1;
		popmessage("bg0: %01x",dislayer[0]);
	}

	if (input_code_pressed_once (KEYCODE_X))
	{
		dislayer[1] ^= 1;
		popmessage("bg1: %01x",dislayer[1]);
	}

	if (input_code_pressed_once (KEYCODE_C))
	{
		dislayer[2] ^= 1;
		popmessage("bg2: %01x",dislayer[2]);
	}

	if (input_code_pressed_once (KEYCODE_V))
	{
		dislayer[3] ^= 1;
		popmessage("sprites: %01x",dislayer[3]);
	}

	if (input_code_pressed_once (KEYCODE_B))
	{
		dislayer[4] ^= 1;
		popmessage("text: %01x",dislayer[4]);
	}
#endif

	/* Setup the tilemaps */
	tilemap_set_scrollx( background_layer, 0, legionna_scrollram16[0] );
	tilemap_set_scrolly( background_layer, 0, legionna_scrollram16[1] );
	tilemap_set_scrollx( midground_layer,  0, legionna_scrollram16[2] );
	tilemap_set_scrolly( midground_layer,  0, legionna_scrollram16[3] );
	tilemap_set_scrollx( foreground_layer, 0, legionna_scrollram16[4] );
	tilemap_set_scrolly( foreground_layer, 0, legionna_scrollram16[5] );

//  if ((legionna_enable&1)!=1)

	fillbitmap(bitmap,get_black_pen(machine),cliprect);	/* wrong color? */

#if LAYER_DB
	if (dislayer[2]==0)
#endif
	tilemap_draw(bitmap,cliprect,foreground_layer,TILEMAP_DRAW_OPAQUE,0);

#if LAYER_DB
	if (dislayer[1]==0)
#endif
	tilemap_draw(bitmap,cliprect,midground_layer,0,0);

	draw_sprites(machine, bitmap,cliprect,3);
#if LAYER_DB
	if (dislayer[0]==0)
#endif
	tilemap_draw(bitmap,cliprect,background_layer,0,0);
	draw_sprites(machine,bitmap,cliprect,2);
	draw_sprites(machine,bitmap,cliprect,1);
	draw_sprites(machine,bitmap,cliprect,0);

#if LAYER_DB
	if (dislayer[4]==0)
#endif
	tilemap_draw(bitmap,cliprect,text_layer,0,0);

	return 0;
}

VIDEO_UPDATE( godzilla )
{
//  tilemap_set_scrollx( text_layer, 0, 0 );
//  tilemap_set_scrolly( text_layer, 0, 112 );
	/* Setup the tilemaps */
	tilemap_set_scrollx( background_layer, 0, legionna_scrollram16[0] );
	tilemap_set_scrolly( background_layer, 0, legionna_scrollram16[1] );
	tilemap_set_scrollx( midground_layer,  0, legionna_scrollram16[2] );
	tilemap_set_scrolly( midground_layer,  0, legionna_scrollram16[3] );
	tilemap_set_scrollx( foreground_layer, 0, legionna_scrollram16[4] );
	tilemap_set_scrolly( foreground_layer, 0, legionna_scrollram16[5] );

	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	tilemap_draw(bitmap,cliprect,background_layer,0,0);
	draw_sprites(machine,bitmap,cliprect,2);
	tilemap_draw(bitmap,cliprect,midground_layer,0,0);
	draw_sprites(machine,bitmap,cliprect,1);
	tilemap_draw(bitmap,cliprect,foreground_layer,0,0);
	draw_sprites(machine,bitmap,cliprect,0);
	draw_sprites(machine,bitmap,cliprect,3);
	tilemap_draw(bitmap,cliprect,text_layer,0,0);

	return 0;
}

VIDEO_UPDATE( sdgndmrb )
{
	/* Setup the tilemaps */
	tilemap_set_scrollx( background_layer, 0, legionna_scrollram16[0] );
	tilemap_set_scrolly( background_layer, 0, legionna_scrollram16[1] );
	tilemap_set_scrollx( midground_layer,  0, legionna_scrollram16[2] );
	tilemap_set_scrolly( midground_layer,  0, legionna_scrollram16[3] );
	tilemap_set_scrollx( foreground_layer, 0, legionna_scrollram16[4] );
	tilemap_set_scrolly( foreground_layer, 0, legionna_scrollram16[5] );
  	tilemap_set_scrollx( text_layer, 0,  legionna_scrollram16[6] );
  	tilemap_set_scrolly( text_layer, 0,  legionna_scrollram16[7] );

	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	if(!(sdgndmrb_pri_n & 1))
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	draw_sprites(machine,bitmap,cliprect,2);

	if(!(sdgndmrb_pri_n & 2))
		tilemap_draw(bitmap,cliprect,midground_layer,0,0);

	draw_sprites(machine,bitmap,cliprect,1);

	if(!(sdgndmrb_pri_n & 4))
		tilemap_draw(bitmap,cliprect,foreground_layer,0,0);

	draw_sprites(machine,bitmap,cliprect,0);

	draw_sprites(machine,bitmap,cliprect,3);

	if(!(sdgndmrb_pri_n & 8))
		tilemap_draw(bitmap,cliprect,text_layer,0,0);

	return 0;
}

