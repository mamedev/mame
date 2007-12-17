/* Aquarium */

#include "driver.h"

extern UINT16 *aquarium_scroll;

static tilemap *aquarium_txt_tilemap;
extern UINT16 *aquarium_txt_videoram;

static tilemap *aquarium_mid_tilemap;
extern UINT16 *aquarium_mid_videoram;

static tilemap *aquarium_bak_tilemap;
extern UINT16 *aquarium_bak_videoram;

/* gcpinbal.c modified */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int y_offs)
{
	int offs,chain_pos;
	int x,y,curx,cury;
	UINT8 col,flipx,flipy,chain;
	UINT16 code;

	for (offs = 0;offs < spriteram_size/2;offs += 8)
	{
		code = ((spriteram16[offs+5])&0xff) + (((spriteram16[offs+6]) &0xff) << 8);
		code &= 0x3fff;

		if (!(spriteram16[offs+4] &0x80))	/* active sprite ? */
		{
			x = ((spriteram16[offs+0]) &0xff) + (((spriteram16[offs+1]) &0xff) << 8);
			y = ((spriteram16[offs+2]) &0xff) + (((spriteram16[offs+3]) &0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col  =   ((spriteram16[offs+7]) &0x0f);
			chain =   (spriteram16[offs+4]) &0x07;
			flipy =   (spriteram16[offs+4]) &0x10;
			flipx =   (spriteram16[offs+4]) &0x20;

			curx = x;
			cury = y;

			if (((spriteram16[offs+4]) &0x08) && flipy)
				cury += (chain * 16);

			if (!(((spriteram16[offs+4]) &0x08)) && flipx)
				curx += (chain * 16);


			for (chain_pos = chain;chain_pos >= 0;chain_pos--)
			{
				drawgfx(bitmap, machine->gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury,
						cliprect,TRANSPARENCY_PEN,0);

				/* wrap around y */
				drawgfx(bitmap, machine->gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury+256,
						cliprect,TRANSPARENCY_PEN,0);

				code++;

				if ((spriteram16[offs+4]) &0x08)	/* Y chain */
				{
					if (flipy)	cury -= 16;
					else cury += 16;
				}
				else	/* X chain */
				{
					if (flipx)	curx -= 16;
					else curx += 16;
				}
			}
		}
	}
#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf,"sprite rotate offs %04x ?",rotate);
		popmessage(buf);
	}
#endif
}

/* TXT Layer */

static TILE_GET_INFO( get_aquarium_txt_tile_info )
{
	int tileno,colour;

	tileno = (aquarium_txt_videoram[tile_index] & 0x0fff);
	colour = (aquarium_txt_videoram[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO(2,tileno,colour,0);
}

WRITE16_HANDLER( aquarium_txt_videoram_w )
{
	aquarium_txt_videoram[offset] = data;
	tilemap_mark_tile_dirty(aquarium_txt_tilemap,offset);
}

/* MID Layer */

static TILE_GET_INFO( get_aquarium_mid_tile_info )
{
	int tileno,colour,flag;

	tileno = (aquarium_mid_videoram[tile_index*2] & 0x0fff);
	colour = (aquarium_mid_videoram[tile_index*2+1] & 0x001f);
	flag   = TILE_FLIPYX((aquarium_mid_videoram[tile_index*2+1] & 0x300) >> 8);

	SET_TILE_INFO(1,tileno,colour,flag);

	tileinfo->category = (aquarium_mid_videoram[tile_index*2+1] & 0x20) >> 5;
}

WRITE16_HANDLER( aquarium_mid_videoram_w )
{
	aquarium_mid_videoram[offset] = data;
	tilemap_mark_tile_dirty(aquarium_mid_tilemap,offset/2);
}

/* BAK Layer */
static TILE_GET_INFO( get_aquarium_bak_tile_info )

{
	int tileno,colour,flag;

	tileno = (aquarium_bak_videoram[tile_index*2] & 0x0fff);
	colour = (aquarium_bak_videoram[tile_index*2+1] & 0x001f);
	flag   = TILE_FLIPYX((aquarium_bak_videoram[tile_index*2+1] & 0x300) >> 8);

	SET_TILE_INFO(3,tileno,colour,flag);

	tileinfo->category = (aquarium_bak_videoram[tile_index*2+1] & 0x20) >> 5;
}

WRITE16_HANDLER( aquarium_bak_videoram_w )
{
	aquarium_bak_videoram[offset] = data;
	tilemap_mark_tile_dirty(aquarium_bak_tilemap,offset/2);
}

VIDEO_START(aquarium)
{
	aquarium_txt_tilemap = tilemap_create(get_aquarium_txt_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
	tilemap_set_transparent_pen(aquarium_txt_tilemap,0);

	aquarium_bak_tilemap = tilemap_create(get_aquarium_bak_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,32,32);
	aquarium_mid_tilemap = tilemap_create(get_aquarium_mid_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,32,32);
	tilemap_set_transparent_pen(aquarium_mid_tilemap,0);
}

VIDEO_UPDATE(aquarium)
{
	tilemap_set_scrollx(aquarium_mid_tilemap, 0, aquarium_scroll[0]);
	tilemap_set_scrolly(aquarium_mid_tilemap, 0, aquarium_scroll[1]);
	tilemap_set_scrollx(aquarium_bak_tilemap, 0, aquarium_scroll[2]);
	tilemap_set_scrolly(aquarium_bak_tilemap, 0, aquarium_scroll[3]);
	tilemap_set_scrollx(aquarium_txt_tilemap, 0, aquarium_scroll[4]);
	tilemap_set_scrolly(aquarium_txt_tilemap, 0, aquarium_scroll[5]);

	tilemap_draw(bitmap,cliprect,aquarium_bak_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,aquarium_mid_tilemap,0,0);

	draw_sprites(machine, bitmap,cliprect,16);

	tilemap_draw(bitmap,cliprect,aquarium_bak_tilemap,1,0);
	tilemap_draw(bitmap,cliprect,aquarium_mid_tilemap,1,0);

	tilemap_draw(bitmap,cliprect,aquarium_txt_tilemap,0,0);
	return 0;
}
