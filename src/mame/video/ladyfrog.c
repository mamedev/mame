/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "driver.h"
static int tilebank=0;

static tilemap *bg_tilemap;
static int palette_bank;
//static int gfxctrl;

UINT8 *ladyfrog_scrlram;

static UINT8 *ladyfrog_spriteram;

WRITE8_HANDLER(ladyfrog_spriteram_w)
{
	ladyfrog_spriteram[offset]=data;
}

READ8_HANDLER(ladyfrog_spriteram_r)
{
	return ladyfrog_spriteram[offset];
}

static TILE_GET_INFO( get_tile_info )
{
	int pal,tile;
	pal=videoram[tile_index*2+1]&0x0f;
	tile=videoram[tile_index*2] + ((videoram[tile_index*2+1] & 0xc0) << 2)+ ((videoram[tile_index*2+1] & 0x30) <<6 );
	SET_TILE_INFO(
			0,
			tile +0x1000 * tilebank,
			pal,TILE_FLIPY
			);
}

WRITE8_HANDLER( ladyfrog_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset>>1);
}

READ8_HANDLER( ladyfrog_videoram_r )
{
	return videoram[offset];
}

WRITE8_HANDLER( ladyfrog_palette_w )
{
	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (palette_bank << 8),data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (palette_bank << 8),data);
}

READ8_HANDLER( ladyfrog_palette_r )
{
	if (offset & 0x100)
		return paletteram_2[ (offset & 0xff) + (palette_bank << 8) ];
	else
		return paletteram  [ (offset & 0xff) + (palette_bank << 8) ];
}

WRITE8_HANDLER( ladyfrog_gfxctrl_w )
{
	palette_bank = (data & 0x20) >> 5;

}

WRITE8_HANDLER( ladyfrog_gfxctrl2_w )
{
	tilebank=((data & 0x18) >> 3)^3;
	tilemap_mark_all_tiles_dirty( bg_tilemap );
}


#ifdef UNUSED_FUNCTION
READ8_HANDLER( ladyfrog_gfxctrl_r )
{
		return 	gfxctrl;
}
#endif

READ8_HANDLER( ladyfrog_scrlram_r )
{
	return ladyfrog_scrlram[offset];
}

WRITE8_HANDLER( ladyfrog_scrlram_w )
{
	ladyfrog_scrlram[offset] = data;
	tilemap_set_scrolly(bg_tilemap, offset, data );
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int i;
	for (i=0;i<0x20;i++)
	{
		int pr = ladyfrog_spriteram[0x9f-i];
		int offs = (pr & 0x1f) * 4;
		{
			int code,sx,sy,flipx,flipy,pal;
			code = ladyfrog_spriteram[offs+2] + ((ladyfrog_spriteram[offs+1] & 0x10) << 4)+0x800;
			pal=ladyfrog_spriteram[offs+1] & 0x0f;
			sx = ladyfrog_spriteram[offs+3];
			sy = 240-ladyfrog_spriteram[offs+0];
			flipx = ((ladyfrog_spriteram[offs+1]&0x40)>>6);
			flipy = ((ladyfrog_spriteram[offs+1]&0x80)>>7);
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if(ladyfrog_spriteram[offs+3]>240)
			{
				sx = (ladyfrog_spriteram[offs+3]-256);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
        				code,
				        pal,
				        flipx,flipy,
					      sx,sy,15);
					}
				}
		}

}

VIDEO_START( ladyfrog )
{
  ladyfrog_spriteram = auto_alloc_array(machine, UINT8, 160);
  bg_tilemap = tilemap_create( machine, get_tile_info,tilemap_scan_rows,8,8,32,32 );

  paletteram = auto_alloc_array(machine, UINT8, 0x200);
  paletteram_2 = auto_alloc_array(machine, UINT8, 0x200);
  tilemap_set_scroll_cols(bg_tilemap,32);
  tilemap_set_scrolldy( bg_tilemap,   15, 15 );
}


VIDEO_UPDATE( ladyfrog )
{
    tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
    draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}

