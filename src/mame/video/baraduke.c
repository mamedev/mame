#include "driver.h"

UINT8 *baraduke_textram, *baraduke_videoram, *baraduke_spriteram;

static tilemap *tx_tilemap, *bg_tilemap[2];
static int xscroll[2], yscroll[2];



/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( baraduke )
{
	int i;
	int bit0,bit1,bit2,bit3,r,g,b;

	for (i = 0; i < 2048; i++)
	{
		/* red component */
		bit0 = (color_prom[2048] >> 0) & 0x01;
		bit1 = (color_prom[2048] >> 1) & 0x01;
		bit2 = (color_prom[2048] >> 2) & 0x01;
		bit3 = (color_prom[2048] >> 3) & 0x01;
		r = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		/* green component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		g = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		/* blue component */
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		b = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tx_tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( tx_get_tile_info )
{
	SET_TILE_INFO(
			0,
			baraduke_textram[tile_index],
			(baraduke_textram[tile_index+0x400] << 2) & 0x1ff,
			0);
}

static TILE_GET_INFO( get_tile_info0 )
{
	int code = baraduke_videoram[2*tile_index];
	int attr = baraduke_videoram[2*tile_index + 1];

	SET_TILE_INFO(
			1,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	int code = baraduke_videoram[0x1000 + 2*tile_index];
	int attr = baraduke_videoram[0x1000 + 2*tile_index + 1];

	SET_TILE_INFO(
			2,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( baraduke )
{
	tx_tilemap = tilemap_create(tx_get_tile_info,tx_tilemap_scan,TILEMAP_TYPE_PEN,8,8,36,28);
	bg_tilemap[0] = tilemap_create(get_tile_info0,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);
	bg_tilemap[1] = tilemap_create(get_tile_info1,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);

	tilemap_set_transparent_pen(tx_tilemap,3);
	tilemap_set_transparent_pen(bg_tilemap[0],7);
	tilemap_set_transparent_pen(bg_tilemap[1],7);

	tilemap_set_scrolldx(tx_tilemap,0,512-288);
	tilemap_set_scrolldy(tx_tilemap,16,16);

	spriteram = baraduke_spriteram + 0x1800;
}



/***************************************************************************

    Memory handlers

***************************************************************************/

READ8_HANDLER( baraduke_videoram_r )
{
	return baraduke_videoram[offset];
}

WRITE8_HANDLER( baraduke_videoram_w )
{
	baraduke_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap[offset/0x1000],(offset&0xfff)/2);
}

READ8_HANDLER( baraduke_textram_r )
{
	return baraduke_textram[offset];
}

WRITE8_HANDLER( baraduke_textram_w )
{
	baraduke_textram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset & 0x3ff);
}


static void scroll_w(int layer,int offset,int data)
{
	switch (offset)
	{
		case 0:	/* high scroll x */
			xscroll[layer] = (xscroll[layer] & 0xff) | (data << 8);
			break;
		case 1:	/* low scroll x */
			xscroll[layer] = (xscroll[layer] & 0xff00) | data;
			break;
		case 2:	/* scroll y */
			yscroll[layer] = data;
			break;
	}
}

WRITE8_HANDLER( baraduke_scroll0_w )
{
	scroll_w(0, offset, data);
}
WRITE8_HANDLER( baraduke_scroll1_w )
{
	scroll_w(1, offset, data);
}


static int copy_sprites;

READ8_HANDLER( baraduke_spriteram_r )
{
	return baraduke_spriteram[offset];
}

WRITE8_HANDLER( baraduke_spriteram_w )
{
	baraduke_spriteram[offset] = data;

	/* a write to this offset tells the sprite chip to buffer the sprite list */
	if (offset == 0x1ff2)
		copy_sprites = 1;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int sprite_priority)
{
	const UINT8 *source = &spriteram[0];
	const UINT8 *finish = &spriteram[0x0800-16];	/* the last is NOT a sprite */

	int sprite_xoffs = spriteram[0x07f5] - 256 * (spriteram[0x07f4] & 1);
	int sprite_yoffs = spriteram[0x07f7];

	while( source<finish )
	{
/*
    source[10] S-FT ---P
    source[11] TTTT TTTT
    source[12] CCCC CCCX
    source[13] XXXX XXXX
    source[14] ---T -S-F
    source[15] YYYY YYYY
*/
		int priority = source[10] & 0x01;
		if (priority == sprite_priority)
		{
			static const int gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int attr1 = source[10];
			int attr2 = source[14];
			int color = source[12];
			int sx = source[13] + (color & 0x01)*256;
			int sy = 240 - source[15];
			int flipx = (attr1 & 0x20) >> 5;
			int flipy = (attr2 & 0x01);
			int sizex = (attr1 & 0x80) >> 7;
			int sizey = (attr2 & 0x04) >> 2;
			int sprite = (source[11] & 0xff)*4;
			int x,y;

			if ((attr1 & 0x10) && !sizex) sprite += 1;
			if ((attr2 & 0x10) && !sizey) sprite += 2;
			color = color >> 1;

			sx += sprite_xoffs;
			sy -= sprite_yoffs;

			sy -= 16 * sizey;

			if (flip_screen)
			{
				sx = 496+3 - 16 * sizex - sx;
				sy = 240 - 16 * sizey - sy;
				flipx ^= 1;
				flipy ^= 1;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					drawgfx( bitmap, machine->gfx[3],
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						-71 + ((sx + 16*x) & 0x1ff),
						1 + ((sy + 16*y) & 0xff),
						cliprect,TRANSPARENCY_PEN,0xf);
				}
			}
		}

		source+=16;
	}
}


static void set_scroll(int layer)
{
	static const int xdisp[2] = { 26, 24 };
	int scrollx, scrolly;

	scrollx = xscroll[layer] + xdisp[layer];
	scrolly = yscroll[layer] + 9;
	if (flip_screen)
	{
		scrollx = -scrollx + 3;
		scrolly = -scrolly;
	}

	tilemap_set_scrollx(bg_tilemap[layer], 0, scrollx);
	tilemap_set_scrolly(bg_tilemap[layer], 0, scrolly);
}


VIDEO_UPDATE( baraduke )
{
	int back;

	/* flip screen is embedded in the sprite control registers */
	/* can't use flip_screen_set() because the visible area is asymmetrical */
	flip_screen = spriteram[0x07f6] & 0x01;
	tilemap_set_flip(ALL_TILEMAPS,flip_screen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	set_scroll(0);
	set_scroll(1);

	if (((xscroll[0] & 0x0e00) >> 9) == 6)
		back = 1;
	else
		back = 0;

	tilemap_draw(bitmap,cliprect,bg_tilemap[back],TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(machine, bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,bg_tilemap[back ^ 1],0,0);
	draw_sprites(machine, bitmap,cliprect,1);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}


VIDEO_EOF( baraduke )
{
	if (copy_sprites)
	{
		int i,j;

		for (i = 0;i < 0x800;i += 16)
		{
			for (j = 10;j < 16;j++)
				spriteram[i+j] = spriteram[i+j - 6];
		}

		copy_sprites = 0;
	}
}
