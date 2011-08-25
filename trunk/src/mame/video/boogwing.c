#include "emu.h"
#include "includes/boogwing.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram_base, int gfx_region )
{
	boogwing_state *state = machine.driver_data<boogwing_state>();
	int offs;
	int flipscreen = !flip_screen_get(machine);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	for (offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0, spri = 0;
		int alpha = 0xff;

		sprite = spriteram_base[offs + 1];
		if (!sprite)
			continue;

		y = spriteram_base[offs];
		flash = y & 0x1000;
		if (flash && (machine.primary_screen->frame_number() & 1))
			continue;

		x = spriteram_base[offs + 2];
		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		// Todo:  This should be verified from the prom
		if (gfx_region == 4)
		{
			// Sprite 2 priority vs sprite 1
			if ((spriteram_base[offs + 2] & 0xc000) == 0xc000)
				spri = 4;
			else if ((spriteram_base[offs + 2] & 0xc000))
				spri = 16;
			else
				spri = 64;

			// Transparency
			if (spriteram_base[offs + 2] & 0x2000)
				alpha = 0x80;

			if (priority == 0x2)
			{
				// Additional sprite alpha in this mode
				if (spriteram_base[offs + 2] & 0x8000)
					alpha = 0x80;

				// Sprite vs playfield
				if ((spriteram_base[offs + 2] & 0xc000) == 0xc000)
					pri = 4;
				else if ((spriteram_base[offs + 2] & 0xc000) == 0x8000)
					pri = 16;
				else
					pri = 64;
			}
			else
			{
				if ((spriteram_base[offs + 2] & 0x8000) == 0x8000)
					pri = 16;
				else
					pri = 64;
			}
		}
		else
		{
			// Sprite 1 priority vs sprite 2
			if (spriteram_base[offs + 2] & 0x8000)		// todo - check only in pri mode 2??
				spri = 8;
			else
				spri = 32;

			// Sprite vs playfield
			if (priority == 0x1)
			{
				if ((spriteram_base[offs + 2] & 0xc000))
					pri = 16;
				else
					pri = 64;
			}
			else
			{
				if ((spriteram_base[offs + 2] & 0xc000) == 0xc000)
					pri = 4;
				else if ((spriteram_base[offs + 2] & 0xc000) == 0x8000)
					pri = 16;
				else
					pri = 64;
			}
		}

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
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
			decocomn_pdrawgfx(
					state->m_decocomn,
					bitmap, cliprect, machine.gfx[gfx_region],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					0, pri, spri, 0, alpha);

			multi--;
		}
	}
}

SCREEN_UPDATE( boogwing )
{
	boogwing_state *state = screen->machine().driver_data<boogwing_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	flip_screen_set(screen->machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields */
	decocomn_clear_sprite_priority_bitmap(state->m_decocomn);
	bitmap_fill(bitmap, cliprect, screen->machine().pens[0x400]); /* pen not confirmed */
	bitmap_fill(screen->machine().priority_bitmap, NULL, 0);

	// bit&0x8 is definitely some kind of palette effect
	// bit&0x4 combines playfields
	if ((priority & 0x7) == 0x5)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_12_combine_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x1 || (priority & 0x7) == 0x2)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 8);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x3)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 8);

		// This mode uses playfield 3 to shadow sprites & playfield 2 (instead of
		// regular alpha-blending, the destination is inverted).  Not yet implemented.
		// deco16ic_tilemap_3_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 32);
	}
	else
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 8);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 32);
	}

	draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram.u16, 3);
	draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram2.u16, 4);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
