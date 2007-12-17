/***************************************************************************

    D-Con video hardware.

***************************************************************************/

#include "driver.h"

UINT16 *dcon_back_data,*dcon_fore_data,*dcon_mid_data,*dcon_scroll_ram,*dcon_textram;

static tilemap *background_layer,*foreground_layer,*midground_layer,*text_layer;
static UINT16 dcon_enable;
static int dcon_gfx_bank_select;

/******************************************************************************/

READ16_HANDLER( dcon_control_r )
{
	return dcon_enable;
}

WRITE16_HANDLER( dcon_control_w )
{
	if (ACCESSING_LSB)
	{
		dcon_enable=data;
		if ((dcon_enable&4)==4)
			tilemap_set_enable(foreground_layer,0);
		else
			tilemap_set_enable(foreground_layer,1);

		if ((dcon_enable&2)==2)
			tilemap_set_enable(midground_layer,0);
		else
			tilemap_set_enable(midground_layer,1);

		if ((dcon_enable&1)==1)
			tilemap_set_enable(background_layer,0);
		else
			tilemap_set_enable(background_layer,1);
	}
}

WRITE16_HANDLER( dcon_gfxbank_w )
{
	if (data&1)
		dcon_gfx_bank_select=0x1000;
	else
		dcon_gfx_bank_select=0;
}

WRITE16_HANDLER( dcon_background_w )
{
	COMBINE_DATA(&dcon_back_data[offset]);
	tilemap_mark_tile_dirty(background_layer,offset);
}

WRITE16_HANDLER( dcon_foreground_w )
{
	COMBINE_DATA(&dcon_fore_data[offset]);
	tilemap_mark_tile_dirty(foreground_layer,offset);
}

WRITE16_HANDLER( dcon_midground_w )
{
	COMBINE_DATA(&dcon_mid_data[offset]);
	tilemap_mark_tile_dirty(midground_layer,offset);
}

WRITE16_HANDLER( dcon_text_w )
{
	COMBINE_DATA(&dcon_textram[offset]);
	tilemap_mark_tile_dirty(text_layer,offset);
}

static TILE_GET_INFO( get_back_tile_info )
{
	int tile=dcon_back_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	int tile=dcon_fore_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	int tile=dcon_mid_data[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			3,
			tile|dcon_gfx_bank_select,
			color,
			0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = dcon_textram[tile_index];
	int color=(tile>>12)&0xf;

	tile&=0xfff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( dcon )
{
	background_layer = tilemap_create(get_back_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,32,32);
	foreground_layer = tilemap_create(get_fore_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	midground_layer =  tilemap_create(get_mid_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	text_layer =       tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,  8,8,64,32);

	tilemap_set_transparent_pen(midground_layer,15);
	tilemap_set_transparent_pen(foreground_layer,15);
	tilemap_set_transparent_pen(text_layer,15);

	dcon_gfx_bank_select = 0;
}

static void draw_sprites(running_machine* machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay,inc,pri_mask = 0;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		if ((spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = spriteram16[offs+1];

		switch((sprite>>14) & 3)
		{
		case 0: pri_mask = 0xf0; // above foreground layer
			break;
		case 1: pri_mask = 0xfc; // above midground layer
			break;
		case 2: pri_mask = 0xfe; // above background layer
			break;
		case 3: pri_mask = 0; // above text layer
			break;
		}

		sprite &= 0x3fff;

		y = spriteram16[offs+3];
		x = spriteram16[offs+2];

		if (x&0x8000) x=0-(0x200-(x&0x1ff));
		else x&=0x1ff;
		if (y&0x8000) y=0-(0x200-(y&0x1ff));
		else y&=0x1ff;

		color = spriteram16[offs+0]&0x3f;
		fx = spriteram16[offs+0]&0x4000;
		fy = spriteram16[offs+0]&0x2000;
		dy=((spriteram16[offs+0]&0x0380)>>7)+1;
		dx=((spriteram16[offs+0]&0x1c00)>>10)+1;

		inc = 0;

		for (ax=0; ax<dx; ax++)
			for (ay=0; ay<dy; ay++) {
				if (!fx && !fy)
				{
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 + 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+ay*16 - 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);
				}
				else if (fx && !fy)
				{
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 + 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16 - 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);
				}
				else if (!fx && fy)
				{
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 + 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+ax*16,y+(dy-1-ay)*16 - 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);
				}
				else
				{
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 + 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);

					// wrap around y
					pdrawgfx(bitmap,machine->gfx[4],
						sprite + inc,
						color,fx,fy,x+(dx-1-ax)*16,y+(dy-1-ay)*16 - 512,
						cliprect,TRANSPARENCY_PEN,15,pri_mask);
				}

				inc++;
			}
	}
}

VIDEO_UPDATE( dcon )
{
	fillbitmap(priority_bitmap,0,cliprect);

	/* Setup the tilemaps */
	tilemap_set_scrollx( background_layer,0, dcon_scroll_ram[0] );
	tilemap_set_scrolly( background_layer,0, dcon_scroll_ram[1] );
	tilemap_set_scrollx( midground_layer, 0, dcon_scroll_ram[2] );
	tilemap_set_scrolly( midground_layer, 0, dcon_scroll_ram[3] );
	tilemap_set_scrollx( foreground_layer,0, dcon_scroll_ram[4] );
	tilemap_set_scrolly( foreground_layer,0, dcon_scroll_ram[5] );

	if ((dcon_enable&1)!=1)
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	else
		fillbitmap(bitmap,machine->pens[15],cliprect); /* Should always be black, not pen 15 */

	tilemap_draw(bitmap,cliprect,midground_layer,0,1);
	tilemap_draw(bitmap,cliprect,foreground_layer,0,2);
	tilemap_draw(bitmap,cliprect,text_layer,0,4);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( sdgndmps )
{
	static int last_gfx_bank=0;

	fillbitmap(priority_bitmap,0,cliprect);

	/* Gfx banking */
	if (last_gfx_bank!=dcon_gfx_bank_select)
	{
		tilemap_mark_all_tiles_dirty(midground_layer);
		last_gfx_bank=dcon_gfx_bank_select;
	}

	/* Setup the tilemaps */
	tilemap_set_scrollx( background_layer,0, dcon_scroll_ram[0]+128 );
	tilemap_set_scrolly( background_layer,0, dcon_scroll_ram[1] );
	tilemap_set_scrollx( midground_layer, 0, dcon_scroll_ram[2]+128 );
	tilemap_set_scrolly( midground_layer, 0, dcon_scroll_ram[3] );
	tilemap_set_scrollx( foreground_layer,0, dcon_scroll_ram[4]+128 );
	tilemap_set_scrolly( foreground_layer,0, dcon_scroll_ram[5] );
	tilemap_set_scrollx( text_layer,0, /*dcon_scroll_ram[6] + */ 128 );
	tilemap_set_scrolly( text_layer,0, /*dcon_scroll_ram[7] + */ 0 );

	if ((dcon_enable&1)!=1)
		tilemap_draw(bitmap,cliprect,background_layer,0,0);
	else
		fillbitmap(bitmap,machine->pens[15],cliprect); /* Should always be black, not pen 15 */

	tilemap_draw(bitmap,cliprect,midground_layer,0,1);
	tilemap_draw(bitmap,cliprect,foreground_layer,0,2);
	tilemap_draw(bitmap,cliprect,text_layer,0,4);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
