/* video/tbowl.c */

/* see drivers/tbowl.c for more info */

#include "driver.h"

static tilemap *tx_tilemap, *bg_tilemap, *bg2_tilemap;
UINT8 *tbowl_txvideoram, *tbowl_bgvideoram, *tbowl_bg2videoram;
UINT8 *tbowl_spriteram;

/*** Tilemap Stuff

***/

static UINT16 tbowl_xscroll;
static UINT16 tbowl_yscroll;

static UINT16 tbowl_bg2xscroll;
static UINT16 tbowl_bg2yscroll;

/* Foreground Layer (tx) Tilemap */

static TILE_GET_INFO( get_tx_tile_info )
{
	int tileno;
	int col;

	tileno = tbowl_txvideoram[tile_index] | ((tbowl_txvideoram[tile_index+0x800] & 0x07) << 8);
	col = (tbowl_txvideoram[tile_index+0x800] & 0xf0) >> 4;

	SET_TILE_INFO(0,tileno,col,0);
}

WRITE8_HANDLER( tbowl_txvideoram_w )
{
	tbowl_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset & 0x7ff);
}

/* Bottom BG Layer (bg) Tilemap */

static TILE_GET_INFO( get_bg_tile_info )
{
	int tileno;
	int col;

	tileno = tbowl_bgvideoram[tile_index] | ((tbowl_bgvideoram[tile_index+0x1000] & 0x0f) << 8);
	col = (tbowl_bgvideoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO(1,tileno,col,0);
}

WRITE8_HANDLER( tbowl_bg2videoram_w )
{
	tbowl_bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(bg2_tilemap,offset & 0xfff);
}

WRITE8_HANDLER (tbowl_bgxscroll_lo)
{
	tbowl_xscroll = (tbowl_xscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bgxscroll_hi)
{
	tbowl_xscroll = (tbowl_xscroll & 0x00ff) | (data << 8);
}

WRITE8_HANDLER (tbowl_bgyscroll_lo)
{
	tbowl_yscroll = (tbowl_yscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bgyscroll_hi)
{
	tbowl_yscroll = (tbowl_yscroll & 0x00ff) | (data << 8);
}

/* Middle BG Layer (bg2) Tilemaps */

static TILE_GET_INFO( get_bg2_tile_info )
{
	int tileno;
	int col;

	tileno = tbowl_bg2videoram[tile_index] | ((tbowl_bg2videoram[tile_index+0x1000] & 0x0f) << 8);
	tileno ^= 0x400;
	col = (tbowl_bg2videoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO(2,tileno,col,0);
}

WRITE8_HANDLER( tbowl_bgvideoram_w )
{
	tbowl_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0xfff);
}

WRITE8_HANDLER (tbowl_bg2xscroll_lo)
{
	tbowl_bg2xscroll = (tbowl_bg2xscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bg2xscroll_hi)
{
	tbowl_bg2xscroll = (tbowl_bg2xscroll & 0x00ff) | (data << 8);
}

WRITE8_HANDLER (tbowl_bg2yscroll_lo)
{
	tbowl_bg2yscroll = (tbowl_bg2yscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bg2yscroll_hi)
{
	tbowl_bg2yscroll = (tbowl_bg2yscroll & 0x00ff) | (data << 8);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect, int xscroll)
{
	int offs;
	static const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = 0;offs < 0x800;offs += 8)
	{
		if (tbowl_spriteram[offs+0] & 0x80)	/* enable */
		{
			int code,color,sizex,sizey,flipx,flipy,xpos,ypos;
			int x,y;//,priority,priority_mask;

			code = (tbowl_spriteram[offs+2])+(tbowl_spriteram[offs+1]<<8);
			color = (tbowl_spriteram[offs+3])&0x1f;
			sizex = 1 << ((tbowl_spriteram[offs+0] & 0x03) >> 0);
			sizey = 1 << ((tbowl_spriteram[offs+0] & 0x0c) >> 2);

			flipx = (tbowl_spriteram[offs+0])&0x20;
			flipy = 0;
			xpos = (tbowl_spriteram[offs+6])+((tbowl_spriteram[offs+4]&0x03)<<8);
			ypos = (tbowl_spriteram[offs+5])+((tbowl_spriteram[offs+4]&0x10)<<4);

			/* bg: 1; fg:2; text: 4 */

			for (y = 0;y < sizey;y++)
			{
				for (x = 0;x < sizex;x++)
				{
					int sx = xpos + 8*(flipx?(sizex-1-x):x);
					int sy = ypos + 8*(flipy?(sizey-1-y):y);

					sx -= xscroll;

					drawgfx(bitmap,machine->gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy,
							cliprect,TRANSPARENCY_PEN,0 );

					/* wraparound */
					drawgfx(bitmap,machine->gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy-0x200,
							cliprect,TRANSPARENCY_PEN,0 );

					/* wraparound */
					drawgfx(bitmap,machine->gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy,
							cliprect,TRANSPARENCY_PEN,0 );

					/* wraparound */
					drawgfx(bitmap,machine->gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy-0x200,
							cliprect,TRANSPARENCY_PEN,0 );



				}
			}
		}
	}

}


/*** Video Start / Update ***/

VIDEO_START( tbowl )
{
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,128,32);
	bg2_tilemap = tilemap_create(get_bg2_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,128,32);

	tilemap_set_transparent_pen(tx_tilemap,0);
	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_transparent_pen(bg2_tilemap,0);
}


VIDEO_UPDATE( tbowl )
{
	if (screen == 0)
	{
		tilemap_set_scrollx(bg_tilemap,  0, tbowl_xscroll );
		tilemap_set_scrolly(bg_tilemap,  0, tbowl_yscroll );
		tilemap_set_scrollx(bg2_tilemap, 0, tbowl_bg2xscroll );
		tilemap_set_scrolly(bg2_tilemap, 0, tbowl_bg2yscroll );
		tilemap_set_scrollx(tx_tilemap,  0, 0 );
		tilemap_set_scrolly(tx_tilemap,  0, 0 );

		fillbitmap(bitmap,0x100,cliprect); /* is there a register controling the colour? looks odd when screen is blank */
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
		draw_sprites(machine, bitmap,cliprect, 0);
		tilemap_draw(bitmap,cliprect,bg2_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	}
	else if (screen ==1)
	{
		tilemap_set_scrollx(bg_tilemap,  0, tbowl_xscroll+32*8 );
		tilemap_set_scrolly(bg_tilemap,  0, tbowl_yscroll );
		tilemap_set_scrollx(bg2_tilemap, 0, tbowl_bg2xscroll+32*8 );
		tilemap_set_scrolly(bg2_tilemap, 0, tbowl_bg2yscroll );
		tilemap_set_scrollx(tx_tilemap,  0, 32*8 );
		tilemap_set_scrolly(tx_tilemap,  0, 0 );

		fillbitmap(bitmap,0x100,cliprect); /* is there a register controling the colour? looks odd when screen is blank */
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
		draw_sprites(machine, bitmap,cliprect, 32*8);
		tilemap_draw(bitmap,cliprect,bg2_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	}
	return 0;
}
