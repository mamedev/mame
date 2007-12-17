/******************************************************************************

Quiz DNA no Hanran (c) 1992 Face
Quiz Gakuen Paradise (c) 1991 NMK
Quiz Gekiretsu Scramble (Gakuen Paradise 2) (c) 1993 Face

Video hardware
    driver by Uki

******************************************************************************/

#include "driver.h"

static UINT8 *quizdna_bg_ram;
static UINT8 *quizdna_fg_ram;

static tilemap *quizdna_bg_tilemap;
static tilemap *quizdna_fg_tilemap;

static UINT8 quizdna_bg_xscroll[2];

static int quizdna_flipscreen = -1;
static int quizdna_video_enable;


static TILE_GET_INFO( get_bg_tile_info )
{
	int code = quizdna_bg_ram[tile_index*2] + quizdna_bg_ram[tile_index*2+1]*0x100 ;
	int col = quizdna_bg_ram[tile_index*2+0x1000] & 0x7f;

	if (code>0x7fff)
		code &= 0x83ff;

	SET_TILE_INFO(1, code, col, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code,col,x,y;
	UINT8 *FG = memory_region(REGION_USER1);

	x = tile_index & 0x1f;
	y = FG[(tile_index >> 5) & 0x1f] & 0x3f;
	code = y & 1;

	y >>= 1;

	col = quizdna_fg_ram[x*2 + y*0x40 + 1];
	code += (quizdna_fg_ram[x*2 + y*0x40] + (col & 0x1f) * 0x100) * 2;
	col >>= 5;
	col = (col & 3) | ((col & 4) << 1);

	SET_TILE_INFO(0, code, col, 0);
}


VIDEO_START( quizdna )
{
	quizdna_bg_ram = auto_malloc(0x2000);
	quizdna_fg_ram = auto_malloc(0x1000);

	quizdna_bg_tilemap = tilemap_create( get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32 );
	quizdna_fg_tilemap = tilemap_create( get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,8,32,32 );

	tilemap_set_transparent_pen( quizdna_fg_tilemap,0 );
}

WRITE8_HANDLER( quizdna_bg_ram_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	quizdna_bg_ram[offset] = data;
	RAM[0x12000+offset] = data;

	tilemap_mark_tile_dirty(quizdna_bg_tilemap, (offset & 0xfff) / 2 );
}

WRITE8_HANDLER( quizdna_fg_ram_w )
{
	int i;
	int offs = offset & 0xfff;
	UINT8 *RAM = memory_region(REGION_CPU1);

	RAM[0x10000+offs] = data;
	RAM[0x11000+offs] = data; /* mirror */
	quizdna_fg_ram[offs] = data;

	for (i=0; i<32; i++)
		tilemap_mark_tile_dirty(quizdna_fg_tilemap, ((offs/2) & 0x1f) + i*0x20 );
}

WRITE8_HANDLER( quizdna_bg_yscroll_w )
{
	tilemap_set_scrolldy( quizdna_bg_tilemap, 255-data, 255-data+1 );
}

WRITE8_HANDLER( quizdna_bg_xscroll_w )
{
	int x;
	quizdna_bg_xscroll[offset] = data;
	x = ~(quizdna_bg_xscroll[0] + quizdna_bg_xscroll[1]*0x100) & 0x1ff;

	tilemap_set_scrolldx( quizdna_bg_tilemap, x+64, x-64+10 );
}

WRITE8_HANDLER( quizdna_screen_ctrl_w )
{
	int tmp = (data & 0x10) >> 4;
	quizdna_video_enable = data & 0x20;

	coin_counter_w(0, data & 1);

	if (quizdna_flipscreen == tmp)
		return;

	quizdna_flipscreen = tmp;

	flip_screen_set(tmp);
	tilemap_set_scrolldx( quizdna_fg_tilemap, 64, -64 +16);
}

WRITE8_HANDLER( paletteram_xBGR_RRRR_GGGG_BBBB_w )
{
	int r,g,b,d0,d1;
	int offs = offset & ~1;

	paletteram[offset] = data;

	d0 = paletteram[offs];
	d1 = paletteram[offs+1];

	r = ((d1 << 1) & 0x1e) | ((d1 >> 4) & 1);
	g = ((d0 >> 3) & 0x1e) | ((d1 >> 5) & 1);
	b = ((d0 << 1) & 0x1e) | ((d1 >> 6) & 1);

	palette_set_color_rgb(Machine,offs/2,pal5bit(r),pal5bit(g),pal5bit(b));
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0; offs<spriteram_size; offs+=8)
	{
		int i;

		int x = spriteram[offs + 3]*0x100 + spriteram[offs + 2] + 64 - 8;
		int y = (spriteram[offs + 1] & 1)*0x100 + spriteram[offs + 0];
		int code = (spriteram[offs + 5] * 0x100 + spriteram[offs + 4]) & 0x3fff;
		int col =  spriteram[offs + 6];
		int fx = col & 0x80;
		int fy = col & 0x40;
		int ysize = (spriteram[offs + 1] & 0xc0) >> 6;
		int dy = 0x10;
		col &= 0x1f;

		if (quizdna_flipscreen)
		{
			x -= 7;
			y += 1;
		}

		x &= 0x1ff;
		if (x>0x1f0)
			x -= 0x200;

		if (fy)
		{
			dy = -0x10;
			y += 0x10 * ysize;
		}

		if (code >= 0x2100)
			code &= 0x20ff;

		for (i=0; i<ysize+1; i++)
		{
			y &= 0x1ff;

			drawgfx(bitmap,machine->gfx[2],
					code ^ i,
					col,
					fx,fy,
					x,y,
					cliprect,TRANSPARENCY_PEN,0);

			y += dy;
		}
	}
}

VIDEO_UPDATE( quizdna )
{
	if (quizdna_video_enable)
	{
		tilemap_draw(bitmap, cliprect, quizdna_bg_tilemap, 0, 0);
		draw_sprites(machine, bitmap, cliprect);
		tilemap_draw(bitmap, cliprect, quizdna_fg_tilemap, 0, 0);
	}
	else
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
	return 0;
}
