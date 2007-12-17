#include "driver.h"

static tilemap *pf3_layer,*pf2_layer,*pf1_layer,*text_layer;
static int deadangle_tilebank, deadangle_oldtilebank;
UINT16 *deadang_video_data,*deadang_scroll_ram;

/******************************************************************************/

WRITE16_HANDLER( deadang_foreground_w )
{
	COMBINE_DATA(&deadang_video_data[offset]);
	tilemap_mark_tile_dirty( pf1_layer, offset );
}

WRITE16_HANDLER( deadang_text_w )
{
	COMBINE_DATA(&videoram16[offset]);
	tilemap_mark_tile_dirty( text_layer, offset );
}

WRITE16_HANDLER( deadang_bank_w )
{
	if (ACCESSING_LSB)
	{
		deadangle_tilebank = data&1;
		if (deadangle_tilebank!=deadangle_oldtilebank)
		{
			deadangle_oldtilebank = deadangle_tilebank;
			tilemap_mark_all_tiles_dirty (pf1_layer);
		}
	}
}

/******************************************************************************/

static TILEMAP_MAPPER( bg_scan )
{
	return (col&0xf) | ((row&0xf)<<4) | ((col&0x70)<<4) | ((row&0xf0)<<7);
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	const UINT16 *bgMap = (const UINT16 *)memory_region(REGION_GFX6);
	int code= bgMap[tile_index];
	SET_TILE_INFO(4,code&0x7ff,code>>12,0);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	const UINT16 *bgMap = (const UINT16 *)memory_region(REGION_GFX7);
	int code= bgMap[tile_index];
	SET_TILE_INFO(3,code&0x7ff,code>>12,0);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	int tile=deadang_video_data[tile_index];
	int color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(2,tile+deadangle_tilebank*0x1000,color,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile=(videoram16[tile_index] & 0xff) | ((videoram16[tile_index] >> 6) & 0x300);
	int color=(videoram16[tile_index] >> 8)&0xf;

	SET_TILE_INFO(0,tile,color,0);
}

VIDEO_START( deadang )
{
	pf3_layer = tilemap_create(get_pf3_tile_info,bg_scan,          TILEMAP_TYPE_PEN,     16,16,128,256);
	pf2_layer = tilemap_create(get_pf2_tile_info,bg_scan,          TILEMAP_TYPE_PEN,16,16,128,256);
	pf1_layer = tilemap_create(get_pf1_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,16,16, 32, 32);
	text_layer = tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(pf2_layer, 15);
	tilemap_set_transparent_pen(pf1_layer, 15);
	tilemap_set_transparent_pen(text_layer, 15);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs,fx,fy,x,y,color,sprite,pri;

	for (offs = 0; offs<0x800/2; offs+=4)
	{
		/* Don't draw empty sprite table entries */
		if ((spriteram16[offs+3] & 0xff00)!=0xf00) continue;

		switch (spriteram16[offs+2]&0xc000) {
		default:
		case 0xc000: pri=0; break; /* Unknown */
		case 0x8000: pri=0; break; /* Over all playfields */
		case 0x4000: pri=0xf0; break; /* Under top playfield */
		case 0x0000: pri=0xf0|0xcc; break; /* Under middle playfield */
		}

		fx= spriteram16[offs+0]&0x2000;
		fy= spriteram16[offs+0]&0x4000;
		y = spriteram16[offs+0] & 0xff;
		x = spriteram16[offs+2] & 0xff;
		if (fy) fy=0; else fy=1;
		if (spriteram16[offs+2]&0x100) x=0-(0xff-x);

		color = (spriteram16[offs+1]>>12)&0xf;
		sprite = spriteram16[offs+1]&0xfff;

		if (flip_screen) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}

		pdrawgfx(bitmap,machine->gfx[1],
				sprite,
				color,fx,fy,x,y,
				cliprect,TRANSPARENCY_PEN,15,pri);
	}
}

VIDEO_UPDATE( deadang )
{
	/* Setup the tilemaps */
	tilemap_set_scrolly( pf3_layer,0, ((deadang_scroll_ram[0x01]&0xf0)<<4)+((deadang_scroll_ram[0x02]&0x7f)<<1)+((deadang_scroll_ram[0x02]&0x80)>>7) );
	tilemap_set_scrollx( pf3_layer,0, ((deadang_scroll_ram[0x09]&0xf0)<<4)+((deadang_scroll_ram[0x0a]&0x7f)<<1)+((deadang_scroll_ram[0x0a]&0x80)>>7) );
	tilemap_set_scrolly( pf1_layer,0, ((deadang_scroll_ram[0x11]&0x10)<<4)+((deadang_scroll_ram[0x12]&0x7f)<<1)+((deadang_scroll_ram[0x12]&0x80)>>7) );
	tilemap_set_scrollx( pf1_layer,0, ((deadang_scroll_ram[0x19]&0x10)<<4)+((deadang_scroll_ram[0x1a]&0x7f)<<1)+((deadang_scroll_ram[0x1a]&0x80)>>7) );
	tilemap_set_scrolly( pf2_layer,0, ((deadang_scroll_ram[0x21]&0xf0)<<4)+((deadang_scroll_ram[0x22]&0x7f)<<1)+((deadang_scroll_ram[0x22]&0x80)>>7) );
	tilemap_set_scrollx( pf2_layer,0, ((deadang_scroll_ram[0x29]&0xf0)<<4)+((deadang_scroll_ram[0x2a]&0x7f)<<1)+((deadang_scroll_ram[0x2a]&0x80)>>7) );

	/* Control byte:
        0x01: Background playfield disable
        0x02: Middle playfield disable
        0x04: Top playfield disable
        0x08: ?  Toggles at start of game
        0x10: Sprite disable
        0x20: Unused?
        0x40: Flipscreen
        0x80: Always set?
    */
	tilemap_set_enable(pf3_layer,!(deadang_scroll_ram[0x34]&1));
	tilemap_set_enable(pf1_layer,!(deadang_scroll_ram[0x34]&2));
	tilemap_set_enable(pf2_layer,!(deadang_scroll_ram[0x34]&4));
	flip_screen_set( deadang_scroll_ram[0x34]&0x40 );

	fillbitmap(bitmap,get_black_pen(machine),cliprect);
	fillbitmap(priority_bitmap,0,cliprect);
	tilemap_draw(bitmap,cliprect,pf3_layer,0,1);
	tilemap_draw(bitmap,cliprect,pf1_layer,0,2);
	tilemap_draw(bitmap,cliprect,pf2_layer,0,4);
	if (!(deadang_scroll_ram[0x34]&0x10)) draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,text_layer,0,0);
	return 0;
}
