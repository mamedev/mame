/***************************************************************************

    Legionnaire / Heated Barrel video hardware (derived from D-Con)

***************************************************************************/

#include "driver.h"
#include "includes/legionna.h"

UINT16 *legionna_back_data,*legionna_fore_data,*legionna_mid_data,*legionna_scrollram16,*legionna_textram;

static tilemap *background_layer,*foreground_layer,*midground_layer,*text_layer;
UINT16 legionna_layer_disable;

/******************************************************************************/

static UINT16 back_gfx_bank = 0,fore_gfx_bank = 0,mid_gfx_bank = 0;
UINT8 grainbow_pri_n;

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

	tilemap_mark_all_tiles_dirty (background_layer);
	tilemap_mark_all_tiles_dirty (foreground_layer);
	tilemap_mark_all_tiles_dirty (midground_layer);
	tilemap_mark_all_tiles_dirty (text_layer);
}

#ifdef UNUSED_FUNCTION
WRITE16_HANDLER( legionna_control_w )
{
	if (ACCESSING_BITS_0_7)
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
	background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	midground_layer =  tilemap_create(machine, get_mid_tile_info, tilemap_scan_rows,16,16,32,32);
	text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);
	legionna_layer_disable = 0x0000;

	legionna_scrollram16 = auto_alloc_array(machine, UINT16, 0x60/2);

	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(midground_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,15);
}

VIDEO_START( denjinmk )
{
	background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	foreground_layer = tilemap_create(machine, get_fore_tile_info_denji,tilemap_scan_rows,16,16,32,32);
	midground_layer =  tilemap_create(machine, get_mid_tile_info_denji, tilemap_scan_rows,16,16,32,32);
	text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);
	legionna_layer_disable = 0x0000;

	legionna_scrollram16 = auto_alloc_array(machine, UINT16, 0x60/2);

	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(midground_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,7);//?
}

VIDEO_START( cupsoc )
{
	background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	midground_layer =  tilemap_create(machine, get_mid_tile_info_cupsoc, tilemap_scan_rows,16,16,32,32);
	text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);
	legionna_layer_disable = 0x0000;

	legionna_scrollram16 = auto_alloc_array(machine, UINT16, 0x60/2);

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

    +2   ----xxxx xxxxxxxx  X coordinate (signed)

    +3   b------- --------  more tile banking used by Denjin Makai
    +3   ----xxxx xxxxxxxx  Y coordinate (signed)

*************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int pri)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
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
		if(data & 0x0040) sprite |= 0x4000;//tile banking,used in Denjin Makai
		if(spriteram16[offs+3] & 0x8000) sprite |= 0x8000;//tile banking?,used in Denjin Makai

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		/* heated barrel hardware seems to need 0x1ff with 0x100 sign bit for sprite warp,
           this doesn't work on denjin makai as the visible area is larger */
		/*
        x&=0x1ff;
        y&=0xfff;

        if (x&0x100) x-=0x200;
        if (y&0x800) y-=0x1000;
        */

		x&=0xfff;
		y&=0xfff;

		if (x&0x800) x-=0x1000;
		if (y&0x800) y-=0x1000;

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
						drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,x+ax*16,y+ay*16,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,x+ax*16,y+(dy-ay-1)*16,15);
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
						drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,x+(dx-ax-1)*16,y+ay*16,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,x+(dx-ax-1)*16,y+(dy-ay-1)*16,15);
					}
			}
		}
	}
}

#define LAYER_DB 0

VIDEO_UPDATE( legionna )
{
	/* Setup the tilemaps */
	tilemap_set_scrollx( background_layer, 0, legionna_scrollram16[0] );
	tilemap_set_scrolly( background_layer, 0, legionna_scrollram16[1] );
	tilemap_set_scrollx( midground_layer,  0, legionna_scrollram16[2] );
	tilemap_set_scrolly( midground_layer,  0, legionna_scrollram16[3] );
	tilemap_set_scrollx( foreground_layer, 0, legionna_scrollram16[4] );
	tilemap_set_scrolly( foreground_layer, 0, legionna_scrollram16[5] );


	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));	/* wrong color? */

	/* legionna_layer_disable is a guess based on 'stage 1' screen in heatbrl  */

	if (!(legionna_layer_disable&0x0020)) tilemap_draw(bitmap,cliprect,foreground_layer,TILEMAP_DRAW_OPAQUE,0);

	if (!(legionna_layer_disable&0x0010)) tilemap_draw(bitmap,cliprect,midground_layer,0,0);

	draw_sprites(screen->machine, bitmap,cliprect,3);

	if (!(legionna_layer_disable&0x0002)) tilemap_draw(bitmap,cliprect,background_layer,0,0);

	draw_sprites(screen->machine,bitmap,cliprect,2);
	draw_sprites(screen->machine,bitmap,cliprect,1);
	draw_sprites(screen->machine,bitmap,cliprect,0);

	if (!(legionna_layer_disable&0x0001)) tilemap_draw(bitmap,cliprect,text_layer,0,0);

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

	bitmap_fill(bitmap,cliprect,0x0200);

	if (!(legionna_layer_disable&0x0001))
	{
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	}

	draw_sprites(screen->machine,bitmap,cliprect,2);

	if (!(legionna_layer_disable&0x0002))
	{
		tilemap_draw(bitmap,cliprect,midground_layer,0,0);
	}

	draw_sprites(screen->machine,bitmap,cliprect,1);

	if (!(legionna_layer_disable&0x0004))
	{
		tilemap_draw(bitmap,cliprect,foreground_layer,0,0);
	}

	draw_sprites(screen->machine,bitmap,cliprect,0);
	draw_sprites(screen->machine,bitmap,cliprect,3);

	if (!(legionna_layer_disable&0x0008))
	{
		tilemap_draw(bitmap,cliprect,text_layer,0,0);
	}

	return 0;
}

VIDEO_UPDATE( grainbow )
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

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if(!(grainbow_pri_n & 1))
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	draw_sprites(screen->machine,bitmap,cliprect,2);

	if(!(grainbow_pri_n & 2))
		tilemap_draw(bitmap,cliprect,midground_layer,0,0);

	draw_sprites(screen->machine,bitmap,cliprect,1);

	if(!(grainbow_pri_n & 4))
		tilemap_draw(bitmap,cliprect,foreground_layer,0,0);

	draw_sprites(screen->machine,bitmap,cliprect,0);

	draw_sprites(screen->machine,bitmap,cliprect,3);

	if(!(grainbow_pri_n & 8))
		tilemap_draw(bitmap,cliprect,text_layer,0,0);

	return 0;
}

