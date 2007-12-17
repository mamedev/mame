#include "driver.h"

static tilemap *sderby_tilemap;
extern UINT16 *sderby_videoram;

static tilemap *sderby_md_tilemap;
extern UINT16 *sderby_md_videoram;

static tilemap *sderby_fg_tilemap;
extern UINT16 *sderby_fg_videoram;

/* BG Layer */

static TILE_GET_INFO( get_sderby_tile_info )
{
	int tileno,colour;

	tileno = sderby_videoram[tile_index*2];
	colour = sderby_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(1,tileno,colour,0);
}

WRITE16_HANDLER( sderby_videoram_w )
{
	COMBINE_DATA(&sderby_videoram[offset]);
	tilemap_mark_tile_dirty(sderby_tilemap,offset/2);
}

/* MD Layer */

static TILE_GET_INFO( get_sderby_md_tile_info )
{
	int tileno,colour;

	tileno = sderby_md_videoram[tile_index*2];
	colour = sderby_md_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(1,tileno,colour+16,0);
}

WRITE16_HANDLER( sderby_md_videoram_w )
{
	COMBINE_DATA(&sderby_md_videoram[offset]);
	tilemap_mark_tile_dirty(sderby_md_tilemap,offset/2);
}

/* FG Layer */

static TILE_GET_INFO( get_sderby_fg_tile_info )
{
	int tileno,colour;

	tileno = sderby_fg_videoram[tile_index*2];
	colour = sderby_fg_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO(0,tileno,colour+32,0);
}

WRITE16_HANDLER( sderby_fg_videoram_w )
{
	COMBINE_DATA(&sderby_fg_videoram[offset]);
	tilemap_mark_tile_dirty(sderby_fg_tilemap,offset/2);
}


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int codeshift)
{
	int offs;
	int height = machine->gfx[0]->height;
	int colordiv = machine->gfx[0]->color_granularity / 16;

	for (offs = 4;offs < spriteram_size/2;offs += 4)
	{
		int sx,sy,code,color,flipx;

		sy = spriteram16[offs+3-4];	/* -4? what the... ??? */
		if (sy == 0x2000) return;	/* end of list marker */

		flipx = sy & 0x4000;
		sx = (spriteram16[offs+1] & 0x01ff) - 16-7;
		sy = (256-8-height - sy) & 0xff;
		code = spriteram16[offs+2] >> codeshift;
		color = (spriteram16[offs+1] & 0x3e00) >> 9;

		drawgfx(bitmap,machine->gfx[1],
				code,
				color/colordiv+48,
				flipx,0,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
}


VIDEO_START( sderby )
{
	sderby_tilemap = tilemap_create(get_sderby_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,32,32);
	sderby_md_tilemap = tilemap_create(get_sderby_md_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,32,32);

	tilemap_set_transparent_pen(sderby_md_tilemap,0);

	sderby_fg_tilemap = tilemap_create(get_sderby_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	tilemap_set_transparent_pen(sderby_fg_tilemap,0);
}

VIDEO_UPDATE( sderby )
{
	tilemap_draw(bitmap,cliprect,sderby_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,sderby_md_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sderby_fg_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( pmroulet )
{
	tilemap_draw(bitmap,cliprect,sderby_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sderby_md_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,sderby_fg_tilemap,0,0);
	return 0;
}


WRITE16_HANDLER( sderby_scroll_w )
{
	static UINT16 scroll[6];


	data = COMBINE_DATA(&scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(sderby_fg_tilemap,0,data+2);break;
		case 1: tilemap_set_scrolly(sderby_fg_tilemap,0,data-8);break;
		case 2: tilemap_set_scrollx(sderby_md_tilemap,0,data+4);break;
		case 3: tilemap_set_scrolly(sderby_md_tilemap,0,data-8);break;
		case 4: tilemap_set_scrollx(sderby_tilemap,0,data+6);   break;
		case 5: tilemap_set_scrolly(sderby_tilemap,0,data-8);   break;
	}
}
