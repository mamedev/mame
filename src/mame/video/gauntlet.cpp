// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(gauntlet_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = ((m_playfield_tile_bank * 0x1000) + (data & 0xfff)) ^ 0x800;
	int color = 0x10 + (m_playfield_color_bank * 8) + ((data >> 12) & 7);
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config gauntlet_state::s_mob_config =
{
	0,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	1,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	8,                  /* pixels per SLIP entry (0 for no-slip) */
	1,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x100,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0,0,0,0x03ff }}, /* mask for the link */
	{{ 0x7fff,0,0,0 }}, /* mask for the code index */
	{{ 0,0x000f,0,0 }}, /* mask for the color */
	{{ 0,0xff80,0,0 }}, /* mask for the X position */
	{{ 0,0,0xff80,0 }}, /* mask for the Y position */
	{{ 0,0,0x0038,0 }}, /* mask for the width, in tiles*/
	{{ 0,0,0x0007,0 }}, /* mask for the height, in tiles */
	{{ 0,0,0x0040,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0 }},            /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                   /* resulting value to indicate "special" */
};

VIDEO_START_MEMBER(gauntlet_state,gauntlet)
{
	/* modify the motion object code lookup table to account for the code XOR */
	std::vector<UINT16> &codelookup = m_mob->code_lookup();
	for (unsigned int i = 0; i < codelookup.size(); i++)
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

WRITE16_MEMBER( gauntlet_state::gauntlet_xscroll_w )
{
	UINT16 oldxscroll = *m_xscroll;
	COMBINE_DATA(m_xscroll);

	/* if something changed, force a partial update */
	if (*m_xscroll != oldxscroll)
	{
		m_screen->update_partial(m_screen->vpos());

		/* adjust the scrolls */
		m_playfield_tilemap->set_scrollx(0, *m_xscroll);
		m_mob->set_xscroll(*m_xscroll & 0x1ff);
	}
}



/*************************************
 *
 *  Vertical scroll/PF bank register
 *
 *************************************/

WRITE16_MEMBER( gauntlet_state::gauntlet_yscroll_w )
{
	UINT16 oldyscroll = *m_yscroll;
	COMBINE_DATA(m_yscroll);

	/* if something changed, force a partial update */
	if (*m_yscroll != oldyscroll)
	{
		m_screen->update_partial(m_screen->vpos());

		/* if the bank changed, mark all tiles dirty */
		if (m_playfield_tile_bank != (*m_yscroll & 3))
		{
			m_playfield_tile_bank = *m_yscroll & 3;
			m_playfield_tilemap->mark_all_dirty();
		}

		/* adjust the scrolls */
		m_playfield_tilemap->set_scrolly(0, *m_yscroll >> 7);
		m_mob->set_yscroll((*m_yscroll >> 7) & 0x1ff);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 gauntlet_state::screen_update_gauntlet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	/* draw the playfield */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw and merge the MO */
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					/* verified via schematics:

					    MO pen 1 clears PF color bit 0x80
					*/
					if ((mo[x] & 0x0f) == 1)
					{
						/* Vindicators Part II has extra logic here for the bases */
						if (!m_vindctr2_screen_refresh || (mo[x] & 0xf0) != 0)
							pf[x] ^= 0x80;
					}
					else
						pf[x] = mo[x];
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
