/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    Each game uses the MXC-06 chip to produce sprites.

    Sprite data:  The unknown bits seem to be unused.

    Byte 0:
        Bit 0 : Y co-ord hi bit
        Bit 1,2 : Sprite width (1x, 2x, 4x, 8x) - NOT YET EMULATED (todo)
        Bit 3,4 : Sprite height (1x, 2x, 4x, 8x)
        Bit 5  - X flip
        Bit 6  - Y flip
        Bit 7  - Only display Sprite if set
    Byte 1: Y-coords
    Byte 2:
        Bit 0,1,2,3: Hi bits of sprite number
        Bit 4,5,6,7: (Probably unused MSB's of sprite)
    Byte 3: Low bits of sprite number
    Byte 4:
        Bit 0 : X co-ords hi bit
        Bit 1,2: ??
        Bit 3: Sprite flash (sprite is displayed every other frame)
        Bit 4,5,6,7:  - Colour
    Byte 5: X-coords

**********************************************************************

  Palette data

    0x000 - character palettes (Sprites on Midnight R)
    0x200 - sprite palettes (Characters on Midnight R)
    0x400 - tiles 1
    0x600 - tiles 2

    Bad Dudes, Robocop, Heavy Barrel, Hippodrome - 24 bit rgb
    Sly Spy, Midnight Resistance - 12 bit rgb


Todo:
    Implement multi-width sprites (used by Birdtry).
    Implement sprite/tilemap orthogonality (not strictly needed as no
    games make deliberate use of it).

***************************************************************************/

#include "emu.h"
#include "includes/dec0.h"
#include "video/decbac06.h"


/******************************************************************************/

WRITE16_HANDLER( dec0_update_sprites_w )
{
	dec0_state *state = space->machine->driver_data<dec0_state>();
	memcpy(state->buffered_spriteram,state->spriteram,0x800);
}

/******************************************************************************/

static void update_24bitcol(running_machine *machine, int offset)
{
	int r,g,b;

	r = (machine->generic.paletteram.u16[offset] >> 0) & 0xff;
	g = (machine->generic.paletteram.u16[offset] >> 8) & 0xff;
	b = (machine->generic.paletteram2.u16[offset] >> 0) & 0xff;

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( dec0_paletteram_rg_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	update_24bitcol(space->machine, offset);
}

WRITE16_HANDLER( dec0_paletteram_b_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram2.u16[offset]);
	update_24bitcol(space->machine, offset);
}

/******************************************************************************/

static void draw_sprites(running_machine* machine, bitmap_t *bitmap,const rectangle *cliprect,int pri_mask,int pri_val)
{
	dec0_state *state = machine->driver_data<dec0_state>();
	UINT16 *spriteram = state->buffered_spriteram;
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		y = spriteram[offs];
		if ((y&0x8000) == 0) continue;

		x = spriteram[offs+2];
		colour = x >> 12;
		if ((colour & pri_mask) != pri_val) continue;

		flash=x&0x800;
		if (flash && (machine->primary_screen->frame_number() & 1)) continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = spriteram[offs+1] & 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (x>256) continue; /* Speedup */

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen_get(machine)) {
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

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

SCREEN_UPDATE( hbarrel )
{
	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
	draw_sprites(screen->machine,bitmap,cliprect,0x08,0x08);
	screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);

	/* HB always keeps pf2 on top of pf3, no need explicitly support priority register */

	draw_sprites(screen->machine,bitmap,cliprect,0x08,0x00);
	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE( baddudes )
{
	dec0_state *state = screen->machine->driver_data<dec0_state>();
	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	/* WARNING: inverted wrt Midnight Resistance */
	if ((state->pri & 0x01) == 0)
	{
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);

		if (state->pri & 2)
			screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */

		draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);

		if (state->pri & 4)
			screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */
	}
	else
	{
		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);

		if (state->pri & 2)
			screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */

		draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);

		if (state->pri & 4)
			screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */
	}

	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE( robocop )
{
	dec0_state *state = screen->machine->driver_data<dec0_state>();
	int trans;

	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	if (state->pri & 0x04)
		trans = 0x08;
	else
		trans = 0x00;

	if (state->pri & 0x01)
	{
		/* WARNING: inverted wrt Midnight Resistance */
		/* Robocop uses it only for the title screen, so this might be just */
		/* completely wrong. The top 8 bits of the register might mean */
		/* something (they are 0x80 in midres, 0x00 here) */
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_LAYER1|TILEMAP_DRAW_OPAQUE);

		if (state->pri & 0x02)
			draw_sprites(screen->machine,bitmap,cliprect,0x08,trans);

		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	}
	else
	{
		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);

		if (state->pri & 0x02)
			draw_sprites(screen->machine,bitmap,cliprect,0x08,trans);

		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	}

	if (state->pri & 0x02)
		draw_sprites(screen->machine,bitmap,cliprect,0x08,trans ^ 0x08);
	else
		draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);

	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE( birdtry )
{
	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	/* This game doesn't have the extra playfield chip on the game board, but
    the palette does show through. */
	bitmap_fill(bitmap,cliprect,screen->machine->pens[768]);
	screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);
	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE( hippodrm )
{
	dec0_state *state = screen->machine->driver_data<dec0_state>();
	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	if (state->pri & 0x01)
	{
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	}
	else
	{
		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	}

	draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);
	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE( slyspy )
{
	dec0_state *state = screen->machine->driver_data<dec0_state>();
	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
	screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);

	draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (state->pri&0x80)
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0);

	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE( midres )
{
	dec0_state *state = screen->machine->driver_data<dec0_state>();
	int trans;

	flip_screen_set(screen->machine, screen->machine->device<deco_bac06_device>("tilegen1")->get_flip_state());

	if (state->pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

	if (state->pri & 0x01)
	{
		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);

		if (state->pri & 0x02)
			draw_sprites(screen->machine,bitmap,cliprect,0x08,trans);

		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	}
	else
	{
		screen->machine->device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);

		if (state->pri & 0x02)
			draw_sprites(screen->machine,bitmap,cliprect,0x08,trans);

		screen->machine->device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	}

	if (state->pri & 0x02)
		draw_sprites(screen->machine,bitmap,cliprect,0x08,trans ^ 0x08);
	else
		draw_sprites(screen->machine,bitmap,cliprect,0x00,0x00);

	screen->machine->device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine,bitmap,cliprect,0);
	return 0;
}


WRITE16_HANDLER( dec0_priority_w )
{
	dec0_state *state = space->machine->driver_data<dec0_state>();
	COMBINE_DATA(&state->pri);
}

VIDEO_START( dec0_nodma )
{
	dec0_state *state = machine->driver_data<dec0_state>();
	state->buffered_spriteram = state->spriteram;
}

VIDEO_START( dec0 )
{
	dec0_state *state = machine->driver_data<dec0_state>();
	VIDEO_START_CALL(dec0_nodma);
	state->buffered_spriteram = auto_alloc_array(machine, UINT16, 0x800/2);
}

/******************************************************************************/
