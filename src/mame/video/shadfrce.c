#include "driver.h"

static tilemap_t *shadfrce_fgtilemap, *shadfrce_bg0tilemap,  *shadfrce_bg1tilemap;
extern UINT16 *shadfrce_fgvideoram, *shadfrce_bg0videoram,  *shadfrce_bg1videoram,   *shadfrce_spvideoram;

extern int shadfrce_video_enable;

static UINT16 *shadfrce_spvideoram_old; /* I *think* the sprites need to be delayed anyway */


static TILE_GET_INFO( get_shadfrce_fgtile_info )
{

	/* ---- ----  tttt tttt  ---- ----  pppp TTTT */
	int tileno, colour;

	tileno = (shadfrce_fgvideoram[tile_index *2] & 0x00ff) | ((shadfrce_fgvideoram[tile_index *2+1] & 0x000f) << 8);
	colour = (shadfrce_fgvideoram[tile_index *2+1] & 0x00f0) >>4;

	SET_TILE_INFO(0,tileno,colour*4,0);
}

WRITE16_HANDLER( shadfrce_fgvideoram_w )
{
	shadfrce_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(shadfrce_fgtilemap,offset/2);
}

static TILE_GET_INFO( get_shadfrce_bg0tile_info )
{

	/* ---- ----  ---- cccc  --TT TTTT TTTT TTTT */
	int tileno, colour,fyx;

	tileno = (shadfrce_bg0videoram[tile_index *2+1] & 0x3fff);
	colour = shadfrce_bg0videoram[tile_index *2] & 0x001f;
	if (colour & 0x10) colour ^= 0x30;	/* skip hole */
	fyx = (shadfrce_bg0videoram[tile_index *2] & 0x00c0) >>6;

	SET_TILE_INFO(2,tileno,colour,TILE_FLIPYX(fyx));
}

WRITE16_HANDLER( shadfrce_bg0videoram_w )
{
	shadfrce_bg0videoram[offset] = data;
	tilemap_mark_tile_dirty(shadfrce_bg0tilemap,offset/2);
}

static TILE_GET_INFO( get_shadfrce_bg1tile_info )
{
	int tileno, colour;

	tileno = (shadfrce_bg1videoram[tile_index] & 0x0fff);
	colour = (shadfrce_bg1videoram[tile_index] & 0xf000) >> 12;

	SET_TILE_INFO(2,tileno,colour+64,0);
}

WRITE16_HANDLER( shadfrce_bg1videoram_w )
{
	shadfrce_bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(shadfrce_bg1tilemap,offset);
}




VIDEO_START( shadfrce )
{
	shadfrce_fgtilemap = tilemap_create(machine, get_shadfrce_fgtile_info,tilemap_scan_rows,    8,  8,64,32);
	tilemap_set_transparent_pen(shadfrce_fgtilemap,0);

	shadfrce_bg0tilemap = tilemap_create(machine, get_shadfrce_bg0tile_info,tilemap_scan_rows, 16, 16,32,32);
	tilemap_set_transparent_pen(shadfrce_bg0tilemap,0);

	shadfrce_bg1tilemap = tilemap_create(machine, get_shadfrce_bg1tile_info,tilemap_scan_rows, 16, 16,32,32);

	shadfrce_spvideoram_old = auto_alloc_array(machine, UINT16, machine->generic.spriteram_size/2);
}

WRITE16_HANDLER ( shadfrce_bg0scrollx_w )
{
	tilemap_set_scrollx( shadfrce_bg0tilemap, 0, data & 0x1ff );
}

WRITE16_HANDLER ( shadfrce_bg0scrolly_w )
{
	tilemap_set_scrolly( shadfrce_bg0tilemap, 0, data  & 0x1ff );
}

WRITE16_HANDLER ( shadfrce_bg1scrollx_w )
{
	tilemap_set_scrollx( shadfrce_bg1tilemap, 0, data  & 0x1ff );
}

WRITE16_HANDLER ( shadfrce_bg1scrolly_w )
{
	tilemap_set_scrolly( shadfrce_bg1tilemap, 0, data & 0x1ff );
}




static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{

	/* | ---- ---- hhhf Fe-Y | ---- ---- yyyy yyyy | ---- ---- TTTT TTTT | ---- ---- tttt tttt |
       | ---- ---- -pCc cccX | ---- ---- xxxx xxxx | ---- ---- ---- ---- | ---- ---- ---- ---- | */

	/* h  = height
       f  = flipx
       F  = flipy
       e  = enable
       Yy = Y Position
       Tt = Tile No.
       Xx = X Position
       Cc = color
       P = priority
    */

	const gfx_element *gfx = machine->gfx[1];
	UINT16 *finish = shadfrce_spvideoram_old;
	UINT16 *source = finish + 0x2000/2 - 8;
	int hcount;
	while( source>=finish )
	{
		int ypos = 0x100 - (((source[0] & 0x0003) << 8) | (source[1] & 0x00ff));
		int xpos = (((source[4] & 0x0001) << 8) | (source[5] & 0x00ff)) + 1;
		int tile = ((source[2] & 0x00ff) << 8) | (source[3] & 0x00ff);
		int height = (source[0] & 0x00e0) >> 5;
		int enable = ((source[0] & 0x0004));
		int flipx = ((source[0] & 0x0010) >> 4);
		int flipy = ((source[0] & 0x0008) >> 3);
		int pal = ((source[4] & 0x003e));
		int pri_mask = (source[4] & 0x0040) ? 0x02 : 0x00;

		if (pal & 0x20) pal ^= 0x60;	/* skip hole */

		height++;
		if (enable)	{
			for (hcount=0;hcount<height;hcount++) {
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16,machine->priority_bitmap,pri_mask,0);
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16,machine->priority_bitmap,pri_mask,0);
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16+0x200,machine->priority_bitmap,pri_mask,0);
				pdrawgfx_transpen(bitmap,cliprect,gfx,tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16+0x200,machine->priority_bitmap,pri_mask,0);
			}
		}
		source-=8;
	}
}

VIDEO_UPDATE( shadfrce )
{
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	if(shadfrce_video_enable)
	{
		tilemap_draw(bitmap,cliprect,shadfrce_bg1tilemap,0,0);
		tilemap_draw(bitmap,cliprect,shadfrce_bg0tilemap,0,1);
		draw_sprites(screen->machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,shadfrce_fgtilemap, 0,0);
	}
	else
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	}

	return 0;
}

VIDEO_EOF( shadfrce )
{
	/* looks like sprites are *two* frames ahead */
	memcpy(shadfrce_spvideoram_old,shadfrce_spvideoram,machine->generic.spriteram_size);
}
