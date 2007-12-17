/*****************************************************************************

    Irem M90 system.  There is 1 video chip - NANAO GA-25, it produces
    2 tilemaps and sprites.  16 control bytes:

    0:  Playfield 1 X scroll
    2:  Playfield 1 Y scroll
    4:  Playfield 2 X scroll
    6:  Playfield 2 Y scroll
    8:  Bit 0x01 - unknown (set by hasamu)
    10: Playfield 1 control
        Bits0x03 - Playfield 1 VRAM base
        Bit 0x04 - Playfield 1 width (0 is 64 tiles, 0x4 is 128 tiles)
        Bit 0x10 - Playfield 1 disable
        Bit 0x20 - Playfield 1 rowscroll enable
    12: Playfield 2 control
        Bits0x03 - Playfield 2 VRAM base
        Bit 0x04 - Playfield 2 width (0 is 64 tiles, 0x4 is 128 tiles)
        Bit 0x10 - Playfield 2 disable
        Bit 0x20 - Playfield 2 rowscroll enable
    14: Bit 0x02 - unknown (set by hasamu)

    Emulation by Bryan McPhail, mish@tendril.co.uk, thanks to Chris Hardy!

*****************************************************************************/

#include "driver.h"

static UINT16 *m90_spriteram;
UINT16 *m90_video_data;
static UINT16 m90_video_control_data[8];
static tilemap *pf1_layer,*pf2_layer,*pf1_wide_layer,*pf2_wide_layer;

INLINE void get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int layer,int page_mask)
{
	int tile,color;
	tile_index = 2*tile_index + ((m90_video_control_data[5+layer] & page_mask) * 0x2000);

	tile=m90_video_data[tile_index];
	color=m90_video_data[tile_index+1];
	SET_TILE_INFO(
			0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
	tileinfo->category = (color & 0x30) ? 1 : 0;
}


static TILE_GET_INFO( get_pf1_tile_info ) { get_tile_info(machine,tileinfo,tile_index,0,0x3); }
static TILE_GET_INFO( get_pf1w_tile_info ) { get_tile_info(machine,tileinfo,tile_index,0,0x2); }
static TILE_GET_INFO( get_pf2_tile_info ) { get_tile_info(machine,tileinfo,tile_index,1,0x3); }
static TILE_GET_INFO( get_pf2w_tile_info ) { get_tile_info(machine,tileinfo,tile_index,1,0x2); }


VIDEO_START( m90 )
{
	pf1_layer =      tilemap_create(get_pf1_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);
	pf1_wide_layer = tilemap_create(get_pf1w_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,128,64);
	pf2_layer =      tilemap_create(get_pf2_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);
	pf2_wide_layer = tilemap_create(get_pf2w_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,128,64);

	tilemap_set_transparent_pen(pf1_layer,0);
	tilemap_set_transparent_pen(pf1_wide_layer,0);

	state_save_register_global_array(m90_video_control_data);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0x1f2/2;offs >= 0;offs -= 3)
	{
		int x,y,sprite,colour,fx,fy,y_multi,i;

		sprite = m90_spriteram[offs+1];
		colour = (m90_spriteram[offs+0] >> 9) & 0x0f;

		y = m90_spriteram[offs+0] & 0x1ff;
		x = m90_spriteram[offs+2] & 0x1ff;

		x = x - 16;
		y = 512 - y;

		fx = (m90_spriteram[offs+2] >> 8) & 0x02;
		fy = (m90_spriteram[offs+0] >> 8) & 0x80;

		y_multi = 1 << ((m90_spriteram[offs+0] & 0x6000) >> 13);
		y -= 16 * y_multi;

		for (i = 0;i < y_multi;i++)
			pdrawgfx(bitmap,machine->gfx[1],
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					cliprect,TRANSPARENCY_PEN,0,
					(colour & 0x08) ? 0x00 : 0x02);
	}
}

#if 0
static void bootleg_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0x0;offs <0x800-8;offs+= 8) {
		int x,y,sprite,colour,fx,fy;

		if (/*spriteram[offs+0]==0x78 &&*/ spriteram[offs+1]==0x7f) continue;

		y=(spriteram[offs+0] | (spriteram[offs+1]<<8))&0x1ff;
		x=(spriteram[offs+6] | (spriteram[offs+7]<<8))&0x1ff;

		x = x - /*64 -*/ 16;
		y = 256 - /*32 -*/ y;

	    sprite=(spriteram[offs+2] | (spriteram[offs+3]<<8));
		colour=(spriteram[offs+5]>>1)&0xf;

		fx=spriteram[offs+5]&1;
		fy=0;//spriteram[offs+5]&2;

		drawgfx(bitmap,machine->gfx[1],
				sprite&0x1fff,
				colour,
				fx,fy,
				x,y,
				cliprect,TRANSPARENCY_PEN,0);
	}
}
#endif

WRITE16_HANDLER( m90_video_control_w )
{
	COMBINE_DATA(&m90_video_control_data[offset]);
}


static void markdirty(tilemap *tmap,int page,offs_t offset)
{
	offset -= page * 0x2000;

	if (offset >= 0 && offset < 0x2000)
		tilemap_mark_tile_dirty(tmap,offset/2);
}


WRITE16_HANDLER( m90_video_w )
{
	COMBINE_DATA(&m90_video_data[offset]);

	markdirty(pf1_layer,     m90_video_control_data[5] & 0x3,offset);
	markdirty(pf1_wide_layer,m90_video_control_data[5] & 0x2,offset);
	markdirty(pf2_layer,     m90_video_control_data[6] & 0x3,offset);
	markdirty(pf2_wide_layer,m90_video_control_data[6] & 0x2,offset);
}

VIDEO_UPDATE( m90 )
{
	static int last_pf1,last_pf2;
	int pf1_base = m90_video_control_data[5] & 0x3;
	int pf2_base = m90_video_control_data[6] & 0x3;
	int i,pf1_enable,pf2_enable;

	if (m90_video_control_data[5]&0x10) pf1_enable=0; else pf1_enable=1;
	if (m90_video_control_data[6]&0x10) pf2_enable=0; else pf2_enable=1;
//  tilemap_set_enable(pf1_layer,pf1_enable);
//  tilemap_set_enable(pf2_layer,pf2_enable);
//  tilemap_set_enable(pf1_wide_layer,pf1_enable);
//  tilemap_set_enable(pf2_wide_layer,pf2_enable);

	/* Dirty tilemaps if VRAM base changes */
	if (pf1_base!=last_pf1)
	{
		tilemap_mark_all_tiles_dirty(pf1_layer);
		tilemap_mark_all_tiles_dirty(pf1_wide_layer);
	}
	if (pf2_base!=last_pf2)
	{
		tilemap_mark_all_tiles_dirty(pf2_layer);
		tilemap_mark_all_tiles_dirty(pf2_wide_layer);
	}
	last_pf1=pf1_base;
	last_pf2=pf2_base;

	m90_spriteram=m90_video_data+0xee00/2;

	/* Setup scrolling */
	if (m90_video_control_data[5]&0x20)
	{
		tilemap_set_scroll_rows(pf1_layer,512);
		tilemap_set_scroll_rows(pf1_wide_layer,512);
		for (i=0; i<512; i++)
			tilemap_set_scrollx( pf1_layer,i, m90_video_data[0xf000/2+i]+2);
		for (i=0; i<512; i++)
			tilemap_set_scrollx( pf1_wide_layer,i, m90_video_data[0xf000/2+i]+256+2);
	}
	else
	{
		tilemap_set_scroll_rows(pf1_layer,1);
		tilemap_set_scroll_rows(pf1_wide_layer,1);
		tilemap_set_scrollx( pf1_layer,0, m90_video_control_data[1]+2);
		tilemap_set_scrollx( pf1_wide_layer,0, m90_video_control_data[1]+256+2);
	}

	/* Setup scrolling */
	if (m90_video_control_data[6]&0x20) {
		tilemap_set_scroll_rows(pf2_layer,512);
		tilemap_set_scroll_rows(pf2_wide_layer,512);
		for (i=0; i<512; i++)
			tilemap_set_scrollx( pf2_layer,i, m90_video_data[0xf400/2+i]-2);
		for (i=0; i<512; i++)
			tilemap_set_scrollx( pf2_wide_layer,i, m90_video_data[0xf400/2+i]+256-2);
	} else {
		tilemap_set_scroll_rows(pf2_layer,1);
		tilemap_set_scroll_rows(pf2_wide_layer,1);
		tilemap_set_scrollx( pf2_layer,0, m90_video_control_data[3]-2);
		tilemap_set_scrollx( pf2_wide_layer,0, m90_video_control_data[3]+256-2 );
	}

	tilemap_set_scrolly( pf1_layer,0, m90_video_control_data[0] );
	tilemap_set_scrolly( pf2_layer,0, m90_video_control_data[2] );
	tilemap_set_scrolly( pf1_wide_layer,0, m90_video_control_data[0] );
	tilemap_set_scrolly( pf2_wide_layer,0, m90_video_control_data[2] );

	fillbitmap(priority_bitmap,0,cliprect);

	if (!pf2_enable)
		fillbitmap(bitmap,machine->pens[0],cliprect);

	if (pf2_enable)
	{
		if (m90_video_control_data[6] & 0x4)
			tilemap_draw(bitmap,cliprect,pf2_wide_layer,0,0);
		else
			tilemap_draw(bitmap,cliprect,pf2_layer,0,0);

		if (m90_video_control_data[6] & 0x4)
			tilemap_draw(bitmap,cliprect,pf2_wide_layer,1,1);
		else
			tilemap_draw(bitmap,cliprect,pf2_layer,1,1);
	}

	if (pf1_enable)
	{
		if (m90_video_control_data[5] & 0x4)
			tilemap_draw(bitmap,cliprect,pf1_wide_layer,0,0);
		else
			tilemap_draw(bitmap,cliprect,pf1_layer,0,0);

		if (m90_video_control_data[5] & 0x4)
			tilemap_draw(bitmap,cliprect,pf1_wide_layer,1,1);
		else
			tilemap_draw(bitmap,cliprect,pf1_layer,1,1);
	}

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
