/*******************************************************************************

    actfancr - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/actfancr.h"
#include "video/decbac06.h"

/******************************************************************************/

static void register_savestate( running_machine *machine )
{
	actfancr_state *state = machine->driver_data<actfancr_state>();
	state->save_item(NAME(state->flipscreen));
}

VIDEO_START( actfancr )
{
	register_savestate(machine);
}

VIDEO_START( triothep )
{
	register_savestate(machine);
}

/******************************************************************************/

SCREEN_UPDATE( actfancr )
{
	actfancr_state *state = screen->machine->driver_data<actfancr_state>();
	UINT8 *buffered_spriteram = screen->machine->generic.buffered_spriteram.u8;
	int offs, mult;

	/* Draw playfield */
	//state->flipscreen = state->control_2[0] & 0x80;
	//tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

	/* Sprites */
	for (offs = 0; offs < 0x800; offs += 8)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash;

		y = buffered_spriteram[offs] + (buffered_spriteram[offs + 1] << 8);
		if ((y & 0x8000) == 0)
			continue;

		x = buffered_spriteram[offs + 4] + (buffered_spriteram[offs + 5] << 8);
		colour = ((x & 0xf000) >> 12);
		flash = x & 0x800;
		if (flash && (screen->frame_number() & 1))
			continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
										/* multi = 0   1   3   7 */

		sprite = buffered_spriteram[offs + 2] + (buffered_spriteram[offs + 3] << 8);
		sprite &= 0x0fff;

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

		if (state->flipscreen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);
			multi--;
		}
	}

	screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	return 0;
}

SCREEN_UPDATE( triothep )
{
	actfancr_state *state = screen->machine->driver_data<actfancr_state>();
	UINT8 *buffered_spriteram = screen->machine->generic.buffered_spriteram.u8;
	int offs, mult;
	
	/* Draw playfield */
	//state->flipscreen = state->control_2[0] & 0x80;
	//tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

	/* Sprites */
	for (offs = 0; offs < 0x800; offs += 8)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash;

		y = buffered_spriteram[offs] + (buffered_spriteram[offs + 1] << 8);
		if ((y & 0x8000) == 0)
			continue;

		x = buffered_spriteram[offs + 4] + (buffered_spriteram[offs + 5] << 8);
		colour = ((x & 0xf000) >> 12);
		flash = x & 0x800;
		if (flash && (screen->frame_number() & 1))
			continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = buffered_spriteram[offs + 2] + (buffered_spriteram[offs + 3] << 8);
		sprite &= 0x0fff;

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

		if (state->flipscreen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);
			multi--;
		}
	}

	screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	return 0;
}
