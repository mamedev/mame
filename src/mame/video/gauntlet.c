/***************************************************************************

    Atari Gauntlet hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/gauntlet.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	gauntlet_state *state = (gauntlet_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha[tile_index];
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	gauntlet_state *state = (gauntlet_state *)machine->driver_data;
	UINT16 data = state->atarigen.playfield[tile_index];
	int code = ((state->playfield_tile_bank * 0x1000) + (data & 0xfff)) ^ 0x800;
	int color = 0x10 + (state->playfield_color_bank * 8) + ((data >> 12) & 7);
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( gauntlet )
{
	static const atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		1,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		1,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0,0,0,0x03ff }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0x7fff,0,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0x000f,0,0 }},	/* mask for the color */
		{{ 0,0xff80,0,0 }},	/* mask for the X position */
		{{ 0,0,0xff80,0 }},	/* mask for the Y position */
		{{ 0,0,0x0038,0 }},	/* mask for the width, in tiles*/
		{{ 0,0,0x0007,0 }},	/* mask for the height, in tiles */
		{{ 0,0,0x0040,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};

	gauntlet_state *state = (gauntlet_state *)machine->driver_data;
	UINT16 *codelookup;
	int i, size;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_cols,  8,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);

	/* modify the motion object code lookup table to account for the code XOR */
	codelookup = atarimo_get_code_lookup(0, &size);
	for (i = 0; i < size; i++)
		codelookup[i] ^= 0x800;

	/* set up the base color for the playfield */
	state->playfield_color_bank = state->vindctr2_screen_refresh ? 0 : 1;

	/* save states */
	state_save_register_global(machine, state->playfield_tile_bank);
	state_save_register_global(machine, state->playfield_color_bank);
}



/*************************************
 *
 *  Horizontal scroll register
 *
 *************************************/

WRITE16_HANDLER( gauntlet_xscroll_w )
{
	gauntlet_state *state = (gauntlet_state *)space->machine->driver_data;
	UINT16 oldxscroll = *state->atarigen.xscroll;
	COMBINE_DATA(state->atarigen.xscroll);

	/* if something changed, force a partial update */
	if (*state->atarigen.xscroll != oldxscroll)
	{
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

		/* adjust the scrolls */
		tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, *state->atarigen.xscroll);
		atarimo_set_xscroll(0, *state->atarigen.xscroll & 0x1ff);
	}
}



/*************************************
 *
 *  Vertical scroll/PF bank register
 *
 *************************************/

WRITE16_HANDLER( gauntlet_yscroll_w )
{
	gauntlet_state *state = (gauntlet_state *)space->machine->driver_data;
	UINT16 oldyscroll = *state->atarigen.yscroll;
	COMBINE_DATA(state->atarigen.yscroll);

	/* if something changed, force a partial update */
	if (*state->atarigen.yscroll != oldyscroll)
	{
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

		/* if the bank changed, mark all tiles dirty */
		if (state->playfield_tile_bank != (*state->atarigen.yscroll & 3))
		{
			state->playfield_tile_bank = *state->atarigen.yscroll & 3;
			tilemap_mark_all_tiles_dirty(state->atarigen.playfield_tilemap);
		}

		/* adjust the scrolls */
		tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, *state->atarigen.yscroll >> 7);
		atarimo_set_yscroll(0, (*state->atarigen.yscroll >> 7) & 0x1ff);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( gauntlet )
{
	gauntlet_state *state = (gauntlet_state *)screen->machine->driver_data;
	atarimo_rect_list rectlist;
	bitmap_t *mobitmap;
	int x, y, r;

	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 0, 0);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* verified via schematics:

                        MO pen 1 clears PF color bit 0x80
                    */
					if ((mo[x] & 0x0f) == 1)
					{
						/* Vindicators Part II has extra logic here for the bases */
						if (!state->vindctr2_screen_refresh || (mo[x] & 0xf0) != 0)
							pf[x] ^= 0x80;
					}
					else
						pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->atarigen.alpha_tilemap, 0, 0);
	return 0;
}
