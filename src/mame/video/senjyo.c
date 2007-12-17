/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "machine/segacrpt.h"

UINT8 *senjyo_fgscroll;
UINT8 *senjyo_scrollx1,*senjyo_scrolly1;
UINT8 *senjyo_scrollx2,*senjyo_scrolly2;
UINT8 *senjyo_scrollx3,*senjyo_scrolly3;
UINT8 *senjyo_fgvideoram,*senjyo_fgcolorram;
UINT8 *senjyo_bg1videoram,*senjyo_bg2videoram,*senjyo_bg3videoram;
UINT8 *senjyo_radarram;
UINT8 *senjyo_bgstripesram;

static tilemap *fg_tilemap,*bg1_tilemap,*bg2_tilemap,*bg3_tilemap;

static int senjyo, scrollhack;
static int senjyo_bgstripes;


DRIVER_INIT( starforc )
{
	senjyo = 0;
	scrollhack = 1;
}
DRIVER_INIT( starfore )
{
	/* encrypted CPU */
	suprloco_decode();

	senjyo = 0;
	scrollhack = 0;
}
DRIVER_INIT( senjyo )
{
	senjyo = 1;
	scrollhack = 0;
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 attr = senjyo_fgcolorram[tile_index];
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	if (senjyo && (tile_index & 0x1f) >= 32-8)
		flags |= TILE_FORCE_LAYER0;

	SET_TILE_INFO(
			0,
			senjyo_fgvideoram[tile_index] + ((attr & 0x10) << 4),
			attr & 0x07,
			flags);
}

static TILE_GET_INFO( senjyo_bg1_tile_info )
{
	UINT8 code = senjyo_bg1videoram[tile_index];
	SET_TILE_INFO(
			1,
			code,
			(code & 0x70) >> 4,
			0);
}

static TILE_GET_INFO( starforc_bg1_tile_info )
{
	/* Star Force has more tiles in bg1, so to get a uniform color code spread */
	/* they wired bit 7 of the tile code in place of bit 4 to get the color code */
	static int colormap[8] = { 0,2,4,6,1,3,5,7 };
	UINT8 code = senjyo_bg1videoram[tile_index];
	SET_TILE_INFO(
			1,
			code,
			colormap[(code & 0xe0) >> 5],
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	UINT8 code = senjyo_bg2videoram[tile_index];
	SET_TILE_INFO(
			2,
			code,
			(code & 0xe0) >> 5,
			0);
}

static TILE_GET_INFO( get_bg3_tile_info )
{
	UINT8 code = senjyo_bg3videoram[tile_index];
	SET_TILE_INFO(
			3,
			code,
			(code & 0xe0) >> 5,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( senjyo )
{
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	if (senjyo)
	{
		bg1_tilemap = tilemap_create(senjyo_bg1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,32);
		bg2_tilemap = tilemap_create(get_bg2_tile_info,   tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,48);	/* only 16x32 used by Star Force */
		bg3_tilemap = tilemap_create(get_bg3_tile_info,   tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,56);	/* only 16x32 used by Star Force */
	}
	else
	{
		bg1_tilemap = tilemap_create(starforc_bg1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,32);
		bg2_tilemap = tilemap_create(get_bg2_tile_info,     tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,32);	/* only 16x32 used by Star Force */
		bg3_tilemap = tilemap_create(get_bg3_tile_info,     tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,32);	/* only 16x32 used by Star Force */
	}

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(bg1_tilemap,0);
	tilemap_set_transparent_pen(bg2_tilemap,0);
	tilemap_set_transparent_pen(bg3_tilemap,0);
	tilemap_set_scroll_cols(fg_tilemap,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( senjyo_fgvideoram_w )
{
	senjyo_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}
WRITE8_HANDLER( senjyo_fgcolorram_w )
{
	senjyo_fgcolorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}
WRITE8_HANDLER( senjyo_bg1videoram_w )
{
	senjyo_bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(bg1_tilemap,offset);
}
WRITE8_HANDLER( senjyo_bg2videoram_w )
{
	senjyo_bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(bg2_tilemap,offset);
}
WRITE8_HANDLER( senjyo_bg3videoram_w )
{
	senjyo_bg3videoram[offset] = data;
	tilemap_mark_tile_dirty(bg3_tilemap,offset);
}

WRITE8_HANDLER( senjyo_bgstripes_w )
{
	*senjyo_bgstripesram = data;
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_bgbitmap(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int x,y,pen,strwid,count;


	if (senjyo_bgstripes == 0xff)	/* off */
	{
		fillbitmap(bitmap,machine->pens[0],cliprect);
	}
	else
	{
		pen = 0;
		count = 0;
		strwid = senjyo_bgstripes;
		if (strwid == 0) strwid = 0x100;
		if (flip_screen) strwid ^= 0xff;

		for (x = 0;x < 256;x++)
		{
			if (flip_screen)
			{
				for (y = 0;y < 256;y++)
					*BITMAP_ADDR16(bitmap, y, 255 - x) = machine->pens[384 + pen];
			}
			else
			{
				for (y = 0;y < 256;y++)
					*BITMAP_ADDR16(bitmap, y, x) = machine->pens[384 + pen];
			}

			count += 0x10;
			if (count >= strwid)
			{
				pen = (pen + 1) & 0x0f;
				count -= strwid;
			}
		}
	}
}

static void draw_radar(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs,x;

	for (offs = 0;offs < 0x400;offs++)
	{
		if (senjyo_radarram[offs])
		{
			for (x = 0;x < 8;x++)
			{
				if (senjyo_radarram[offs] & (1 << x))
				{
					int sx, sy;

					sx = (8 * (offs % 8) + x) + 256-64;
					sy = ((offs & 0x1ff) / 8) + 96;

					if (flip_screen)
					{
						sx = 255 - sx;
						sy = 255 - sy;
					}

					if (sy >= cliprect->min_y && sy <= cliprect->max_y &&
						sx >= cliprect->min_x && sx <= cliprect->max_x)
						*BITMAP_ADDR16(bitmap, sy, sx) = machine->pens[offs < 0x200 ? 512 : 513];
				}
			}
		}
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int priority)
{
	int offs;


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int big,sx,sy,flipx,flipy;

		if (((spriteram[offs+1] & 0x30) >> 4) == priority)
		{
			if (senjyo)	/* Senjyo */
				big = (spriteram[offs] & 0x80);
			else	/* Star Force */
				big = ((spriteram[offs] & 0xc0) == 0xc0);
			sx = spriteram[offs+3];
			if (big)
				sy = 224-spriteram[offs+2];
			else
				sy = 240-spriteram[offs+2];
			flipx = spriteram[offs+1] & 0x40;
			flipy = spriteram[offs+1] & 0x80;

			if (flip_screen)
			{
				flipx = !flipx;
				flipy = !flipy;

				if (big)
				{
					sx = 224 - sx;
					sy = 226 - sy;
				}
				else
				{
					sx = 240 - sx;
					sy = 242 - sy;
				}
			}


			drawgfx(bitmap,machine->gfx[big ? 5 : 4],
					spriteram[offs],
					spriteram[offs + 1] & 0x07,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( senjyo )
{
	int i;


	/* two colors for the radar dots (verified on the real board) */
	palette_set_color(machine,512,MAKE_RGB(0xff,0x00,0x00));	/* red for enemies */
	palette_set_color(machine,513,MAKE_RGB(0xff,0xff,0x00));	/* yellow for player */

	{
		int scrollx,scrolly;

		for (i = 0;i < 32;i++)
			tilemap_set_scrolly(fg_tilemap,i,senjyo_fgscroll[i]);

		scrollx = senjyo_scrollx1[0];
		scrolly = senjyo_scrolly1[0] + 256 * senjyo_scrolly1[1];
		if (flip_screen)
			scrollx = -scrollx;
		tilemap_set_scrollx(bg1_tilemap,0,scrollx);
		tilemap_set_scrolly(bg1_tilemap,0,scrolly);

		scrollx = senjyo_scrollx2[0];
		scrolly = senjyo_scrolly2[0] + 256 * senjyo_scrolly2[1];
		if (scrollhack)	/* Star Force, but NOT the encrypted version */
		{
			scrollx = senjyo_scrollx1[0];
			scrolly = senjyo_scrolly1[0] + 256 * senjyo_scrolly1[1];
		}
		if (flip_screen)
			scrollx = -scrollx;
		tilemap_set_scrollx(bg2_tilemap,0,scrollx);
		tilemap_set_scrolly(bg2_tilemap,0,scrolly);

		scrollx = senjyo_scrollx3[0];
		scrolly = senjyo_scrolly3[0] + 256 * senjyo_scrolly3[1];
		if (flip_screen)
			scrollx = -scrollx;
		tilemap_set_scrollx(bg3_tilemap,0,scrollx);
		tilemap_set_scrolly(bg3_tilemap,0,scrolly);
	}

	draw_bgbitmap(machine, bitmap,cliprect);
	draw_sprites(machine, bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,bg3_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect,1);
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect,2);
	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect,3);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_radar(machine, bitmap,cliprect);

#if 0
{
	char baf[80];

	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x00],
		senjyo_scrolly3[0x01],
		senjyo_scrolly3[0x02],
		senjyo_scrolly3[0x03],
		senjyo_scrolly3[0x04],
		senjyo_scrolly3[0x05],
		senjyo_scrolly3[0x06],
		senjyo_scrolly3[0x07]);
	ui_draw_text(baf,0,0);
	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x08],
		senjyo_scrolly3[0x09],
		senjyo_scrolly3[0x0a],
		senjyo_scrolly3[0x0b],
		senjyo_scrolly3[0x0c],
		senjyo_scrolly3[0x0d],
		senjyo_scrolly3[0x0e],
		senjyo_scrolly3[0x0f]);
	ui_draw_text(baf,0,10);
	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x10],
		senjyo_scrolly3[0x11],
		senjyo_scrolly3[0x12],
		senjyo_scrolly3[0x13],
		senjyo_scrolly3[0x14],
		senjyo_scrolly3[0x15],
		senjyo_scrolly3[0x16],
		senjyo_scrolly3[0x17]);
	ui_draw_text(baf,0,20);
	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x18],
		senjyo_scrolly3[0x19],
		senjyo_scrolly3[0x1a],
		senjyo_scrolly3[0x1b],
		senjyo_scrolly3[0x1c],
		senjyo_scrolly3[0x1d],
		senjyo_scrolly3[0x1e],
		senjyo_scrolly3[0x1f]);
	ui_draw_text(baf,0,30);
}
#endif
	return 0;
}
