/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *retofinv_bg_videoram;
UINT8 *retofinv_fg_videoram;
UINT8 *retofinv_sharedram;

static int fg_bank,bg_bank;
static tilemap *bg_tilemap,*fg_tilemap;



PALETTE_INIT( retofinv )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i + 0*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 0*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + 0*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + 0*machine->drv->total_colors] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 1*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 1*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + 1*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + 1*machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 3*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup table */

	/* fg chars (1bpp) */
	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		if (i % 2)
			COLOR(0,i) = i/2;
		else
			COLOR(0,i) = 0;
	}

	/* sprites */
	for(i = 0;i < TOTAL_COLORS(2);i++)
		COLOR(2,i) = BITSWAP8(color_prom[i],4,5,6,7,3,2,1,0);

	/* bg tiles */
	for(i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = BITSWAP8(color_prom[TOTAL_COLORS(2) + i],4,5,6,7,3,2,1,0);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tilemap_scan )
{
	row += 2;
	col -= 2;
	if (col  & 0x20)
		return ((col & 0x1f) << 5) + row;
	else
		return (row << 5) + col;
}

static TILE_GET_INFO( bg_get_tile_info )
{
	SET_TILE_INFO(
			1,
			retofinv_bg_videoram[tile_index] + 256 * bg_bank,
			retofinv_bg_videoram[0x400 + tile_index] & 0x3f,
			0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	/* not sure about the transparency thing, but it makes sense */
	SET_TILE_INFO(
			0,
			retofinv_fg_videoram[tile_index] + 256 * fg_bank,
			retofinv_fg_videoram[0x400 + tile_index],
			(tile_index < 0x40 || tile_index >= 0x3c0) ? TILE_FORCE_LAYER0 : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( retofinv )
{
	bg_tilemap = tilemap_create(bg_get_tile_info,tilemap_scan,TILEMAP_TYPE_PEN,8,8,36,28);
	fg_tilemap = tilemap_create(fg_get_tile_info,tilemap_scan,TILEMAP_TYPE_COLORTABLE,8,8,36,28);

	tilemap_set_transparent_pen(fg_tilemap,0);

	spriteram = retofinv_sharedram + 0x0780;
	spriteram_2 = retofinv_sharedram + 0x0f80;
	spriteram_3 = retofinv_sharedram + 0x1780;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( retofinv_bg_videoram_w )
{
	retofinv_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( retofinv_fg_videoram_w )
{
	retofinv_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( retofinv_gfx_ctrl_w )
{
	switch (offset)
	{
		case 0:
			flip_screen_set(data & 1);
			break;

		case 1:
			if (fg_bank != (data & 1))
			{
				fg_bank = data & 1;
				tilemap_mark_all_tiles_dirty(fg_tilemap);
			}
			break;

		case 2:
			if (bg_bank != (data & 1))
			{
				bg_bank = data & 1;
				tilemap_mark_all_tiles_dirty(bg_tilemap);
			}
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap)
{
	int offs;
	static rectangle spritevisiblearea =
	{
		2*8, 34*8-1,
		0*8, 28*8-1
	};

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs];
		int color = spriteram[offs+1] & 0x3f;
		int sx = ((spriteram_2[offs+1] << 1) + ((spriteram_3[offs+1] & 0x80) >> 7)) - 39;
		int sy = 256 - ((spriteram_2[offs] << 1) + ((spriteram_3[offs] & 0x80) >> 7)) + 1;
		/* not sure about the flipping, it's hardly ever used (mostly for shots) */
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizey = (spriteram_3[offs] & 0x04) >> 2;
		int sizex = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen)
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;	// fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				drawgfx(bitmap,machine->gfx[2],
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x,sy + 16*y,
					&spritevisiblearea,TRANSPARENCY_COLOR,0xff);
			}
		}
	}
}



VIDEO_UPDATE( retofinv )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
