/***************************************************************************

    Goal '92 video hardware

***************************************************************************/

#include "driver.h"

UINT16 *goal92_back_data,*goal92_fore_data,*goal92_textram,*goal92_scrollram16;

static tilemap *background_layer,*foreground_layer,*text_layer;

static UINT16 fg_bank = 0;

READ16_HANDLER( goal92_fg_bank_r )
{
	return fg_bank;
}

WRITE16_HANDLER( goal92_fg_bank_w )
{
	COMBINE_DATA(&fg_bank);

	if(ACCESSING_BITS_0_7)
	{
		tilemap_mark_all_tiles_dirty(foreground_layer);
	}
}

WRITE16_HANDLER( goal92_text_w )
{
	COMBINE_DATA(&goal92_textram[offset]);
	tilemap_mark_tile_dirty(text_layer,offset);
}

WRITE16_HANDLER( goal92_background_w )
{
	COMBINE_DATA(&goal92_back_data[offset]);
	tilemap_mark_tile_dirty(background_layer,offset);
}

WRITE16_HANDLER( goal92_foreground_w )
{
	COMBINE_DATA(&goal92_fore_data[offset]);
	tilemap_mark_tile_dirty(foreground_layer,offset);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = goal92_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	tile |= 0xc000;

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_back_tile_info )
{
	int tile=goal92_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(2,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	int tile=goal92_fore_data[tile_index];
	int color=(tile>>12)&0xf;
	int region;

	tile &= 0xfff;

	if(fg_bank & 0xff)
	{
		region = 3;
		tile |= 0x1000;
	}
	else
	{
		region = 4;
		tile |= 0x2000;
	}

	SET_TILE_INFO(region,tile,color,0);
}

static void draw_sprites(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int pri)
{
	int offs,fx,fy,x,y,color,sprite;

	for (offs = 3;offs <= 0x400-5;offs += 4)
	{
		UINT16 data = buffered_spriteram16[offs+2];

		y = buffered_spriteram16[offs+0];

		if (y & 0x8000)
			break;

		if (!(data & 0x8000))
			continue;

		sprite = buffered_spriteram16[offs+1];

		if ((sprite>>14)!=pri)
			continue;

		x = buffered_spriteram16[offs+3];

		sprite &= 0x1fff;

		x &= 0x1ff;
		y &= 0x1ff;

		color = (data & 0x3f) + 0x40;
		fx = (data & 0x4000) >> 14;
		fy = 0;

		x -= 320/4-16-1;

		y = 256-(y+7);

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				sprite,
				color,fx,fy,x,y,15);
	}
}


VIDEO_START( goal92 )
{
	background_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_rows,16,16,32,32);
	foreground_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_rows,16,16,32,32);
	text_layer       = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,  8,8,64,32);

	buffered_spriteram16 = auto_alloc_array(machine, UINT16, 0x400*2);

	tilemap_set_transparent_pen(background_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,15);
}

VIDEO_UPDATE( goal92 )
{
	tilemap_set_scrollx(background_layer, 0, goal92_scrollram16[0] + 60);
	tilemap_set_scrolly(background_layer, 0, goal92_scrollram16[1] + 8);

	if(fg_bank & 0xff)
	{
		tilemap_set_scrollx(foreground_layer, 0, goal92_scrollram16[0] + 60);
		tilemap_set_scrolly(foreground_layer, 0, goal92_scrollram16[1] + 8);
	}
	else
	{
		tilemap_set_scrollx(foreground_layer, 0, goal92_scrollram16[2] + 60);
		tilemap_set_scrolly(foreground_layer, 0, goal92_scrollram16[3] + 8);
	}

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	tilemap_draw(bitmap,cliprect,background_layer,0,0);
	draw_sprites(screen->machine,bitmap,cliprect,2);

	if(!(fg_bank & 0xff))
		draw_sprites(screen->machine,bitmap,cliprect,1);

	tilemap_draw(bitmap,cliprect,foreground_layer,0,0);

	if(fg_bank & 0xff)
		draw_sprites(screen->machine,bitmap,cliprect,1);

	draw_sprites(screen->machine,bitmap,cliprect,0);
	draw_sprites(screen->machine,bitmap,cliprect,3);
	tilemap_draw(bitmap,cliprect,text_layer,0,0);
	return 0;
}

VIDEO_EOF( goal92 )
{
	memcpy(buffered_spriteram16,spriteram16,0x400*2);
}
