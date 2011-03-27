/***************************************************************************

  Mad Motor video emulation - Bryan McPhail, mish@tendril.co.uk

  Notes:  Playfield 3 can change size between 512x1024 and 2048x256

***************************************************************************/

#include "emu.h"
#include "includes/madmotor.h"
#include "video/decbac06.h"

/******************************************************************************/

VIDEO_START( madmotor )
{
}

/******************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri_mask, int pri_val )
{
	madmotor_state *state = machine->driver_data<madmotor_state>();
	UINT16 *spriteram = state->spriteram;
	int offs;

	offs = 0;
	while (offs < state->spriteram_size / 2)
	{
		int sx, sy, code, color, w, h, flipx, flipy, incy, flash, mult, x, y;

		sy = spriteram[offs];
		sx = spriteram[offs + 2];
		color = sx >> 12;

		flash = sx & 0x800;

		flipx = sy & 0x2000;
		flipy = sy & 0x4000;
		h = (1 << ((sy & 0x1800) >> 11));	/* 1x, 2x, 4x, 8x height */
		w = (1 << ((sy & 0x0600) >>  9));	/* 1x, 2x, 4x, 8x width */
		/* multi width used only on the title screen? */

		code = spriteram[offs + 1] & 0x1fff;

		sx = sx & 0x01ff;
		sy = sy & 0x01ff;
		if (sx >= 256) sx -= 512;
		if (sy >= 256) sy -= 512;
		sx = 240 - sx;
		sy = 240 - sy;

		code &= ~(h-1);
		if (flipy)
			incy = -1;
		else
		{
			code += h-1;
			incy = 1;
		}

		if (state->flipscreen)
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (flipx) flipx = 0; else flipx = 1;
			if (flipy) flipy = 0; else flipy = 1;
			mult = 16;
		}
		else
			mult = -16;

		for (x = 0; x < w; x++)
		{
			for (y = 0; y < h; y++)
			{
				if ((color & pri_mask) == pri_val &&
							(!flash || (machine->primary_screen->frame_number() & 1)))
					drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
							code - y * incy + h * x,
							color,
							flipx,flipy,
							sx + mult * x,sy + mult * y,0);
			}

			offs += 4;
			if (offs >= state->spriteram_size / 2 || spriteram[offs] & 0x8000)	// seems the expected behaviour on the title screen
				 break;
		}
	}
}


/******************************************************************************/

SCREEN_UPDATE( madmotor )
{
	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

//	tilemap_set_flip_all(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state() ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	draw_sprites(screen->machine, bitmap, cliprect, 0x00, 0x00);
	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}
