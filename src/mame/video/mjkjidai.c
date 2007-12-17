#include "driver.h"


UINT8 *mjkjidai_videoram;

static int display_enable;
static tilemap *bg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	int attr = mjkjidai_videoram[tile_index + 0x800];
	int code = mjkjidai_videoram[tile_index] + ((attr & 0x1f) << 8);
	int color = mjkjidai_videoram[tile_index + 0x1000];
	SET_TILE_INFO(0,code,color >> 3,0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mjkjidai )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mjkjidai_videoram_w )
{
	mjkjidai_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( mjkjidai_ctrl_w )
{
	UINT8 *rom = memory_region(REGION_CPU1);

//  logerror("%04x: port c0 = %02x\n",activecpu_get_pc(),data);

	/* bit 0 = NMI enable */
	interrupt_enable_w(0,data & 1);

	/* bit 1 = flip screen */
	flip_screen_set(data & 0x02);

	/* bit 2 =display enable */
	display_enable = data & 0x04;

	/* bit 5 = coin counter */
	coin_counter_w(0,data & 0x20);

	/* bits 6-7 select ROM bank */
	if (data & 0xc0)
	{
		memory_set_bankptr(1,rom + 0x10000-0x4000 + ((data & 0xc0) << 8));
	}
	else
	{
		/* there is code flowing from 7fff to this bank so they have to be contiguous in memory */
		memory_set_bankptr(1,rom + 0x08000);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0x20-2;offs >= 0;offs -= 2)
	{
		int code = spriteram[offs] + ((spriteram_2[offs] & 0x1f) << 8);
		int color = (spriteram_3[offs] & 0x78) >> 3;
		int sx = 2*spriteram_2[offs+1];
		int sy = 240 - spriteram[offs+1];
		int flipx = code & 1;
		int flipy = code & 2;

		code >>= 2;

		sx += (spriteram_2[offs] & 0x20) >> 5;	// not sure about this

		if (flip_screen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx += 16;
		sy += 1;

		drawgfx(bitmap,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
}



VIDEO_UPDATE( mjkjidai )
{
	if (!display_enable)
	{
		fillbitmap(bitmap,get_black_pen(machine),cliprect);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
		draw_sprites(machine, bitmap,cliprect);
	}
	return 0;
}
