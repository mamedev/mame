/***************************************************************************

   Caveman Ninja Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/cninja.h"
#include "video/decospr.h"
#include "video/decocomn.h"

/******************************************************************************/

VIDEO_START_MEMBER(cninja_state,stoneage)
{
	/* The bootleg has broken scroll registers */
	deco16ic_set_scrolldx(m_deco_tilegen1, 3, 0, -10, -10); /* pf4 16x16 tilemap */
	deco16ic_set_scrolldx(m_deco_tilegen1, 1, 0, -10, -10); /* pf2 16x16 tilemap */
	deco16ic_set_scrolldx(m_deco_tilegen1, 0, 1, 2, 2); /* pf1 8x8 tilemap */
}

/******************************************************************************/


/* The bootleg sprites are in a different format! */
static void cninjabl_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	cninja_state *state = machine.driver_data<cninja_state>();
	UINT16 *buffered_spriteram = state->m_spriteram->buffer();
	int offs;
	int endoffs;

	// bootleg seems to use 0x180 as an end of list marker
	// find it first, so we can use normal list processing
	endoffs = 0x400 - 4;
	for (offs = 0; offs < 0x400 - 4 ; offs += 4)
	{
		int y = buffered_spriteram[offs + 1];

		if (y == 0x180)
		{
			endoffs = offs;
			offs = 0x400 - 4;
		}
	}

	for (offs = endoffs; offs >=0 ; offs -= 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0;

		sprite = buffered_spriteram[offs + 0]; // changed on bootleg!
		y = buffered_spriteram[offs + 1]; // changed on bootleg!

		if (!sprite)
			continue;

		x = buffered_spriteram[offs + 2];

		/* Sprite/playfield priority */
		switch (x & 0xc000)
		{
		case 0x0000: pri = 0; break;
		case 0x4000: pri = 0xf0; break;
		case 0x8000: pri = 0xf0 | 0xcc; break;
		case 0xc000: pri = 0xf0 | 0xcc; break; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
		}

		flash = y & 0x1000;
		if (flash && (machine.primary_screen->frame_number() & 1))
			continue;

		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;

		multi = (1 << ((y & 0x0600) >> 9)) - 1; /* 1x, 2x, 4x, 8x height */

		y -= multi * 16; // changed on bootleg!
		y += 4;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		//sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->flip_screen())
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

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

UINT32 cninja_state::screen_update_cninja(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);
	deco16ic_pf_update(m_deco_tilegen2, m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields */
	machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(512, cliprect);
	deco16ic_tilemap_2_draw(m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	deco16ic_tilemap_1_draw(m_deco_tilegen2, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 2);
	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 4);
	machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 cninja_state::screen_update_cninjabl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);

	/* force layers to be enabled */
	deco16ic_set_enable(m_deco_tilegen2, 0, 1 );
	deco16ic_set_enable(m_deco_tilegen2, 1, 1 );

	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);
	deco16ic_pf_update(m_deco_tilegen2, m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields */
	machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(512, cliprect);
	deco16ic_tilemap_2_draw(m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	deco16ic_tilemap_1_draw(m_deco_tilegen2, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 2);
	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 4);
	cninjabl_draw_sprites(machine(), bitmap, cliprect);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 cninja_state::screen_update_edrandy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);
	deco16ic_pf_update(m_deco_tilegen2, m_pf3_rowscroll, m_pf4_rowscroll);

	machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);
	deco16ic_tilemap_2_draw(m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	deco16ic_tilemap_1_draw(m_deco_tilegen2, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 4);
	machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 cninja_state::screen_update_robocop2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(m_decocomn, space, 0, 0xffff);

	/* One of the tilemap chips can switch between 2 tilemaps at 4bpp, or 1 at 8bpp */
	if (priority & 4)
	{
		deco16ic_set_tilemap_colour_mask(m_deco_tilegen1, 2, 0);
		deco16ic_set_tilemap_colour_mask(m_deco_tilegen1, 3, 0);
		deco16ic_pf12_set_gfxbank(m_deco_tilegen2, 0, 4);
	}
	else
	{
		deco16ic_set_tilemap_colour_mask(m_deco_tilegen1, 2, 0xf);
		deco16ic_set_tilemap_colour_mask(m_deco_tilegen1, 3, 0xf);
		deco16ic_pf12_set_gfxbank(m_deco_tilegen2, 0, 2);
	}

	/* Update playfields */
	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);
	deco16ic_pf_update(m_deco_tilegen2, m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields */
	machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0x200, cliprect);

	if ((priority & 4) == 0)
		deco16ic_tilemap_2_draw(m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);

	/* Switchable priority */
	switch (priority & 0x8)
	{
		case 8:
			deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 2);
			deco16ic_tilemap_1_draw(m_deco_tilegen2, bitmap, cliprect, 0, 4);
			break;
		default:
		case 0:
			deco16ic_tilemap_1_draw(m_deco_tilegen2, bitmap, cliprect, 0, 2);
			deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 4);
			break;
	}

	machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_START_MEMBER(cninja_state,mutantf)
{
	machine().device<decospr_device>("spritegen1")->alloc_sprite_bitmap();
	machine().device<decospr_device>("spritegen2")->alloc_sprite_bitmap();
}

UINT32 cninja_state::screen_update_mutantf(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(m_decocomn, space, 0, 0xffff);


	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);
	deco16ic_pf_update(m_deco_tilegen2, m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields */
	bitmap.fill(0x400, cliprect); /* Confirmed */

	machine().device<decospr_device>("spritegen1")->set_alt_format(true);
	machine().device<decospr_device>("spritegen2")->set_alt_format(true);
	machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, m_spriteram2->buffer(), 0x400, true);
	machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400, true);


	/* There is no priority prom on this board, but there is a
	priority control word, the only values used in game appear
	to be 2, 6 & 7 though:

	Bit 0:  If set sprite chip 2 above sprite chip 1 else vice versa
	Bit 1:  Always set?
	Bit 2:  Almost always set  (Sometimes not set on screen transitions)

	The other bits may control alpha blend on the 2nd sprite chip, or
	layer order.
	*/
	deco16ic_tilemap_2_draw(m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	deco16ic_tilemap_1_draw(m_deco_tilegen2, bitmap, cliprect, 0, 0);


	if (priority & 1)
	{
		machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x100, 0x1ff);
		machine().device<decospr_device>("spritegen2")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 1024+768, 0x0ff, 0x80); // fixed alpha of 0x80 for this layer?
	}
	else
	{
		machine().device<decospr_device>("spritegen2")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 1024+768, 0x0ff, 0x80);  // fixed alpha of 0x80 for this layer?
		machine().device<decospr_device>("spritegen1")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x100, 0x1ff);
	}
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
