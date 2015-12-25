// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Batman hardware

****************************************************************************/

#include "emu.h"
#include "includes/batman.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(batman_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = ((data & 0x400) ? (m_alpha_tile_bank * 0x400) : 0) + (data & 0x3ff);
	int color = (data >> 11) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(batman_state::get_playfield_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) & 0xff;
	int code = data1 & 0x7fff;
	int color = 0x10 + (data2 & 0x0f);
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
	tileinfo.category = (data2 >> 4) & 3;
}


TILE_GET_INFO_MEMBER(batman_state::get_playfield2_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) >> 8;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
	tileinfo.category = (data2 >> 4) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config batman_state::s_mob_config =
{
	1,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	1,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	8,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x100,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0x03ff,0,0,0 }}, /* mask for the link */
	{{ 0,0x7fff,0,0 }}, /* mask for the code index */
	{{ 0,0,0x000f,0 }}, /* mask for the color */
	{{ 0,0,0xff80,0 }}, /* mask for the X position */
	{{ 0,0,0,0xff80 }}, /* mask for the Y position */
	{{ 0,0,0,0x0070 }}, /* mask for the width, in tiles*/
	{{ 0,0,0,0x0007 }}, /* mask for the height, in tiles */
	{{ 0,0x8000,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0,0,0x0070,0 }}, /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0,                  /* resulting value to indicate "special" */
};

VIDEO_START_MEMBER(batman_state,batman)
{
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 batman_state::screen_update_batman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_vad->mob()->draw_async(cliprect);

	/* draw the playfield */
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_vad->playfield()->draw(screen, bitmap, cliprect, 0, 0x00);
	m_vad->playfield()->draw(screen, bitmap, cliprect, 1, 0x01);
	m_vad->playfield()->draw(screen, bitmap, cliprect, 2, 0x02);
	m_vad->playfield()->draw(screen, bitmap, cliprect, 3, 0x03);
	m_vad->playfield2()->draw(screen, bitmap, cliprect, 0, 0x80);
	m_vad->playfield2()->draw(screen, bitmap, cliprect, 1, 0x84);
	m_vad->playfield2()->draw(screen, bitmap, cliprect, 2, 0x88);
	m_vad->playfield2()->draw(screen, bitmap, cliprect, 3, 0x8c);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_vad->mob()->bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob()->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					/* verified on real hardware:

					    for all MO colors, MO priority 0:
					        obscured by low fg playfield pens priority 1-3
					        obscured by high fg playfield pens priority 3 only
					        obscured by bg playfield priority 3 only

					    for all MO colors, MO priority 1:
					        obscured by low fg playfield pens priority 2-3
					        obscured by high fg playfield pens priority 3 only
					        obscured by bg playfield priority 3 only

					    for all MO colors, MO priority 2-3:
					        obscured by low fg playfield pens priority 3 only
					        obscured by high fg playfield pens priority 3 only
					        obscured by bg playfield priority 3 only
					*/
					int mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
						continue;

					/* foreground playfield case */
					if (pri[x] & 0x80)
					{
						int pfpriority = (pri[x] >> 2) & 3;

						/* playfield priority 3 always wins */
						if (pfpriority == 3)
							;

						/* priority is consistent for upper pens in playfield */
						else if (pf[x] & 0x08)
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;

						/* otherwise, we need to compare */
						else if (mopriority >= pfpriority)
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}

					/* background playfield case */
					else
					{
						int pfpriority = pri[x] & 3;

						/* playfield priority 3 always wins */
						if (pfpriority == 3)
							;

						/* otherwise, MOs get shown */
						else
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}

					/* don't erase yet -- we need to make another pass later */
				}
		}

	/* add the alpha on top */
	m_vad->alpha()->draw(screen, bitmap, cliprect, 0, 0);

	/* now go back and process the upper bit of MO priority */
	for (const sparse_dirty_rect *rect = m_vad->mob()->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					int mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					/* upper bit of MO priority might mean palette kludges */
					if (mopriority & 4)
					{
						/* if bit 2 is set, start setting high palette bits */
						if (mo[x] & 2)
							m_vad->mob()->apply_stain(bitmap, pf, mo, x, y);
					}
				}
		}
	return 0;
}
