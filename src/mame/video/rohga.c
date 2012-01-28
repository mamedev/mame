/***************************************************************************

    Rohga Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/rohga.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

WRITE16_HANDLER( rohga_buffer_spriteram16_w )
{
	// Spriteram seems to be triple buffered (no sprite lag on real pcb, but there
	// is on driver with only double buffering)
	rohga_state *state = space->machine().driver_data<rohga_state>();
	memcpy(state->m_spriteram, space->machine().generic.buffered_spriteram.u16, 0x800);
	memcpy(space->machine().generic.buffered_spriteram.u16, space->machine().generic.spriteram.u16, 0x800);
}

VIDEO_START( rohga )
{
	rohga_state *state = machine.driver_data<rohga_state>();
	state->m_spriteram = auto_alloc_array(machine, UINT16, 0x800/2);
	state->save_pointer(NAME(state->m_spriteram), 0x800/2);
}

/******************************************************************************/

static void rohga_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT16 *spriteptr, int is_schmeisr )
{
	int offs;

	for (offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0;
		sprite = spriteptr[offs + 1];
		if (!sprite)
			continue;

		x = spriteptr[offs + 2];

		/* Sprite/playfield priority */
		switch (x & 0x6000)
		{
		case 0x0000: pri = 0; break;
		case 0x4000: pri = 0xf0; break;
		case 0x6000: pri = 0xf0 | 0xcc; break;
		case 0x2000: pri = 0;//0xf0|0xcc; break; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
		}

		y = spriteptr[offs];
		flash = y & 0x1000;
		if (flash && (machine.primary_screen->frame_number() & 1))
			continue;

		// Sprite colour is different between Rohga (6bpp) and Schmeisr (4bpp plus wire mods on pcb)
		if (is_schmeisr)
		{
			colour = ((x >> 9) & 0xf) << 2;
			if (x & 0x8000)
				colour++;
		}
		else
		{
			colour = (x >> 9) & 0xf;
		}

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

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
			x = 304 - x;
			y = 240 - y;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = -16;
		}
		else
			mult = +16;

		while (multi >= 0)
		{
			pdrawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine.priority_bitmap,pri,0);

			multi--;
		}
	}
}

/******************************************************************************/

static void update_rohga( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int is_schmeisr )
{
	rohga_state *state = screen.machine().driver_data<rohga_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	/* Update playfields */
	flip_screen_set(screen.machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields */
	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(screen.machine().pens[768], cliprect);

	switch (priority & 3)
	{
	case 0:
		if (priority & 4)
		{
			// Draw as 1 8BPP layer
			deco16ic_tilemap_12_combine_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 3);
		}
		else
		{
			// Draw as 2 4BPP layers
			deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
			deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2);
		}
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 4);
		break;
	case 1:
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 2);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 4);
		break;
	case 2:
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 2);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 4);
		break;
	}

	rohga_draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram, is_schmeisr);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
}

SCREEN_UPDATE_IND16( rohga )
{
	update_rohga(screen, bitmap, cliprect, 0);
	return 0;
}

SCREEN_UPDATE_IND16( schmeisr )
{
	// The Schmeisr pcb has wire mods which seem to remap sprite palette indices.
	// Otherwise video update is the same as Rohga.
	update_rohga(screen, bitmap, cliprect, 1);
	return 0;
}

VIDEO_START(wizdfire)
{
	machine.device<decospr_device>("spritegen1")->alloc_sprite_bitmap();
	machine.device<decospr_device>("spritegen2")->alloc_sprite_bitmap();
}

// not amazingly efficient, called multiple times to pull a layer out of the sprite bitmaps, but keeps correct sprite<->sprite priorities
static void mixwizdfirelayer(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mode, int gfxregion)
{
	int y, x;
	const pen_t *paldata = machine.pens;
	bitmap_ind16* sprite_bitmap;
	int penbase;

	if (gfxregion==3)
	{
		sprite_bitmap = &machine.device<decospr_device>("spritegen1")->get_sprite_temp_bitmap();
		penbase = 0x400;
	}
	else
	{
		sprite_bitmap = &machine.device<decospr_device>("spritegen2")->get_sprite_temp_bitmap();
		penbase = 0x600;
	}

	UINT16* srcline;
	UINT32* dstline;


	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcline=&sprite_bitmap->pix16(y,0);
		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			UINT16 pix = srcline[x];
			switch (mode)
			{
			case 4:
				if ((pix & 0x600) != 0x600)
					continue;
				break;
			case 3:
				if ((pix & 0x600) != 0x400)
					continue;
				break;
			case 2:
				if ((pix & 0x400) != 0x400)
					continue;
				break;
			case 1:
			case 0:
			default:
				if ((pix & 0x400) != 0x000)
					continue;
				break;
			}

			if (pix&0xf)
			{
				UINT16 pen = pix&0x1ff;
				if (gfxregion==3)
				{
					dstline[x] = paldata[pen+penbase];
				}
				else
				{
					
					if (pen&0x100)
					{
						
						UINT32 base = dstline[x];
						pen &=0xff;
						dstline[x] = alpha_blend_r32(base, paldata[pen+penbase], 0x80);
						
					}
					else
					{
						dstline[x] = paldata[pen+penbase];
					}
				}
			}
		}
	}
}

SCREEN_UPDATE_RGB32( wizdfire )
{
	rohga_state *state = screen.machine().driver_data<rohga_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	/* draw sprite gfx to temp bitmaps */
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, screen.machine().generic.buffered_spriteram2.u16, 0x400, true);
	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, screen.machine().generic.buffered_spriteram.u16, 0x400, true);

	/* Update playfields */
	flip_screen_set(screen.machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, 0, 0);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(screen.machine().pens[512], cliprect);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	mixwizdfirelayer(screen.machine(), bitmap, cliprect, 4,3);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	mixwizdfirelayer(screen.machine(), bitmap, cliprect, 3,3);

	if ((priority & 0x1f) == 0x1f) /* Wizdfire has bit 0x40 always set, Dark Seal 2 doesn't?! */
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 0);
	else
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);

	mixwizdfirelayer(screen.machine(), bitmap, cliprect, 0,3);
	mixwizdfirelayer(screen.machine(), bitmap, cliprect, 2,4);
	mixwizdfirelayer(screen.machine(), bitmap, cliprect, 1,4);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_RGB32( nitrobal )
{
	rohga_state *state = screen.machine().driver_data<rohga_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	/* draw sprite gfx to temp bitmaps */
	screen.machine().device<decospr_device>("spritegen1")->set_alt_format(true);
	screen.machine().device<decospr_device>("spritegen2")->set_alt_format(true);
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, screen.machine().generic.buffered_spriteram2.u16, 0x400, false);
	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, screen.machine().generic.buffered_spriteram.u16, 0x400, false);

	/* Update playfields */
	flip_screen_set(screen.machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(screen.machine().pens[512], cliprect);
	screen.machine().priority_bitmap.fill(0);
	decocomn_clear_sprite_priority_bitmap(state->m_decocomn);

	/* pf3 and pf4 are combined into a single 8bpp bitmap */
	deco16ic_tilemap_12_combine_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 16);

	/* ToDo reimplement priorities + mixing / alpha, it was busted worse than this before anyway, so no big loss that we don't do it for now ;-) */
	screen.machine().device<decospr_device>("spritegen2")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x600, 0xff);
	screen.machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x400, 0x1ff);


	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
