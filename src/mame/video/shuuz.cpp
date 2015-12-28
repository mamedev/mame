// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Shuuz hardware

****************************************************************************/

#include "emu.h"
#include "includes/shuuz.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(shuuz_state::get_playfield_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) >> 8;
	int code = data1 & 0x3fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config shuuz_state::s_mob_config =
{
	1,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	8,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x000,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0x00ff,0,0,0 }}, /* mask for the link */
	{{ 0,0x7fff,0,0 }}, /* mask for the code index */
	{{ 0,0,0x000f,0 }}, /* mask for the color */
	{{ 0,0,0xff80,0 }}, /* mask for the X position */
	{{ 0,0,0,0xff80 }}, /* mask for the Y position */
	{{ 0,0,0,0x0070 }}, /* mask for the width, in tiles*/
	{{ 0,0,0,0x0007 }}, /* mask for the height, in tiles */
	{{ 0,0x8000,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0 }},            /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                   /* resulting value to indicate "special" */
};


/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 shuuz_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob()->draw_async(cliprect);

	/* draw the playfield */
	m_vad->playfield()->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob()->bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob()->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					/* verified from the GALs on the real PCB; equations follow
					 *
					 *      --- O13 is 1 if (PFS7-4 == 0xf)
					 *      O13=PFS6*PFS7*(PFS5&PFS4)
					 *
					 *      --- PF/M is 1 if MOs have priority, or 0 if playfield has priority
					 *      MO/PF=!PFS7*!(LBD7&LBD6)*!M1*!O13
					 *         +!PFS7*!(LBD7&LBD6)*!M2*!O13
					 *         +!PFS7*!(LBD7&LBD6)*!M3*!O13
					 *         +PFS7*(LBD7&LBD6)*!M1*!O13
					 *         +PFS7*(LBD7&LBD6)*!M2*!O13
					 *         +PFS7*(LBD7&LBD6)*!M3*!O13
					 *
					 */
					int o13 = ((pf[x] & 0xf0) == 0xf0);
					int mopf = 0;

					/* compute the MO/PF signal */
					if ((!(pf[x] & 0x80) && ((mo[x] & 0xc0) != 0xc0) && ((mo[x] & 0x0e) != 0x00) && !o13) ||
						((pf[x] & 0x80) && ((mo[x] & 0xc0) == 0xc0) && ((mo[x] & 0x0e) != 0x00) && !o13))
						mopf = 1;

					/* if MO/PF is 1, we draw the MO */
					if (mopf)
						pf[x] = mo[x];
				}
		}
	return 0;
}
