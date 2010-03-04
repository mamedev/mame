/***************************************************************************

   Crude Buster Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/cbuster.h"
#include "video/deco16ic.h"

/******************************************************************************/

static void update_24bitcol( running_machine *machine, int offset )
{
	UINT8 r, g, b; /* The highest palette value seems to be 0x8e */

	r = (UINT8)((float)((machine->generic.paletteram.u16[offset]  >> 0) & 0xff) * 1.75);
	g = (UINT8)((float)((machine->generic.paletteram.u16[offset]  >> 8) & 0xff) * 1.75);
	b = (UINT8)((float)((machine->generic.paletteram2.u16[offset] >> 0) & 0xff) * 1.75);

	palette_set_color(machine, offset, MAKE_RGB(r, g, b));
}

WRITE16_HANDLER( twocrude_palette_24bit_rg_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	update_24bitcol(space->machine, offset);
}

WRITE16_HANDLER( twocrude_palette_24bit_b_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram2.u16[offset]);
	update_24bitcol(space->machine, offset);
}


/******************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri )
{
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
	int offs;

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = buffered_spriteram[offs + 1] & 0x7fff;
		if (!sprite)
			continue;

		y = buffered_spriteram[offs];
		x = buffered_spriteram[offs + 2];

		if ((y & 0x8000) && pri == 1)
			continue;
		if (!(y & 0x8000) && pri == 0)
			continue;

		colour = (x >> 9) & 0xf;
		if (x & 0x2000)
			colour += 64;

		flash = y & 0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (x > 256)
			continue; /* Speedup */

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
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

/******************************************************************************/

VIDEO_UPDATE( twocrude )
{
	cbuster_state *state = (cbuster_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, !BIT(flip, 7));

	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);
	deco16ic_pf34_update(state->deco16ic, state->pf3_rowscroll, state->pf4_rowscroll);

	/* Draw playfields & sprites */
	deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 0);

	if (state->pri)
	{
		deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
		deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	}
	else
	{
		deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 0);
		deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	}

	draw_sprites(screen->machine, bitmap, cliprect, 1);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}
