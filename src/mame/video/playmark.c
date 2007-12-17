#include "driver.h"

UINT16 *bigtwin_bgvideoram;
UINT16 *wbeachvl_videoram1,*wbeachvl_videoram2,*wbeachvl_videoram3;
UINT16 *wbeachvl_rowscroll;

static int bgscrollx,bgscrolly,bg_enable,bg_full_size;
static int fgscrollx,fg_rowscroll_enable;
static tilemap *tx_tilemap,*fg_tilemap,*bg_tilemap;

static int xoffset = 0;
static int yoffset = 0;
static int txt_tile_offset = 0;
static int pri_masks[3];
static UINT16 playmark_scroll[7];


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( bigtwin_get_tx_tile_info )
{
	UINT16 code = wbeachvl_videoram1[2*tile_index];
	UINT16 color = wbeachvl_videoram1[2*tile_index+1];
	SET_TILE_INFO(
			2,
			code,
			color,
			0);
}

static TILE_GET_INFO( bigtwin_get_fg_tile_info )
{
	UINT16 code = wbeachvl_videoram2[2*tile_index];
	UINT16 color = wbeachvl_videoram2[2*tile_index+1];
	SET_TILE_INFO(
			1,
			code,
			color,
			0);
}

static TILE_GET_INFO( wbeachvl_get_tx_tile_info )
{
	UINT16 code = wbeachvl_videoram1[2*tile_index];
	UINT16 color = wbeachvl_videoram1[2*tile_index+1];

	SET_TILE_INFO(
			2,
			code,
			color / 4,
			0);
}

static TILE_GET_INFO( wbeachvl_get_fg_tile_info )
{
	UINT16 code = wbeachvl_videoram2[2*tile_index];
	UINT16 color = wbeachvl_videoram2[2*tile_index+1];

	SET_TILE_INFO(
			1,
			code & 0x7fff,
			color / 4 + 8,
			(code & 0x8000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( wbeachvl_get_bg_tile_info )
{
	UINT16 code = wbeachvl_videoram3[2*tile_index];
	UINT16 color = wbeachvl_videoram3[2*tile_index+1];

	SET_TILE_INFO(
			1,
			code & 0x7fff,
			color / 4,
			(code & 0x8000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( hrdtimes_get_tx_tile_info )
{
	int code = wbeachvl_videoram1[tile_index] & 0x03ff;
	int colr = wbeachvl_videoram1[tile_index] & 0xe000;

	SET_TILE_INFO(2,code + txt_tile_offset,colr >> 13,0);
}

static TILE_GET_INFO( hrdtimes_get_fg_tile_info )
{
	int code = wbeachvl_videoram2[tile_index] & 0x1fff;
	int colr = wbeachvl_videoram2[tile_index] & 0xe000;

	SET_TILE_INFO(1,code + 0x2000,(colr >> 13) + 8,0);
}

static TILE_GET_INFO( hrdtimes_get_bg_tile_info )
{
	int code = wbeachvl_videoram3[tile_index] & 0x1fff;
	int colr = wbeachvl_videoram3[tile_index] & 0xe000;

	SET_TILE_INFO(1,code,colr >> 13,0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bigtwin )
{
	tx_tilemap = tilemap_create(bigtwin_get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	fg_tilemap = tilemap_create(bigtwin_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,32,32);

	tilemap_set_transparent_pen(tx_tilemap,0);

	pri_masks[0] = 0;
	pri_masks[1] = 0;
	pri_masks[2] = 0;
}


VIDEO_START( wbeachvl )
{
	tx_tilemap = tilemap_create(wbeachvl_get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	fg_tilemap = tilemap_create(wbeachvl_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64,32);
	bg_tilemap = tilemap_create(wbeachvl_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,64,32);

	tilemap_set_transparent_pen(tx_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);

	pri_masks[0] = 0xfff0;
	pri_masks[1] = 0xfffc;
	pri_masks[2] = 0;
}

VIDEO_START( excelsr )
{
	tx_tilemap = tilemap_create(bigtwin_get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	fg_tilemap = tilemap_create(bigtwin_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,32,32);

	tilemap_set_transparent_pen(tx_tilemap,0);

	pri_masks[0] = 0;
	pri_masks[1] = 0xfffc;
	pri_masks[2] = 0xfff0;
}

VIDEO_START( hotmind )
{
	tx_tilemap = tilemap_create(hrdtimes_get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
	fg_tilemap = tilemap_create(hrdtimes_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	bg_tilemap = tilemap_create(hrdtimes_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,32,32);

	tilemap_set_transparent_pen(tx_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(tx_tilemap, -14, -14);
	tilemap_set_scrolldx(fg_tilemap, -14, -14);
	tilemap_set_scrolldx(bg_tilemap, -14, -14);

	xoffset = -9;
	yoffset = -8;
	txt_tile_offset = 0x9000;

	pri_masks[0] = 0xfff0;
	pri_masks[1] = 0xfffc;
	pri_masks[2] = 0;
}

VIDEO_START( hrdtimes )
{
	tx_tilemap = tilemap_create(hrdtimes_get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
	fg_tilemap = tilemap_create(hrdtimes_get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	bg_tilemap = tilemap_create(hrdtimes_get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,32,32);

	tilemap_set_transparent_pen(tx_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(tx_tilemap, -14, -14);
	tilemap_set_scrolldx(fg_tilemap, -10, -10);
	tilemap_set_scrolldx(bg_tilemap, -12, -12);

	xoffset = -8;
	yoffset = -8;
	txt_tile_offset = 0xfc00;

	pri_masks[0] = 0xfff0;
	pri_masks[1] = 0xfffc;
	pri_masks[2] = 0;
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( wbeachvl_txvideoram_w )
{
	COMBINE_DATA(&wbeachvl_videoram1[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset / 2);
}

WRITE16_HANDLER( wbeachvl_fgvideoram_w )
{
	COMBINE_DATA(&wbeachvl_videoram2[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset / 2);
}

WRITE16_HANDLER( wbeachvl_bgvideoram_w )
{
	COMBINE_DATA(&wbeachvl_videoram3[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset / 2);
}

WRITE16_HANDLER( hrdtimes_txvideoram_w )
{
	COMBINE_DATA(&wbeachvl_videoram1[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

WRITE16_HANDLER( hrdtimes_fgvideoram_w )
{
	COMBINE_DATA(&wbeachvl_videoram2[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE16_HANDLER( hrdtimes_bgvideoram_w )
{
	COMBINE_DATA(&wbeachvl_videoram3[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}


WRITE16_HANDLER( bigtwin_paletteram_w )
{
	int r,g,b,val;

	COMBINE_DATA(&paletteram16[offset]);

	val = paletteram16[offset];
	r = (val >> 11) & 0x1e;
	g = (val >>  7) & 0x1e;
	b = (val >>  3) & 0x1e;

	r |= ((val & 0x08) >> 3);
	g |= ((val & 0x04) >> 2);
	b |= ((val & 0x02) >> 1);

	palette_set_color_rgb(Machine,offset,pal5bit(r),pal5bit(g),pal5bit(b));
}

WRITE16_HANDLER( bigtwin_scroll_w )
{
	data = COMBINE_DATA(&playmark_scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(tx_tilemap,0,data+2); break;
		case 1: tilemap_set_scrolly(tx_tilemap,0,data);   break;
		case 2: bgscrollx = -(data+4);                    break;
		case 3: bgscrolly = (-data) & 0x1ff;
				bg_enable = data & 0x0200;
				bg_full_size = data & 0x0400;
				break;
		case 4: tilemap_set_scrollx(fg_tilemap,0,data+6); break;
		case 5: tilemap_set_scrolly(fg_tilemap,0,data);   break;
	}
}

WRITE16_HANDLER( wbeachvl_scroll_w )
{
	data = COMBINE_DATA(&playmark_scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(tx_tilemap,0,data+2); break;
		case 1: tilemap_set_scrolly(tx_tilemap,0,data);   break;
		case 2: fgscrollx = data+4;break;
		case 3: tilemap_set_scrolly(fg_tilemap,0,data & 0x3ff);
				fg_rowscroll_enable = data & 0x0800;
				break;
		case 4: tilemap_set_scrollx(bg_tilemap,0,data+6); break;
		case 5: tilemap_set_scrolly(bg_tilemap,0,data);   break;
	}
}

WRITE16_HANDLER( excelsr_scroll_w )
{
	data = COMBINE_DATA(&playmark_scroll[offset]);

	switch (offset)
	{
		case 0:	tilemap_set_scrollx(tx_tilemap,0,data+2); break;
		case 1: tilemap_set_scrolly(tx_tilemap,0,data);   break;
		case 2: bgscrollx = -data;                        break;
		case 3: bgscrolly = (-data+2)& 0x1ff;
				bg_enable = data & 0x0200;
				bg_full_size = data & 0x0400;
				break;
		case 4:	tilemap_set_scrollx(fg_tilemap,0,data+6); break;
		case 5:	tilemap_set_scrolly(fg_tilemap,0,data);   break;
	}
}

WRITE16_HANDLER( hrdtimes_scroll_w )
{
	data = COMBINE_DATA(&playmark_scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(tx_tilemap,0,data); break;
		case 1: tilemap_set_scrolly(tx_tilemap,0,data); break;
		case 2: tilemap_set_scrollx(fg_tilemap,0,data); break;
		case 3: tilemap_set_scrolly(fg_tilemap,0,data);	break;
		case 4: tilemap_set_scrollx(bg_tilemap,0,data); break;
		case 5: tilemap_set_scrolly(bg_tilemap,0,data); break;
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int codeshift)
{
	int offs, start_offset = spriteram_size/2 - 4;
	int height = machine->gfx[0]->height;
	int colordiv = machine->gfx[0]->color_granularity / 16;

	// find the "end of list" to draw the sprites in reverse order
	for (offs = 4;offs < spriteram_size/2;offs += 4)
	{
		if (spriteram16[offs+3-4] == 0x2000) /* end of list marker */
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (offs = start_offset;offs >= 4;offs -= 4)
	{
		int sx,sy,code,color,flipx,pri;

		sy = spriteram16[offs+3-4];	/* -4? what the... ??? */

		flipx = sy & 0x4000;
		sx = (spriteram16[offs+1] & 0x01ff) - 16-7;
		sy = (256-8-height - sy) & 0xff;
		code = spriteram16[offs+2] >> codeshift;
		color = ((spriteram16[offs+1] & 0x3e00) >> 9) / colordiv;
		pri = (spriteram16[offs+1] & 0x8000) >> 15;

		if(!pri && (color & 0x0c) == 0x0c)
			pri = 2;

		pdrawgfx(bitmap,machine->gfx[0],
		 		 code,
				 color,
				 flipx,0,
				 sx + xoffset,sy + yoffset,
				 cliprect,TRANSPARENCY_PEN,0,pri_masks[pri]);
	}
}

static void draw_bitmap(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int x,y,count;
	int color;
	UINT8 *pri;

	count = 0;
	for (y=0;y<512;y++)
	{
		for (x=0;x<512;x++)
		{
			color = bigtwin_bgvideoram[count] & 0xff;

			if(color)
			{
				if(bg_full_size)
				{
					*BITMAP_ADDR16(bitmap, (y + bgscrolly) & 0x1ff, (x + bgscrollx) & 0x1ff) = machine->pens[0x100 + color];

					pri = BITMAP_ADDR8(priority_bitmap, (y + bgscrolly) & 0x1ff, 0);
					pri[(x + bgscrollx) & 0x1ff] |= 2;
				}
				else
				{
					/* 50% size */
					if(!(x % 2) && !(y % 2))
					{
						*BITMAP_ADDR16(bitmap, (y / 2 + bgscrolly) & 0x1ff, (x / 2 + bgscrollx) & 0x1ff) = machine->pens[0x100 + color];

						pri = BITMAP_ADDR8(priority_bitmap, (y / 2 + bgscrolly) & 0x1ff, 0);
						pri[(x / 2 + bgscrollx) & 0x1ff] |= 2;
					}
				}
			}

			count++;
		}
	}
}

VIDEO_UPDATE( bigtwin )
{
	fillbitmap(priority_bitmap,0,cliprect);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	if (bg_enable)
		draw_bitmap(machine, bitmap, cliprect);
	draw_sprites(machine, bitmap,cliprect,4);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( excelsr )
{
	fillbitmap(priority_bitmap,0,cliprect);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,1);
	if (bg_enable)
		draw_bitmap(machine, bitmap, cliprect);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,4);
	draw_sprites(machine,bitmap,cliprect,2);
	return 0;
}

VIDEO_UPDATE( wbeachvl )
{
	if (fg_rowscroll_enable)
	{
		int i;

		tilemap_set_scroll_rows(fg_tilemap,512);
		for (i = 0;i < 256;i++)
			tilemap_set_scrollx(fg_tilemap,i+1,wbeachvl_rowscroll[8*i]);
	}
	else
	{
		tilemap_set_scroll_rows(fg_tilemap,1);
		tilemap_set_scrollx(fg_tilemap,0,fgscrollx);
	}

	fillbitmap(priority_bitmap,0,cliprect);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,1);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,2);
	draw_sprites(machine,bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( hrdtimes )
{
	fillbitmap(priority_bitmap,0,cliprect);

	// video enabled
	if(playmark_scroll[6] & 1)
	{
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,1);
		tilemap_draw(bitmap,cliprect,fg_tilemap,0,2);
		draw_sprites(machine,bitmap,cliprect,2);
		tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	}
	else
	{
		fillbitmap(bitmap,get_black_pen(machine),cliprect);
	}
	return 0;
}
