#include "driver.h"


UINT16 *sf_objectram,*sf_videoram;

static int sf_active = 0;

static tilemap *bg_tilemap, *fg_tilemap, *tx_tilemap;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *base = memory_region(REGION_GFX5) + 2*tile_index;
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000+1]<<8) | base[1];
	SET_TILE_INFO(
			0,
			code,
			color,
			TILE_FLIPYX(attr & 3));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 *base = memory_region(REGION_GFX5) + 0x20000 + 2*tile_index;
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000+1]<<8) | base[1];
	SET_TILE_INFO(
			1,
			code,
			color,
			TILE_FLIPYX(attr & 3));
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = sf_videoram[tile_index];
	SET_TILE_INFO(
			3,
			code & 0x3ff,
			code>>12,
			TILE_FLIPYX((code & 0xc00)>>10));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( sf )
{
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,     16,16,2048,16);
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,16,16,2048,16);
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,  64,32);

	tilemap_set_transparent_pen(fg_tilemap,15);
	tilemap_set_transparent_pen(tx_tilemap,3);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( sf_videoram_w )
{
	COMBINE_DATA(&sf_videoram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

WRITE16_HANDLER( sf_bg_scroll_w )
{
	static UINT16 scroll;
	COMBINE_DATA(&scroll);
	tilemap_set_scrollx(bg_tilemap,0,scroll);
}

WRITE16_HANDLER( sf_fg_scroll_w )
{
	static UINT16 scroll;
	COMBINE_DATA(&scroll);
	tilemap_set_scrollx(fg_tilemap,0,scroll);
}

WRITE16_HANDLER( sf_gfxctrl_w )
{
/* b0 = reset, or maybe "set anyway" */
/* b1 = pulsed when control6.b6==0 until it's 1 */
/* b2 = active when dip 8 (flip) on */
/* b3 = active character plane */
/* b4 = unused */
/* b5 = active background plane */
/* b6 = active middle plane */
/* b7 = active sprites */
	if (ACCESSING_LSB)
	{
		sf_active = data & 0xff;
		flip_screen_set(data & 0x04);
		tilemap_set_enable(tx_tilemap,data & 0x08);
		tilemap_set_enable(bg_tilemap,data & 0x20);
		tilemap_set_enable(fg_tilemap,data & 0x40);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

INLINE int sf_invert(int nb)
{
	static int delta[4] = {0x00, 0x18, 0x18, 0x00};
	return nb ^ delta[(nb >> 3) & 3];
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0x1000-0x20;offs >= 0;offs -= 0x20)
	{
		int c = sf_objectram[offs];
		int attr = sf_objectram[offs+1];
		int sy = sf_objectram[offs+2];
		int sx = sf_objectram[offs+3];
		int color = attr & 0x000f;
		int flipx = attr & 0x0100;
		int flipy = attr & 0x0200;

		if (attr & 0x400)	/* large sprite */
		{
			int c1,c2,c3,c4,t;

			if (flip_screen)
			{
				sx = 480 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			c1 = c;
			c2 = c+1;
			c3 = c+16;
			c4 = c+17;

			if (flipx)
			{
				t = c1; c1 = c2; c2 = t;
				t = c3; c3 = c4; c4 = t;
			}
			if (flipy)
			{
				t = c1; c1 = c3; c3 = t;
				t = c2; c2 = c4; c4 = t;
			}

			drawgfx(bitmap,
					machine->gfx[2],
					sf_invert(c1),
					color,
					flipx,flipy,
					sx,sy,
					cliprect, TRANSPARENCY_PEN, 15);
			drawgfx(bitmap,
					machine->gfx[2],
					sf_invert(c2),
					color,
					flipx,flipy,
					sx+16,sy,
					cliprect, TRANSPARENCY_PEN, 15);
			drawgfx(bitmap,
					machine->gfx[2],
					sf_invert(c3),
					color,
					flipx,flipy,
					sx,sy+16,
					cliprect, TRANSPARENCY_PEN, 15);
			drawgfx(bitmap,
					machine->gfx[2],
					sf_invert(c4),
					color,
					flipx,flipy,
					sx+16,sy+16,
					cliprect, TRANSPARENCY_PEN, 15);
		}
		else
		{
			if (flip_screen)
			{
				sx = 496 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap,
					machine->gfx[2],
					sf_invert(c),
					color,
					flipx,flipy,
					sx,sy,
					cliprect, TRANSPARENCY_PEN, 15);
		}
	}
}


VIDEO_UPDATE( sf )
{
	if (sf_active & 0x20)
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	else
		fillbitmap(bitmap,machine->pens[0],cliprect);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	if (sf_active & 0x80)
		draw_sprites(machine,bitmap,cliprect);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}
