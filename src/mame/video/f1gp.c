#include "driver.h"
#include "video/konamiic.h"
#include "f1gp.h"


UINT16 *f1gp_spr1vram,*f1gp_spr2vram,*f1gp_spr1cgram,*f1gp_spr2cgram;
UINT16 *f1gp_fgvideoram,*f1gp_rozvideoram;
UINT16 *f1gp2_sprcgram,*f1gp2_spritelist;
UINT16 *f1gpb_rozregs, *f1gpb_fgregs;
size_t f1gp_spr1cgram_size,f1gp_spr2cgram_size;

static UINT16 *zoomdata;
static int dirtygfx,flipscreen,gfxctrl;
static UINT8 *dirtychar;

#define TOTAL_CHARS 0x800

static tilemap *fg_tilemap,*roz_tilemap;
static int f1gp2_roz_bank;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( f1gp_get_roz_tile_info )
{
	int code = f1gp_rozvideoram[tile_index];

	SET_TILE_INFO(3,code & 0x7ff,code >> 12,0);
}

static TILE_GET_INFO( f1gp2_get_roz_tile_info )
{
	int code = f1gp_rozvideoram[tile_index];

	SET_TILE_INFO(2,(code & 0x7ff) + (f1gp2_roz_bank << 11),code >> 12,0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = f1gp_fgvideoram[tile_index];

	SET_TILE_INFO(0,code & 0x7fff,0,(code & 0x8000)?TILE_FLIPY:0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( f1gp )
{
	roz_tilemap = tilemap_create(f1gp_get_roz_tile_info,tilemap_scan_rows,16,16,64,64);
	fg_tilemap =  tilemap_create(get_fg_tile_info,      tilemap_scan_rows, 8, 8,64,32);

	K053936_wraparound_enable(0, 1);
	K053936_set_offset(0, -58, -2);

	tilemap_set_transparent_pen(fg_tilemap,0xff);

	dirtychar = auto_malloc(TOTAL_CHARS);
	memset(dirtychar,1,TOTAL_CHARS);

	zoomdata = (UINT16 *)memory_region(REGION_GFX4);
}

VIDEO_START( f1gpb )
{
	roz_tilemap = tilemap_create(f1gp_get_roz_tile_info,tilemap_scan_rows,16,16,64,64);
	fg_tilemap =  tilemap_create(get_fg_tile_info,      tilemap_scan_rows, 8, 8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0xff);

	dirtychar = auto_malloc(TOTAL_CHARS);
	memset(dirtychar,1,TOTAL_CHARS);

	zoomdata = (UINT16 *)memory_region(REGION_GFX4);
}

VIDEO_START( f1gp2 )
{
	roz_tilemap = tilemap_create(f1gp2_get_roz_tile_info,tilemap_scan_rows,16,16,64,64);
	fg_tilemap =  tilemap_create(get_fg_tile_info,       tilemap_scan_rows, 8, 8,64,32);

	K053936_wraparound_enable(0, 1);
	K053936_set_offset(0, -48, -21);

	tilemap_set_transparent_pen(fg_tilemap,0xff);
	tilemap_set_transparent_pen(roz_tilemap,0x0f);

	tilemap_set_scrolldx(fg_tilemap,-80,0);
	tilemap_set_scrolldy(fg_tilemap,-26,0);

	dirtychar = auto_malloc(TOTAL_CHARS);
	memset(dirtychar,1,TOTAL_CHARS);

	zoomdata = (UINT16 *)memory_region(REGION_GFX4);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_HANDLER( f1gp_zoomdata_r )
{
	return zoomdata[offset];
}

WRITE16_HANDLER( f1gp_zoomdata_w )
{
	int oldword = zoomdata[offset];
	COMBINE_DATA(&zoomdata[offset]);
	if (oldword != zoomdata[offset])
	{
		dirtygfx = 1;
		dirtychar[offset / 64] = 1;
	}
}

READ16_HANDLER( f1gp_rozvideoram_r )
{
	return f1gp_rozvideoram[offset];
}

WRITE16_HANDLER( f1gp_rozvideoram_w )
{
	COMBINE_DATA(&f1gp_rozvideoram[offset]);
	tilemap_mark_tile_dirty(roz_tilemap,offset);
}

WRITE16_HANDLER( f1gp_fgvideoram_w )
{
	COMBINE_DATA(&f1gp_fgvideoram[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE16_HANDLER( f1gp_fgscroll_w )
{
	static int scroll[2];

	COMBINE_DATA(&scroll[offset]);

	tilemap_set_scrollx(fg_tilemap,0,scroll[0]);
	tilemap_set_scrolly(fg_tilemap,0,scroll[1]);
}

WRITE16_HANDLER( f1gp_gfxctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		flipscreen = data & 0x20;
		gfxctrl = data & 0xdf;
	}
}

WRITE16_HANDLER( f1gp2_gfxctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		flipscreen = data & 0x20;

		/* bit 0/1 = fg/sprite/roz priority */
		/* bit 2 = blank screen */

		gfxctrl = data & 0xdf;
	}

	if (ACCESSING_BITS_8_15)
	{
		if (f1gp2_roz_bank != (data >> 8))
		{
			f1gp2_roz_bank = (data >> 8);
			tilemap_mark_all_tiles_dirty(roz_tilemap);
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void f1gp_draw_sprites(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int chip,int primask)
{
	int attr_start,first;
	UINT16 *spram = chip ? f1gp_spr2vram : f1gp_spr1vram;

	first = 4 * spram[0x1fe];

	for (attr_start = 0x0200-8;attr_start >= first;attr_start -= 4)
	{
		int map_start;
		int ox,oy,x,y,xsize,ysize,zoomx,zoomy,flipx,flipy,color,pri;
		/* table hand made by looking at the ship explosion in attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(spram[attr_start + 2] & 0x0080)) continue;

		ox = spram[attr_start + 1] & 0x01ff;
		xsize = (spram[attr_start + 2] & 0x0700) >> 8;
		zoomx = (spram[attr_start + 1] & 0xf000) >> 12;
		oy = spram[attr_start + 0] & 0x01ff;
		ysize = (spram[attr_start + 2] & 0x7000) >> 12;
		zoomy = (spram[attr_start + 0] & 0xf000) >> 12;
		flipx = spram[attr_start + 2] & 0x0800;
		flipy = spram[attr_start + 2] & 0x8000;
		color = (spram[attr_start + 2] & 0x000f);// + 16 * spritepalettebank;
		pri = 0;//spram[attr_start + 2] & 0x0010;
		map_start = spram[attr_start + 3];

		zoomx = 16 - zoomtable[zoomx]/8;
		zoomy = 16 - zoomtable[zoomy]/8;

		for (y = 0;y <= ysize;y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0;x <= xsize;x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = f1gp_spr1cgram[map_start % (f1gp_spr1cgram_size/2)];
				else
					code = f1gp_spr2cgram[map_start % (f1gp_spr2cgram_size/2)];

				pdrawgfxzoom(bitmap,machine->gfx[1 + chip],
						code,
						color,
						flipx,flipy,
						sx,sy,
						cliprect,TRANSPARENCY_PEN,15,
						0x1000 * zoomx,0x1000 * zoomy,
//                      pri ? 0 : 0x2);
						primask);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void f1gpb_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int attr_start, start_offset = spriteram_size/2 - 4;

	// find the "end of list" to draw the sprites in reverse order
	for (attr_start = 4;attr_start < spriteram_size/2;attr_start += 4)
	{
		if (spriteram16[attr_start+3-4] == 0xffff) /* end of list marker */
		{
			start_offset = attr_start - 4;
			break;
		}
	}

	for (attr_start = start_offset;attr_start >= 4;attr_start -= 4)
	{
		int code,gfx;
		int x,y,flipx,flipy,color,pri;

		x = (spriteram16[attr_start + 2] & 0x03ff) - 48;
		y = (256 - (spriteram16[attr_start + 3 - 4] & 0x03ff)) - 15;
		flipx = spriteram16[attr_start + 1] & 0x0800;
		flipy = spriteram16[attr_start + 1] & 0x8000;
		color = spriteram16[attr_start + 1] & 0x000f;
		code = spriteram16[attr_start + 0] & 0x3fff;
		pri = 0; //?

		if((spriteram16[attr_start + 1] & 0x00f0) && (spriteram16[attr_start + 1] & 0x00f0) != 0xc0)
		{
			printf("attr %X\n",spriteram16[attr_start + 1] & 0x00f0);
			code = mame_rand(machine);
		}

/*
        if(spriteram16[attr_start + 1] & ~0x88cf)
            printf("1 = %X\n",spriteram16[attr_start + 1] & ~0x88cf);
*/
		if(code >= 0x2000)
		{
			gfx = 1;
			code -= 0x2000;
		}
		else
		{
			gfx = 0;
		}

		pdrawgfx(bitmap,machine->gfx[1 + gfx],
			code,
			color,
			flipx,flipy,
			x,y,
			cliprect,TRANSPARENCY_PEN,15,
			pri ? 0 : 0x2);

		// wrap around x
		pdrawgfx(bitmap,machine->gfx[1 + gfx],
			code,
			color,
			flipx,flipy,
			x - 512,y,
			cliprect,TRANSPARENCY_PEN,15,
			pri ? 0 : 0x2);
	}
}


VIDEO_UPDATE( f1gp )
{
	if (dirtygfx)
	{
		int i;

		dirtygfx = 0;

		for (i = 0;i < TOTAL_CHARS;i++)
		{
			if (dirtychar[i])
			{
				dirtychar[i] = 0;
				decodechar(screen->machine->gfx[3],i,(UINT8 *)zoomdata);
			}
		}

		tilemap_mark_all_tiles_dirty(roz_tilemap);
	}



	fillbitmap(priority_bitmap, 0, cliprect);

	K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,0,0);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,1);

	/* quick kludge for "continue" screen priority */
	if (gfxctrl == 0x00)
	{
		f1gp_draw_sprites(screen->machine,bitmap,cliprect,0,0x02);
		f1gp_draw_sprites(screen->machine,bitmap,cliprect,1,0x02);
	}
	else
	{
		f1gp_draw_sprites(screen->machine,bitmap,cliprect,0,0x00);
		f1gp_draw_sprites(screen->machine,bitmap,cliprect,1,0x02);
	}
	return 0;
}

VIDEO_UPDATE( f1gpb )
{
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;

	if (dirtygfx)
	{
		int i;

		dirtygfx = 0;

		for (i = 0;i < TOTAL_CHARS;i++)
		{
			if (dirtychar[i])
			{
				dirtychar[i] = 0;
				decodechar(screen->machine->gfx[3],i,(UINT8 *)zoomdata);
			}
		}

		tilemap_mark_all_tiles_dirty(roz_tilemap);
	}

	incxy = (INT16)f1gpb_rozregs[1];
	incyx = -incxy;
	incxx = incyy = (INT16)f1gpb_rozregs[3];
	startx = f1gpb_rozregs[0] + 328;
	starty = f1gpb_rozregs[2];

	tilemap_set_scrolly(fg_tilemap,0,f1gpb_fgregs[0] + 8);

	fillbitmap(priority_bitmap, 0, cliprect);

	tilemap_draw_roz(bitmap, cliprect, roz_tilemap,
		startx << 13, starty << 13,
		incxx << 5, incxy << 5, incyx << 5, incyy << 5,
		1, 0, 0);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,1);

	f1gpb_draw_sprites(screen->machine,bitmap,cliprect);

	return 0;
}


static void f1gp2_draw_sprites(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs;


	offs = 0;
	while (offs < 0x0400 && (f1gp2_spritelist[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox,oy,x,y,xsize,ysize,zoomx,zoomy,flipx,flipy,color;

		attr_start = 4 * (f1gp2_spritelist[offs++] & 0x01ff);

		ox = f1gp2_spritelist[attr_start + 1] & 0x01ff;
		xsize = (f1gp2_spritelist[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (f1gp2_spritelist[attr_start + 1] & 0xf000) >> 12;
		oy = f1gp2_spritelist[attr_start + 0] & 0x01ff;
		ysize = (f1gp2_spritelist[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (f1gp2_spritelist[attr_start + 0] & 0xf000) >> 12;
		flipx = f1gp2_spritelist[attr_start + 2] & 0x4000;
		flipy = f1gp2_spritelist[attr_start + 2] & 0x8000;
		color = (f1gp2_spritelist[attr_start + 2] & 0x1f00) >> 8;
		map_start = f1gp2_spritelist[attr_start + 3] & 0x7fff;

// aerofgt has the following adjustment, but doing it here would break the title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		if (f1gp2_spritelist[attr_start + 2] & 0x20ff) color = mame_rand(machine);

		for (y = 0;y <= ysize;y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0;x <= xsize;x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				code = f1gp2_sprcgram[map_start & 0x3fff];
				map_start++;

				if (flipscreen)
					drawgfxzoom(bitmap,machine->gfx[1],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							cliprect,TRANSPARENCY_PEN,15,
							zoomx << 11,zoomy << 11);
				else
					drawgfxzoom(bitmap,machine->gfx[1],
							code,
							color,
							flipx,flipy,
							sx,sy,
							cliprect,TRANSPARENCY_PEN,15,
							zoomx << 11,zoomy << 11);
			}
		}
	}
}


VIDEO_UPDATE( f1gp2 )
{
	if (gfxctrl & 4)	/* blank screen */
		fillbitmap(bitmap, get_black_pen(screen->machine), cliprect);
	else
	{
		switch (gfxctrl & 3)
		{
			case 0:
				K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,TILEMAP_DRAW_OPAQUE,0);
				f1gp2_draw_sprites(screen->machine,bitmap,cliprect);
				tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
				break;
			case 1:
				K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,TILEMAP_DRAW_OPAQUE,0);
				tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
				f1gp2_draw_sprites(screen->machine,bitmap,cliprect);
				break;
			case 2:
				tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_DRAW_OPAQUE,0);
				K053936_0_zoom_draw(bitmap,cliprect,roz_tilemap,0,0);
				f1gp2_draw_sprites(screen->machine,bitmap,cliprect);
				break;
#ifdef MAME_DEBUG
			case 3:
				popmessage("unsupported priority 3\n");
#endif
		}
	}
	return 0;
}
