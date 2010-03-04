/***************************************************************************

   Vapour Trail Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

    2 Data East 55 chips for playfields (same as Dark Seal, etc)
    1 Data East MXC-06 chip for sprites (same as Bad Dudes, etc)

***************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/vaportra.h"

/******************************************************************************/

WRITE16_HANDLER( vaportra_priority_w )
{
	vaportra_state *state = (vaportra_state *)space->machine->driver_data;
	COMBINE_DATA(&state->priority[offset]);
}

/******************************************************************************/

static void update_24bitcol( running_machine *machine, int offset )
{
	UINT8 r, g, b;

	r = (machine->generic.paletteram.u16[offset] >> 0) & 0xff;
	g = (machine->generic.paletteram.u16[offset] >> 8) & 0xff;
	b = (machine->generic.paletteram2.u16[offset] >> 0) & 0xff;

	palette_set_color(machine, offset, MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( vaportra_palette_24bit_rg_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	update_24bitcol(space->machine, offset);
}

WRITE16_HANDLER( vaportra_palette_24bit_b_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram2.u16[offset]);
	update_24bitcol(space->machine, offset);
}

/******************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri )
{
	vaportra_state *state = (vaportra_state *)machine->driver_data;
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
	int offs;
	int priority_value = state->priority[1];

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		y = buffered_spriteram[offs];
		if ((y & 0x8000) == 0)
			continue;

		sprite = buffered_spriteram[offs + 1] & 0x1fff;
		x = buffered_spriteram[offs + 2];
		colour = (x >> 12) & 0xf;
		if (pri && (colour >= priority_value))
			continue;
		if (!pri && !(colour >= priority_value))
			continue;

		flash = x & 0x800;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */

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


VIDEO_UPDATE( vaportra )
{
	vaportra_state *state = (vaportra_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);
	int pri = state->priority[0] & 0x03;

	flip_screen_set(screen->machine, !BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, 0, 0);
	deco16ic_pf34_update(state->deco16ic, 0, 0);

	/* Draw playfields */
	if (pri == 0)
	{
		deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	}
	else if (pri == 1)
	{
		deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	}
	else if (pri == 2)
	{
		deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	}
	else
	{
		deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, 0);
		deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	}

	draw_sprites(screen->machine, bitmap, cliprect, 1);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}
