#include "emu.h"
#include "includes/mjkjidai.h"

static int display_enable;
static tilemap_t *bg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	mjkjidai_state *state = (mjkjidai_state *)machine->driver_data;

	int attr = state->videoram[tile_index + 0x800];
	int code = state->videoram[tile_index] + ((attr & 0x1f) << 8);
	int color = state->videoram[tile_index + 0x1000];
	SET_TILE_INFO(0,code,color >> 3,0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mjkjidai )
{
	bg_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,64,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mjkjidai_videoram_w )
{
	mjkjidai_state *state = (mjkjidai_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( mjkjidai_ctrl_w )
{
	UINT8 *rom = memory_region(space->machine, "maincpu");

//  logerror("%04x: port c0 = %02x\n",cpu_get_pc(space->cpu),data);

	/* bit 0 = NMI enable */
	interrupt_enable_w(space,0,data & 1);

	/* bit 1 = flip screen */
	flip_screen_set(space->machine, data & 0x02);

	/* bit 2 =display enable */
	display_enable = data & 0x04;

	/* bit 5 = coin counter */
	coin_counter_w(space->machine, 0,data & 0x20);

	/* bits 6-7 select ROM bank */
	if (data & 0xc0)
	{
		memory_set_bankptr(space->machine, "bank1",rom + 0x10000-0x4000 + ((data & 0xc0) << 8));
	}
	else
	{
		/* there is code flowing from 7fff to this bank so they have to be contiguous in memory */
		memory_set_bankptr(space->machine, "bank1",rom + 0x08000);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	mjkjidai_state *state = (mjkjidai_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram1;
	UINT8 *spriteram_2 = state->spriteram2;
	UINT8 *spriteram_3 = state->spriteram3;
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

		if (flip_screen_get(machine))
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx += 16;
		sy += 1;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}



VIDEO_UPDATE( mjkjidai )
{
	if (!display_enable)
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	else
	{
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
		draw_sprites(screen->machine, bitmap,cliprect);
	}
	return 0;
}
