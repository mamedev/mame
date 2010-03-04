/***************************************************************************

   Caveman Ninja Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/cninja.h"

/******************************************************************************/

VIDEO_START( stoneage )
{
	cninja_state *state = (cninja_state *)machine->driver_data;

	/* The bootleg has broken scroll registers */
	deco16ic_set_scrolldx(state->deco16ic, 3, 0, -10, -10);	/* pf4 16x16 tilemap */
	deco16ic_set_scrolldx(state->deco16ic, 1, 0, -10, -10);	/* pf2 16x16 tilemap */
	deco16ic_set_scrolldx(state->deco16ic, 0, 1, 2, 2);	/* pf1 8x8 tilemap */
}

/******************************************************************************/

static void cninja_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
	int offs;

	for (offs = 0x400 - 4; offs >=0 ; offs -= 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0;

		sprite = buffered_spriteram[offs + 1];
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

		y = buffered_spriteram[offs];
		flash = y & 0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

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

		if (flip_screen_get(machine))
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
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}

/* The bootleg sprites are in a different format! */
static void cninjabl_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
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
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;

		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

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

		if (flip_screen_get(machine))
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
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}


static void robocop2_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
	int offs;

	for (offs = 0x400 - 4; offs >=0 ; offs -= 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0;
		sprite = buffered_spriteram[offs + 1];
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

		y = buffered_spriteram[offs];
		flash = y & 0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;
		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		x = 304 - x;
		y = 240 - y;

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
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}

static void mutantf_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT16 *spriteptr, int gfxbank )
{
	int offs, end, inc;

	/*
        Alternate format from most 16 bit games - same as Captain America

        Word 0:
            0x8000: Y flip
            0x4000: X flip
            0x2000: Flash (Sprite toggles on/off every frame)
            0x1fff: Y value
        Word 1:
            0xffff: X value
        Word 2:
            0xf000: Block height
            0x0f00: Block width
            0x00e0: Unused?
            0x001f: Colour
        Word 3:
            0xffff: Sprite value
    */

	/* This may look strange, but the alpha-blended sprite chip definitely draws end to
        front, ie, reversed from normal pdrawgfx style. */
	if (gfxbank == 4)
	{
		offs = 0;
		end = 0x400;
		inc = 4;
	}
	else
	{
		offs = 0x3fc;
		end = -4;
		inc = -4;
	}

	while (offs != end)
	{
		int x, y, sprite, colour, fx, fy, w, h, sx, sy, x_mult, y_mult;
		int alpha = 0xff;

		sprite = spriteptr[offs + 3];
		if (!sprite)
		{
			offs += inc;
			continue;
		}

		sx = spriteptr[offs + 1];

		h = (spriteptr[offs + 2] & 0xf000) >> 12;
		w = (spriteptr[offs + 2] & 0x0f00) >>  8;

		sy = spriteptr[offs];
		if ((sy & 0x2000) && (video_screen_get_frame_number(machine->primary_screen) & 1))
		{
			offs += inc;
			continue;
		}

		colour = (spriteptr[offs + 2] >> 0) & 0x1f;

		if (gfxbank == 4)
		{ /* Seems to be always alpha'd */
			alpha = 0x80;
			colour &= 0xf;
		}

		fx = (spriteptr[offs + 0] & 0x4000);
		fy = (spriteptr[offs + 0] & 0x8000);

		if (flip_screen_get(machine))
		{
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;

			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx > 0x180) sx = -(0x200 - sx);
			if (sy > 0x180) sy = -(0x200 - sy);

			if (fx) { x_mult = -16; sx += 16 * w; } else { x_mult = 16; sx -= 16; }
			if (fy) { y_mult = -16; sy += 16 * h; } else { y_mult = 16; sy -= 16; }
		}
		else
		{
			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx & 0x100) sx = -(0x100 - (sx & 0xff));
			if (sy & 0x100) sy = -(0x100 - (sy & 0xff));
			sx = 304 - sx;
			sy = 240 - sy;
			if (sx >= 432) sx -= 512;
			if (sy >= 384) sy -= 512;
			if (fx) { x_mult = -16; sx += 16; } else { x_mult = 16; sx -= 16*w; }
			if (fy) { y_mult = -16; sy += 16; } else { y_mult = 16; sy -= 16*h; }
		}

		for (x = 0; x < w; x++)
		{
			for (y = 0; y < h; y++)
			{
				pdrawgfx_alpha(bitmap,cliprect,machine->gfx[gfxbank],
						sprite + y + h * x,
						colour,
						fx,fy,
						sx + x_mult * (w-x),sy + y_mult * (h-y),
						machine->priority_bitmap,0,
						0,alpha);
			}
		}
		offs += inc;
	}
}

/******************************************************************************/

VIDEO_UPDATE( cninja )
{
	cninja_state *state = (cninja_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);
	deco16ic_pf34_update(state->deco16ic, state->pf3_rowscroll, state->pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 512);
	deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 2);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 4);
	cninja_draw_sprites(screen->machine, bitmap, cliprect);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_UPDATE( cninjabl )
{
	cninja_state *state = (cninja_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);
	deco16ic_pf34_update(state->deco16ic, state->pf3_rowscroll, state->pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 512);
	deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 2);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 4);
	cninjabl_draw_sprites(screen->machine, bitmap, cliprect);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_UPDATE( edrandy )
{
	cninja_state *state = (cninja_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);
	deco16ic_pf34_update(state->deco16ic, state->pf3_rowscroll, state->pf4_rowscroll);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);
	deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 4);
	cninja_draw_sprites(screen->machine, bitmap, cliprect);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_UPDATE( robocop2 )
{
	cninja_state *state = (cninja_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);
	UINT16 priority = deco16ic_priority_r(state->deco16ic, 0, 0xffff);

	/* One of the tilemap chips can switch between 2 tilemaps at 4bpp, or 1 at 8bpp */
	if (priority & 4)
	{
		deco16ic_set_tilemap_colour_mask(state->deco16ic, 2, 0);
		deco16ic_set_tilemap_colour_mask(state->deco16ic, 3, 0);
		deco16ic_pf34_set_gfxbank(state->deco16ic, 0, 4);
	}
	else
	{
		deco16ic_set_tilemap_colour_mask(state->deco16ic, 2, 0xf);
		deco16ic_set_tilemap_colour_mask(state->deco16ic, 3, 0xf);
		deco16ic_pf34_set_gfxbank(state->deco16ic, 0, 2);
	}

	/* Update playfields */
	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);
	deco16ic_pf34_update(state->deco16ic, state->pf3_rowscroll, state->pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0x200);

	if ((priority & 4) == 0)
		deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);

	/* Switchable priority */
	switch (priority & 0x8)
	{
		case 8:
			deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 2);
			deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 4);
			break;
		default:
		case 0:
			deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 2);
			deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 4);
			break;
	}

	robocop2_draw_sprites(screen->machine, bitmap, cliprect);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_UPDATE( mutantf )
{
	cninja_state *state = (cninja_state *)screen->machine->driver_data;
	UINT16 flip = deco16ic_pf12_control_r(state->deco16ic, 0, 0xffff);
	UINT16 priority = deco16ic_priority_r(state->deco16ic, 0, 0xffff);

	flip_screen_set(screen->machine, BIT(flip, 7));
	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);
	deco16ic_pf34_update(state->deco16ic, state->pf3_rowscroll, state->pf4_rowscroll);

	/* Draw playfields */
	bitmap_fill(bitmap, cliprect, 0x400); /* Confirmed */

	/* There is no priority prom on this board, but there is a
    priority control word, the only values used in game appear
    to be 2, 6 & 7 though:

    Bit 0:  If set sprite chip 2 above sprite chip 1 else vice versa
    Bit 1:  Always set?
    Bit 2:  Almost always set  (Sometimes not set on screen transitions)

    The other bits may control alpha blend on the 2nd sprite chip, or
    layer order.
    */
	deco16ic_tilemap_4_draw(state->deco16ic, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	deco16ic_tilemap_3_draw(state->deco16ic, bitmap, cliprect, 0, 0);

	/* We need to abuse the priority bitmap a little by clearing it before
        drawing each sprite layer.  This is because there is no priority
        orthogonality between sprite layers, but the alpha layer must obey
        priority between sprites in each layer.  Ie, if we didn't do this,
        then when two alpha blended shadows overlapped then they would be 25%
        transparent against the background, rather than 50% */
	if (priority & 1)
	{
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
		mutantf_draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u16, 3);
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
		mutantf_draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram2.u16, 4);
	}
	else
	{
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
		mutantf_draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram2.u16, 4);
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
		mutantf_draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u16, 3);
	}
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 0);
	return 0;
}
