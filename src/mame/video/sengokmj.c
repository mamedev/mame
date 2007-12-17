/******************************************************************************************

Sengoku Mahjong Video Hardware section

*******************************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap,*md_tilemap,*fg_tilemap,*tx_tilemap;
UINT16 *sengokmj_bgvram,*sengokmj_mdvram,*sengokmj_fgvram,*sengokmj_txvram;

WRITE16_HANDLER( sengokmj_bgvram_w )
{
	COMBINE_DATA(&sengokmj_bgvram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE16_HANDLER( sengokmj_mdvram_w )
{
	COMBINE_DATA(&sengokmj_mdvram[offset]);
	tilemap_mark_tile_dirty(md_tilemap,offset);
}

WRITE16_HANDLER( sengokmj_fgvram_w )
{
	COMBINE_DATA(&sengokmj_fgvram[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE16_HANDLER( sengokmj_txvram_w )
{
	COMBINE_DATA(&sengokmj_txvram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

static TILE_GET_INFO( sengoku_bg_tile_info )
{
	int tile = sengokmj_bgvram[tile_index];
	int color = (tile >> 12) & 0x0f;
	SET_TILE_INFO(1, tile & 0xfff, color, 0);
}

static TILE_GET_INFO( sengoku_md_tile_info )
{
	int tile = sengokmj_mdvram[tile_index];
	int color = (tile >> 12) & 0x0f;
	SET_TILE_INFO(1, (tile & 0xfff) + 0x1000, color + 0x10, 0);
}

static TILE_GET_INFO( sengoku_fg_tile_info )
{
	int tile = sengokmj_fgvram[tile_index];
	int color = (tile >> 12) & 0x0f;
	SET_TILE_INFO(1, (tile & 0xfff) + 0x2000, color + 0x20, 0);
}

static TILE_GET_INFO( sengoku_tx_tile_info )
{
	int tile = sengokmj_txvram[tile_index];
	int color = (tile >> 12) & 0x0f;
	SET_TILE_INFO(2, (tile & 0xfff) + 0x3000, color, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int pri)
{
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay;

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		if ((spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = spriteram16[offs+1];
		if ((sprite>>14)!=pri) continue;
		sprite &= 0x1fff;

		y = spriteram16[offs+3];
		x = 128 + spriteram16[offs+2];

		if (x&0x8000) x=0-(0x200-(x&0x1ff));
		//else x&=0x1ff;
		if (y&0x8000) y=0-(0x200-(y&0x1ff));
		//else y&=0x1ff;

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

VIDEO_START( sengokmj )
{
	bg_tilemap = tilemap_create(sengoku_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,32,16);
	md_tilemap = tilemap_create(sengoku_md_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,16);
	fg_tilemap = tilemap_create(sengoku_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,16);
	tx_tilemap = tilemap_create(sengoku_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);

	tilemap_set_transparent_pen(md_tilemap,15);
	tilemap_set_transparent_pen(fg_tilemap,15);
	tilemap_set_transparent_pen(tx_tilemap,15);
}

VIDEO_UPDATE( sengokmj )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect, 2);
	draw_sprites(machine, bitmap,cliprect, 1);
	tilemap_draw(bitmap,cliprect,md_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect, 0);
	draw_sprites(machine, bitmap,cliprect, 3);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}
