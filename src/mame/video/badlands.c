/***************************************************************************

    Atari Bad Lands hardware

***************************************************************************/

#include "emu.h"
#include "video/atarimo.h"
#include "includes/badlands.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(badlands_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	int code = (data & 0x1fff) + ((data & 0x1000) ? (m_playfield_tile_bank << 12) : 0);
	int color = (data >> 13) & 0x07;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}



/*************************************
 *
 *  Generic video system start
 *
 *************************************/

VIDEO_START_MEMBER(badlands_state,badlands)
{
	static const atarimo_desc modesc =
	{
		1,                  /* index to which gfx system */
		1,                  /* number of motion object banks */
		0,                  /* are the entries linked? */
		1,                  /* are the entries split? */
		0,                  /* render in reverse order? */
		0,                  /* render in swapped X/Y order? */
		0,                  /* does the neighbor bit affect the next object? */
		0,                  /* pixels per SLIP entry (0 for no-slip) */
		0,                  /* pixel offset for SLIPs */
		0,                  /* maximum number of links to visit/scanline (0=all) */

		0x80,               /* base palette entry */
		0x80,               /* maximum number of colors */
		0,                  /* transparent pen index */

		{{ 0x1f }},         /* mask for the link */
		{{ 0 }},            /* mask for the graphics bank */
		{{ 0x0fff,0,0,0 }}, /* mask for the code index */
		{{ 0 }},            /* mask for the upper code index */
		{{ 0,0,0,0x0007 }}, /* mask for the color */
		{{ 0,0,0,0xff80 }}, /* mask for the X position */
		{{ 0,0xff80,0,0 }}, /* mask for the Y position */
		{{ 0 }},            /* mask for the width, in tiles*/
		{{ 0,0x000f,0,0 }}, /* mask for the height, in tiles */
		{{ 0 }},            /* mask for the horizontal flip */
		{{ 0 }},            /* mask for the vertical flip */
		{{ 0,0,0,0x0008 }}, /* mask for the priority */
		{{ 0 }},            /* mask for the neighbor */
		{{ 0 }},            /* mask for absolute coordinates */

		{{ 0 }},            /* mask for the special value */
		0,                  /* resulting value to indicate "special" */
		0                   /* callback routine for special entries */
	};

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(badlands_state::get_playfield_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 64,32);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);

	/* save states */
	save_item(NAME(m_playfield_tile_bank));
}



/*************************************
 *
 *  Playfield bank write handler
 *
 *************************************/

WRITE16_HANDLER( badlands_pf_bank_w )
{
	badlands_state *state = space.machine().driver_data<badlands_state>();
	if (ACCESSING_BITS_0_7)
		if (state->m_playfield_tile_bank != (data & 1))
		{
			space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());
			state->m_playfield_tile_bank = data & 1;
			state->m_playfield_tilemap->mark_all_dirty();
		}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 badlands_state::screen_update_badlands(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);

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
					/* not yet verified
					*/
					if ((mo[x] & ATARIMO_PRIORITY_MASK) || !(pf[x] & 8))
						pf[x] = mo[x] & ATARIMO_DATA_MASK;

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
