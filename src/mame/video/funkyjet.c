/***************************************************************************

   Funky Jet Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/funkyjet.h"
#include "video/deco16ic.h"

/******************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	funkyjet_state *state = (funkyjet_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs;

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y,sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = spriteram[offs + 1] & 0x3fff;

		y = spriteram[offs];
		flash = y & 0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		x = spriteram[offs + 2];
		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		if (x > 320)
			continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen_get(machine))
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

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

VIDEO_UPDATE( funkyjet )
{
	funkyjet_state *state = (funkyjet_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);

	bitmap_fill(bitmap, cliprect, 768);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
