// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Round" hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/relief.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(relief_state::get_playfield_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) & 0xff;
	int code = data1 & 0x7fff;
	int color = 0x20 + (data2 & 0x0f);
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
}


TILE_GET_INFO_MEMBER(relief_state::get_playfield2_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) >> 8;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config relief_state::s_mob_config =
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

VIDEO_START_MEMBER(relief_state,relief)
{
	/* MOs are 5bpp but with a 4-bit color granularity */
	m_gfxdecode->gfx(1)->set_granularity(16);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 relief_state::screen_update_relief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob().draw_async(cliprect);

	/* draw the playfield */
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_vad->playfield().draw(screen, bitmap, cliprect, 0, 0);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 0, 1);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob().bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					/* verified from the GALs on the real PCB; equations follow
					 *
					 *      --- PF/M is 1 if playfield has priority, or 0 if MOs have priority
					 *      PF/M = PFXS
					 *
					 *      --- CS0 is set to 1 if the MO is transparent
					 *      CS0=!MPX0*!MPX1*!MPX2*!MPX3
					 *
					 *      --- CS1 is 1 to select playfield pixels or 0 to select MO pixels
					 *      !CS1=MPX5*MPX6*MPX7*!CS0
					 *          +!MPX4*MPX5*MPX6*MPX7
					 *          +PFXS*!CS0
					 *          +!MPX4*PFXS
					 *
					 *      --- CRA10 is the 0x200 bit of the color RAM index; set for the top playfield only
					 *      CRA10:=CS1*PFXS
					 *
					 *      --- CRA9 is the 0x100 bit of the color RAM index; set for MOs only
					 *      !CA9:=CS1
					 *
					 *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
					 */
					int cs0 = 0;
					int cs1 = 1;

					/* compute the CS0 signal */
					cs0 = ((mo[x] & 0x0f) == 0);

					/* compute the CS1 signal */
					if ((!cs0 && (mo[x] & 0xe0) == 0xe0) ||
						((mo[x] & 0xf0) == 0xe0) ||
						(!pri[x] && !cs0) ||
						(!pri[x] && !(mo[x] & 0x10)))
						cs1 = 0;

					/* MO is displayed if cs1 == 0 */
					if (!cs1)
						pf[x] = mo[x];
				}
		}
	return 0;
}
