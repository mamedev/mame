/***************************************************************************

    Rohga Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/rohga.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

WRITE16_MEMBER(rohga_state::rohga_buffer_spriteram16_w)
{
	// Spriteram seems to be triple buffered (no sprite lag on real pcb, but there
	// is on driver with only double buffering)
	m_spriteram->copy();
}

VIDEO_START( rohga )
{
	machine.device<decospr_device>("spritegen1")->set_col_callback(rohga_col_callback);
	machine.device<decospr_device>("spritegen1")->set_pri_callback(rohga_pri_callback);
}

VIDEO_START( schmeisr )
{
	VIDEO_START_CALL( rohga );
	// wire mods on pcb..
	machine.device<decospr_device>("spritegen1")->set_col_callback(schmeisr_col_callback);
}


UINT16 rohga_pri_callback(UINT16 x)
{
	switch (x & 0x6000)
	{
		case 0x0000: return 0;
		case 0x4000: return 0xf0;
		case 0x6000: return 0xf0 | 0xcc;
		case 0x2000: return 0;//0xf0|0xcc; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
	}

	return 0;
}

UINT16 schmeisr_col_callback(UINT16 x)
{
	UINT16 colour = ((x >> 9) & 0xf) << 2;
	if (x & 0x8000)
		colour++;

	return colour;
}

UINT16 rohga_col_callback(UINT16 x)
{
	return (x >> 9) & 0xf;
}



/******************************************************************************/

SCREEN_UPDATE_IND16( rohga )
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

	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400, true);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);

	return 0;
}



VIDEO_START(wizdfire)
{
	machine.device<decospr_device>("spritegen1")->alloc_sprite_bitmap();
	machine.device<decospr_device>("spritegen2")->alloc_sprite_bitmap();
}

// not amazingly efficient, called multiple times to pull a layer out of the sprite bitmaps, but keeps correct sprite<->sprite priorities
static void mixwizdfirelayer(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int gfxregion, UINT16 pri, UINT16 primask)
{
	int y, x;
	const pen_t *paldata = machine.pens;
	bitmap_ind16* sprite_bitmap;
	int penbase;

	sprite_bitmap = &machine.device<decospr_device>("spritegen2")->get_sprite_temp_bitmap();
	penbase = 0x600;

	UINT16* srcline;
	UINT32* dstline;


	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcline=&sprite_bitmap->pix16(y,0);
		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			UINT16 pix = srcline[x];

			if ((pix & primask) != pri)
				continue;

			if (pix&0xf)
			{
				UINT16 pen = pix&0x1ff;

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

SCREEN_UPDATE_RGB32( wizdfire )
{
	rohga_state *state = screen.machine().driver_data<rohga_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	/* draw sprite gfx to temp bitmaps */
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, state->m_spriteram2->buffer(), 0x400, true);
	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400, true);

	/* Update playfields */
	flip_screen_set(screen.machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, 0, 0);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(screen.machine().pens[512], cliprect);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	screen.machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0600, 0x0600, 0x400, 0x1ff);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0400, 0x0600, 0x400, 0x1ff);

	if ((priority & 0x1f) == 0x1f) /* Wizdfire has bit 0x40 always set, Dark Seal 2 doesn't?! */
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 0);
	else
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);

	screen.machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0400, 0x400, 0x1ff); // 0x000 and 0x200 of 0x600

	mixwizdfirelayer(screen.machine(), bitmap, cliprect, 4, 0x000, 0x000);

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
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, state->m_spriteram2->buffer(), 0x400, false);
	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400, false);

	/* Update playfields */
	flip_screen_set(screen.machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(screen.machine().pens[512], cliprect);
	screen.machine().priority_bitmap.fill(0);

	/* pf3 and pf4 are combined into a single 8bpp bitmap */
	deco16ic_tilemap_12_combine_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 16);

	/* ToDo reimplement priorities + mixing / alpha, it was busted worse than this before anyway, so no big loss that we don't do it for now ;-) */
	screen.machine().device<decospr_device>("spritegen2")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x600, 0xff);
	screen.machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x400, 0x1ff);


	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
