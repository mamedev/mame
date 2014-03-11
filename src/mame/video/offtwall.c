// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Round" hardware

****************************************************************************/

#include "emu.h"
#include "includes/offtwall.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(offtwall_state::get_playfield_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) >> 8;
	int code = data1 & 0x7fff;
	int color = 0x10 + (data2 & 0x0f);
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config offtwall_state::s_mob_config =
{
	0,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	8,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x100,              /* base palette entry */
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


VIDEO_START_MEMBER(offtwall_state,offtwall)
{
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 offtwall_state::screen_update_offtwall(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob()->draw_async(cliprect);

	/* draw the playfield */
	m_vad->playfield()->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob()->bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob()->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					/* not yet verified
					*/
					pf[x] = mo[x];
				}
		}
	return 0;
}
