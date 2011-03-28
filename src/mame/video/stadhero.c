/***************************************************************************

  stadhero video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    MXC-06 chip to produce sprites, see dec0.c
    BAC-06 chip for background
	??? for text layer

***************************************************************************/

#include "emu.h"
#include "includes/stadhero.h"
#include "video/decbac06.h"

/******************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int pri_mask,int pri_val)
{
	stadhero_state *state = machine->driver_data<stadhero_state>();
	UINT16 *spriteram16 = state->spriteram;
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		y = spriteram16[offs];
		if ((y&0x8000) == 0) continue;

		x = spriteram16[offs+2];
		colour = x >> 12;
		if ((colour & pri_mask) != pri_val) continue;

		flash=x&0x800;
		if (flash && (machine->primary_screen->frame_number() & 1)) continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = spriteram16[offs+1] & 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->flipscreen) {
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);
			multi--;
		}
	}
}

/******************************************************************************/

SCREEN_UPDATE( stadhero )
{
	stadhero_state *state = screen->machine->driver_data<stadhero_state>();
//	tilemap_set_flip_all(screen->machine,state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen->machine->device<deco_bac06_device>("tilegen1")->set_bppmultmask(0x8, 0x7);
	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

	//pf2 draw
	draw_sprites(screen->machine, bitmap,cliprect,0x00,0x00);
	tilemap_draw(bitmap,cliprect,state->pf1_tilemap,0,0);
	return 0;
}

/******************************************************************************/

WRITE16_HANDLER( stadhero_pf1_data_w )
{
	stadhero_state *state = space->machine->driver_data<stadhero_state>();
	COMBINE_DATA(&state->pf1_data[offset]);
	tilemap_mark_tile_dirty(state->pf1_tilemap,offset);
}


/******************************************************************************/

static TILE_GET_INFO( get_pf1_tile_info )
{
	stadhero_state *state = machine->driver_data<stadhero_state>();
	int tile=state->pf1_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( stadhero )
{
	stadhero_state *state = machine->driver_data<stadhero_state>();
	state->pf1_tilemap =     tilemap_create(machine, get_pf1_tile_info,tilemap_scan_rows, 8, 8,32,32);
	tilemap_set_transparent_pen(state->pf1_tilemap,0);
}

/******************************************************************************/
