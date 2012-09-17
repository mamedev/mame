/***************************************************************************

    Atari Gauntlet hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "includes/gauntlet.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(gauntlet_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(gauntlet_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	int code = ((m_playfield_tile_bank * 0x1000) + (data & 0xfff)) ^ 0x800;
	int color = 0x10 + (m_playfield_color_bank * 8) + ((data >> 12) & 7);
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(gauntlet_state,gauntlet)
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

	UINT16 *codelookup;
	int i, size;

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(gauntlet_state::get_playfield_tile_info),this), TILEMAP_SCAN_COLS,  8,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);

	/* initialize the alphanumerics */
	m_alpha_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(gauntlet_state::get_alpha_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 64,32);
	m_alpha_tilemap->set_transparent_pen(0);

	/* modify the motion object code lookup table to account for the code XOR */
	codelookup = atarimo_get_code_lookup(0, &size);
	for (i = 0; i < size; i++)
		codelookup[i] ^= 0x800;

	/* set up the base color for the playfield */
	m_playfield_color_bank = m_vindctr2_screen_refresh ? 0 : 1;

	/* save states */
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_color_bank));
}



/*************************************
 *
 *  Horizontal scroll register
 *
 *************************************/

WRITE16_HANDLER( gauntlet_xscroll_w )
{
	gauntlet_state *state = space.machine().driver_data<gauntlet_state>();
	UINT16 oldxscroll = *state->m_xscroll;
	COMBINE_DATA(state->m_xscroll);

	/* if something changed, force a partial update */
	if (*state->m_xscroll != oldxscroll)
	{
		space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

		/* adjust the scrolls */
		state->m_playfield_tilemap->set_scrollx(0, *state->m_xscroll);
		atarimo_set_xscroll(0, *state->m_xscroll & 0x1ff);
	}
}



/*************************************
 *
 *  Vertical scroll/PF bank register
 *
 *************************************/

WRITE16_HANDLER( gauntlet_yscroll_w )
{
	gauntlet_state *state = space.machine().driver_data<gauntlet_state>();
	UINT16 oldyscroll = *state->m_yscroll;
	COMBINE_DATA(state->m_yscroll);

	/* if something changed, force a partial update */
	if (*state->m_yscroll != oldyscroll)
	{
		space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

		/* if the bank changed, mark all tiles dirty */
		if (state->m_playfield_tile_bank != (*state->m_yscroll & 3))
		{
			state->m_playfield_tile_bank = *state->m_yscroll & 3;
			state->m_playfield_tilemap->mark_all_dirty();
		}

		/* adjust the scrolls */
		state->m_playfield_tilemap->set_scrolly(0, *state->m_yscroll >> 7);
		atarimo_set_yscroll(0, (*state->m_yscroll >> 7) & 0x1ff);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( gauntlet )
{
	gauntlet_state *state = screen.machine().driver_data<gauntlet_state>();
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	state->m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* verified via schematics:

                        MO pen 1 clears PF color bit 0x80
                    */
					if ((mo[x] & 0x0f) == 1)
					{
						/* Vindicators Part II has extra logic here for the bases */
						if (!state->m_vindctr2_screen_refresh || (mo[x] & 0xf0) != 0)
							pf[x] ^= 0x80;
					}
					else
						pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	/* add the alpha on top */
	state->m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
