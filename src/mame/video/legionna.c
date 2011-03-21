/***************************************************************************

    Legionnaire / Heated Barrel video hardware (derived from D-Con)

***************************************************************************/

#include "emu.h"
#include "includes/legionna.h"


/******************************************************************************/

void heatbrl_setgfxbank(running_machine *machine, UINT16 data)
{
	legionna_state *state = machine->driver_data<legionna_state>();
	state->back_gfx_bank = (data &0x4000) >> 2;
}

/*xxx- --- ---- ---- banking*/
void denjinmk_setgfxbank(running_machine *machine, UINT16 data)
{
	legionna_state *state = machine->driver_data<legionna_state>();
	state->fore_gfx_bank = (data &0x2000) >> 1;//???
	state->back_gfx_bank = (data &0x4000) >> 2;
	state->mid_gfx_bank  = (data &0x8000) >> 3;//???

	tilemap_mark_all_tiles_dirty (state->background_layer);
	tilemap_mark_all_tiles_dirty (state->foreground_layer);
	tilemap_mark_all_tiles_dirty (state->midground_layer);
	tilemap_mark_all_tiles_dirty (state->text_layer);
}

WRITE16_HANDLER( legionna_background_w )
{
	legionna_state *state = space->machine->driver_data<legionna_state>();
	COMBINE_DATA(&state->back_data[offset]);
	tilemap_mark_tile_dirty(state->background_layer,offset);
}

WRITE16_HANDLER( legionna_midground_w )
{
	legionna_state *state = space->machine->driver_data<legionna_state>();
	COMBINE_DATA(&state->mid_data[offset]);
	tilemap_mark_tile_dirty(state->midground_layer,offset);
}

WRITE16_HANDLER( legionna_foreground_w )
{
	legionna_state *state = space->machine->driver_data<legionna_state>();
	COMBINE_DATA(&state->fore_data[offset]);
	tilemap_mark_tile_dirty(state->foreground_layer,offset);
}

WRITE16_HANDLER( legionna_text_w )
{
	legionna_state *state = space->machine->driver_data<legionna_state>();
	COMBINE_DATA(&state->textram[offset]);
	tilemap_mark_tile_dirty(state->text_layer,offset);
}

static TILE_GET_INFO( get_back_tile_info )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile=state->back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;
	tile |= state->back_gfx_bank;		/* Heatbrl uses banking */

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile=state->mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(5,tile,color,0);
}

static TILE_GET_INFO( get_mid_tile_info_denji )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile=state->mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;
	tile |= state->mid_gfx_bank;

	SET_TILE_INFO(5,tile,color,0);
}

static TILE_GET_INFO( get_mid_tile_info_cupsoc )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile=state->mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	tile |= 0x1000;
	color += 0x10;

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info )	/* this is giving bad tiles... */
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile=state->fore_data[tile_index];
	int color=(tile>>12)&0xf;

	// legionnaire tile numbers / gfx set wrong, see screen after coin insertion
	tile &= 0xfff;

	SET_TILE_INFO(4,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info_denji )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile=state->fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;
	tile |= state->fore_gfx_bank;

	SET_TILE_INFO(4,tile,color,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	int tile = state->textram[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(0,tile,color,0);
}

VIDEO_START( legionna )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	state->background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	state->foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	state->midground_layer =  tilemap_create(machine, get_mid_tile_info, tilemap_scan_rows,16,16,32,32);
	state->text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);

	state->scrollram16 = auto_alloc_array(machine, UINT16, 0x60/2);
	state->sprite_xoffs = 0;
	state->sprite_yoffs = 0;

	state->has_extended_banking = 0;
	state->has_extended_priority = 0;

	tilemap_set_transparent_pen(state->background_layer,15);
	tilemap_set_transparent_pen(state->midground_layer,15);
	tilemap_set_transparent_pen(state->foreground_layer,15);
	tilemap_set_transparent_pen(state->text_layer,15);
}

VIDEO_START( denjinmk )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	state->background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	state->foreground_layer = tilemap_create(machine, get_fore_tile_info_denji,tilemap_scan_rows,16,16,32,32);
	state->midground_layer =  tilemap_create(machine, get_mid_tile_info_denji, tilemap_scan_rows,16,16,32,32);
	state->text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);

	state->scrollram16 = auto_alloc_array(machine, UINT16, 0x60/2);
	state->sprite_xoffs = 0;
	state->sprite_yoffs = 0;

	state->has_extended_banking = 1;
	state->has_extended_priority = 0;

	tilemap_set_transparent_pen(state->background_layer,15);
	tilemap_set_transparent_pen(state->midground_layer,15);
	tilemap_set_transparent_pen(state->foreground_layer,15);
	tilemap_set_transparent_pen(state->text_layer,7);//?
}

VIDEO_START( cupsoc )
{
	legionna_state *state = machine->driver_data<legionna_state>();
	state->background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	state->foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	state->midground_layer =  tilemap_create(machine, get_mid_tile_info_cupsoc, tilemap_scan_rows,16,16,32,32);
	state->text_layer =       tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);

	state->scrollram16 = auto_alloc_array(machine, UINT16, 0x60/2);
	state->sprite_xoffs = 0;
	state->sprite_yoffs = 0;

	state->has_extended_banking = 0;
	state->has_extended_priority = 1;

	tilemap_set_transparent_pen(state->background_layer,15);
	tilemap_set_transparent_pen(state->midground_layer,15);
	tilemap_set_transparent_pen(state->foreground_layer,15);
	tilemap_set_transparent_pen(state->text_layer,15);
}

VIDEO_START(grainbow)
{
	legionna_state *state = machine->driver_data<legionna_state>();
	VIDEO_START_CALL(legionna);
	state->sprite_xoffs = state->sprite_yoffs = 16;

	state->has_extended_banking = 0;
	state->has_extended_priority = 1;
}

VIDEO_START(godzilla)
{
	legionna_state *state = machine->driver_data<legionna_state>();
	VIDEO_START_CALL(legionna);

	state->has_extended_banking = 1;
	state->has_extended_priority = 0;

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
    +0   ........ .x......  Tile bank,used in Denjin Makai / extra Priority in Grainbow (to external pin?)
    +0   ........ ..xxxxxx  Color bank

    +1   xx...... ........  Priority? (1=high?)
    +1   ..xxxxxx xxxxxxxx  Tile number

    +2   ----xxxx xxxxxxxx  X coordinate (signed)

    +3   b------- --------  more tile banking used by Denjin Makai
    +3   ----xxxx xxxxxxxx  Y coordinate (signed)

*************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	legionna_state *state = machine->driver_data<legionna_state>();
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs,fx,fy,x,y,color,sprite,cur_pri;
	int dx,dy,ax,ay;
	int pri_mask;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		UINT16 data = spriteram16[offs];
		if (!(data &0x8000)) continue;

		pri_mask = 0;

		if (state->has_extended_priority)
		{

			cur_pri = (spriteram16[offs+1] & 0xc000) >> 14;

			if(data & 0x0040)
			{
				cur_pri |= 0x4; // definitely seems to be needed by grainbow
			}

			//
			// -4 behind bg? (mask sprites)
			// -32 behind mid
			// -256 behind tx
			// 0    above all

			// is the low bit REALLY priority?

			switch (cur_pri)
			{
				case 0:	pri_mask = -256; break; // gumdam swamp monster l2
				case 1:	pri_mask = -256; break; // cupsoc
				case 2:	pri_mask = -4; break; // masking effect for gundam l2 monster
				case 3: pri_mask = -4; break; // cupsoc (not sure what..)
				case 4: pri_mask = -32; break; // gundam level 2/3 player
				//case 5: pri_mask = 0; break;
				case 6: pri_mask = 0; break; // insert coin in gundam
				//case 7: pri_mask = 0; break;

				default: printf("unhandled pri %d\n",cur_pri); pri_mask=0;
			}

		}
		else
		{
			cur_pri = (spriteram16[offs+1] & 0xc000) >> 14;

			switch (cur_pri)
			{
				case 0:	pri_mask = 0xfffc; break; //?
				case 1:	pri_mask = 0xfffc; break; //?
				case 2:	pri_mask = 0xfffc; break; // player sprites in legionnaire
				case 3: pri_mask = 0xfffe; break; // stuff that goes behind the playfield (barriers on train level in legionnaire)
			}

		}

		sprite = spriteram16[offs+1];

		sprite &= 0x3fff;

		if (state->has_extended_banking)
		{
			if(data & 0x0040)
			{
				sprite |= 0x4000;//tile banking,used in Denjin Makai
			}
			if(spriteram16[offs+3] & 0x8000)
			{
				sprite |= 0x8000;//tile banking?,used in Denjin Makai
			}
		}


		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		/* heated barrel hardware seems to need 0x1ff with 0x100 sign bit for sprite warp,
           this doesn't work on denjin makai as the visible area is larger */
		if (cliprect->max_x<(320-1))
		{
			x&=0x1ff;
			y&=0x1ff;

			if (x&0x100) x-=0x200;
			if (y&0x100) y-=0x200;
		}
		else
		{
			x&=0xfff;
			y&=0xfff;

			if (x&0x800) x-=0x1000;
			if (y&0x800) y-=0x1000;

		}


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
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,(x+ax*16)+state->sprite_xoffs,y+ay*16+state->sprite_yoffs,
						machine->priority_bitmap,pri_mask, 15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,(x+ax*16)+state->sprite_xoffs,y+(dy-ay-1)*16+state->sprite_yoffs,
						machine->priority_bitmap,pri_mask,15);
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
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,(x+(dx-ax-1)*16)+state->sprite_xoffs,y+ay*16+state->sprite_yoffs,
						machine->priority_bitmap,pri_mask,15);
					}
			}
			else
			{
				for (ax=0; ax<dx; ax++)
					for (ay=0; ay<dy; ay++)
					{
						pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite++,
						color,fx,fy,(x+(dx-ax-1)*16)+state->sprite_xoffs,y+(dy-ay-1)*16+state->sprite_yoffs,
						machine->priority_bitmap,pri_mask, 15);
					}
			}
		}
	}
}

#define LAYER_DB 0

SCREEN_UPDATE( legionna )
{
	legionna_state *state = screen->machine->driver_data<legionna_state>();
	/* Setup the tilemaps */
	tilemap_set_scrollx( state->background_layer, 0, state->scrollram16[0] );
	tilemap_set_scrolly( state->background_layer, 0, state->scrollram16[1] );
	tilemap_set_scrollx( state->midground_layer,  0, state->scrollram16[2] );
	tilemap_set_scrolly( state->midground_layer,  0, state->scrollram16[3] );
	tilemap_set_scrollx( state->foreground_layer, 0, state->scrollram16[4] );
	tilemap_set_scrolly( state->foreground_layer, 0, state->scrollram16[5] );
	tilemap_set_scrollx( state->text_layer, 0,  0/*state->scrollram16[6]*/ );
	tilemap_set_scrolly( state->text_layer, 0,  0/*state->scrollram16[7]*/ );

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));	/* wrong color? */

	/* state->layer_disable is a guess based on 'stage 1' screen in heatbrl  */

	if (!(state->layer_disable&0x0020)) tilemap_draw(bitmap,cliprect,state->foreground_layer,0, 0);
	if (!(state->layer_disable&0x0010)) tilemap_draw(bitmap,cliprect,state->midground_layer,0, 0);
	if (!(state->layer_disable&0x0002)) tilemap_draw(bitmap,cliprect,state->background_layer,0, 1);
	if (!(state->layer_disable&0x0001)) tilemap_draw(bitmap,cliprect,state->text_layer,0, 2);

	draw_sprites(screen->machine,bitmap,cliprect);


	return 0;
}


SCREEN_UPDATE( godzilla )
{
	legionna_state *state = screen->machine->driver_data<legionna_state>();
//  tilemap_set_scrollx( state->text_layer, 0, 0 );
//  tilemap_set_scrolly( state->text_layer, 0, 112 );
	/* Setup the tilemaps */
	tilemap_set_scrollx( state->background_layer, 0, state->scrollram16[0] );
	tilemap_set_scrolly( state->background_layer, 0, state->scrollram16[1] );
	tilemap_set_scrollx( state->midground_layer,  0, state->scrollram16[2] );
	tilemap_set_scrolly( state->midground_layer,  0, state->scrollram16[3] );
	tilemap_set_scrollx( state->foreground_layer, 0, state->scrollram16[4] );
	tilemap_set_scrolly( state->foreground_layer, 0, state->scrollram16[5] );
	tilemap_set_scrollx( state->text_layer, 0,  0/*state->scrollram16[6]*/ );
	tilemap_set_scrolly( state->text_layer, 0,  0/*state->scrollram16[7]*/ );


	bitmap_fill(bitmap,cliprect,0x0200);
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	if (!(state->layer_disable&0x0001)) tilemap_draw(bitmap,cliprect,state->background_layer,0,0);
	if (!(state->layer_disable&0x0002)) tilemap_draw(bitmap,cliprect,state->midground_layer,0,0);
	if (!(state->layer_disable&0x0004)) tilemap_draw(bitmap,cliprect,state->foreground_layer,0,1);
	if (!(state->layer_disable&0x0008)) tilemap_draw(bitmap,cliprect,state->text_layer,0,2);

	draw_sprites(screen->machine,bitmap,cliprect);

	return 0;
}

SCREEN_UPDATE( grainbow )
{
	legionna_state *state = screen->machine->driver_data<legionna_state>();
	/* Setup the tilemaps */
	tilemap_set_scrollx( state->background_layer, 0, state->scrollram16[0] );
	tilemap_set_scrolly( state->background_layer, 0, state->scrollram16[1] );
	tilemap_set_scrollx( state->midground_layer,  0, state->scrollram16[2] );
	tilemap_set_scrolly( state->midground_layer,  0, state->scrollram16[3] );
	tilemap_set_scrollx( state->foreground_layer, 0, state->scrollram16[4] );
	tilemap_set_scrolly( state->foreground_layer, 0, state->scrollram16[5] );
	tilemap_set_scrollx( state->text_layer, 0,  0/*state->scrollram16[6]*/ );
	tilemap_set_scrolly( state->text_layer, 0,  0/*state->scrollram16[7]*/ );

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	if(!(state->layer_disable & 1))
		tilemap_draw(bitmap,cliprect,state->background_layer,0,1);

	if(!(state->layer_disable & 2))
		tilemap_draw(bitmap,cliprect,state->midground_layer,0,2);

	if(!(state->layer_disable & 4))
		tilemap_draw(bitmap,cliprect,state->foreground_layer,0,4);

	if(!(state->layer_disable & 8))
		tilemap_draw(bitmap,cliprect,state->text_layer,0,8);

	draw_sprites(screen->machine,bitmap,cliprect);

	return 0;
}

