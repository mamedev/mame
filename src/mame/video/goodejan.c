#include "driver.h"

static tilemap *bg_tilemap, *tx_tilemap;
UINT16 *goodejan_bgvram,*goodejan_txvram;

WRITE16_HANDLER( goodejan_bgvram_w )
{
	COMBINE_DATA(&goodejan_bgvram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE16_HANDLER( goodejan_txvram_w )
{
	COMBINE_DATA(&goodejan_txvram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

static TILE_GET_INFO( goodejan_bg_tile_info )
{
	int tile = goodejan_bgvram[tile_index]&0x0fff;
	int color = (goodejan_bgvram[tile_index]&0xf000)>>12;

	// WRONG!
	if ((goodejan_bgvram[tile_index]&0x8000)==0x0000) tile+=0x1000;

	SET_TILE_INFO(1, tile, color, 0);
}

static TILE_GET_INFO( goodejan_tx_tile_info )
{
	int tile = goodejan_txvram[tile_index];
	int color = (tile >> 12) & 0x0f;
	SET_TILE_INFO(2, (tile & 0xfff) + 0x3000, color, 0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int pri)
{
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay;

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		if ((spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = spriteram16[offs+1];
		if ((sprite>>14)!=pri) continue;
		sprite &= 0x1fff;

		y = spriteram16[offs+3];//&0x1ff;
		x = spriteram16[offs+2];//&0x1ff;

		if (spriteram16[offs+2]&0x8000) x=0x10000-x;
		if (spriteram16[offs+3]&0x8000) y=0x10000-y;

		color = spriteram16[offs+0]&0x3f;
		fx = spriteram16[offs+0]&0x40;
		fy = 0; /* To do - probably 0x2000 */
		dy = ((spriteram16[offs+0]&0x0380)>>7) + 1;
		dx = ((spriteram16[offs+0]&0x1c00)>>10) + 1;

		for (ax=0; ax<dx; ax++)
			for (ay=0; ay<dy; ay++) {
				if (!fx)
					drawgfx(bitmap,machine->gfx[0],
						sprite++,
						color,fx,fy,x+ax*16,y+ay*16,
						cliprect,TRANSPARENCY_PEN,15);
				else
					drawgfx(bitmap,machine->gfx[0],
						sprite++,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,
						cliprect,TRANSPARENCY_PEN,15);
			}
	}
}


WRITE16_HANDLER( goodejan_bg_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap,0, data);
}

WRITE16_HANDLER( goodejan_bg_scrolly_w )
{
	tilemap_set_scrolly(bg_tilemap,0, data);
}

VIDEO_START( goodejan )
{
	bg_tilemap = tilemap_create(goodejan_bg_tile_info,tilemap_scan_rows,     16,16,32,32);
	tx_tilemap = tilemap_create(goodejan_tx_tile_info,tilemap_scan_rows, 8, 8,32,32);
	tilemap_set_transparent_pen(tx_tilemap,15);
}

VIDEO_UPDATE( goodejan )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect, 2);
	draw_sprites(machine, bitmap,cliprect, 1);
	draw_sprites(machine, bitmap,cliprect, 0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect, 3);

	return 0;
}
