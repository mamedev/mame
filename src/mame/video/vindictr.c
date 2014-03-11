// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Vindicators hardware

****************************************************************************/

#include "emu.h"
#include "includes/vindictr.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(vindictr_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(vindictr_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = (m_playfield_tile_bank * 0x1000) + (data & 0xfff);
	int color = 0x10 + 2 * ((data >> 12) & 7);
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config vindictr_state::s_mob_config =
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

	{{ 0,0,0,0x03ff }}, /* mask for the link */
	{{ 0x7fff,0,0,0 }}, /* mask for the code index */
	{{ 0,0x000f,0,0 }}, /* mask for the color */
	{{ 0,0xff80,0,0 }}, /* mask for the X position */
	{{ 0,0,0xff80,0 }}, /* mask for the Y position */
	{{ 0,0,0x0038,0 }}, /* mask for the width, in tiles*/
	{{ 0,0,0x0007,0 }}, /* mask for the height, in tiles */
	{{ 0,0,0x0040,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0,0x0070,0,0 }}, /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                  /* resulting value to indicate "special" */
};

VIDEO_START_MEMBER(vindictr_state,vindictr)
{
	/* save states */
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
}



/*************************************
 *
 *  Palette RAM control
 *
 *************************************/

WRITE16_MEMBER( vindictr_state::vindictr_paletteram_w )
{
	static const int ztable[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };
	int c;

	/* first blend the data */
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	data = m_generic_paletteram_16[offset];

	/* now generate colors at all 16 intensities */
	for (c = 0; c < 8; c++)
	{
		int i = ztable[((data >> 12) + (c * 2)) & 15];
		int r = ((data >> 8) & 15) * i;
		int g = ((data >> 4) & 15) * i;
		int b = ((data >> 0) & 15) * i;

		m_palette->set_pen_color(offset + c*2048,rgb_t(r,g,b));
	}
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void vindictr_state::scanline_update(screen_device &screen, int scanline)
{
	int x;

	/* keep in range */
	int offset = ((scanline - 8) / 8) * 64 + 42;
	if (offset < 0)
		offset += 0x7c0;
	else if (offset >= 0x7c0)
		return;

	/* update the current parameters */
	for (x = 42; x < 64; x++)
	{
		UINT16 data = m_alpha_tilemap->basemem_read(offset++);

		switch ((data >> 9) & 7)
		{
			case 2:     /* /PFB */
				if (m_playfield_tile_bank != (data & 7))
				{
					screen.update_partial(scanline - 1);
					m_playfield_tile_bank = data & 7;
					m_playfield_tilemap->mark_all_dirty();
				}
				break;

			case 3:     /* /PFHSLD */
				if (m_playfield_xscroll != (data & 0x1ff))
				{
					screen.update_partial(scanline - 1);
					m_playfield_tilemap->set_scrollx(0, data);
					m_playfield_xscroll = data & 0x1ff;
				}
				break;

			case 4:     /* /MOHS */
				if (m_mob->xscroll() != (data & 0x1ff))
				{
					screen.update_partial(scanline - 1);
					m_mob->set_xscroll(data & 0x1ff);
				}
				break;

			case 5:     /* /PFSPC */
				break;

			case 6:     /* /VIRQ */
				scanline_int_gen(m_maincpu);
				break;

			case 7:     /* /PFVS */
			{
				/* a new vscroll latches the offset into a counter; we must adjust for this */
				int offset = scanline;
				const rectangle &visible_area = screen.visible_area();
				if (offset > visible_area.max_y)
					offset -= visible_area.max_y + 1;

				if (m_playfield_yscroll != ((data - offset) & 0x1ff))
				{
					screen.update_partial(scanline - 1);
					m_playfield_tilemap->set_scrolly(0, data - offset);
					m_mob->set_yscroll((data - offset) & 0x1ff);
				}
				break;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 vindictr_state::screen_update_vindictr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	/* draw the playfield */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					/* partially verified via schematics (there are a lot of PALs involved!):

					    SHADE = PAL(MPR1-0, LB7-0, PFX6-5, PFX3-2, PF/M)

					    if (SHADE)
					        CRA |= 0x100

					    MOG3-1 = ~MAT3-1 if MAT6==1 and MSD3==1
					*/
					int mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
						continue;

					/* MO pen 1 doesn't draw, but it sets the SHADE flag and bumps the palette offset */
					if ((mo[x] & 0x0f) == 1)
					{
						if ((mo[x] & 0xf0) != 0)
							pf[x] |= 0x100;
					}
					else
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;

					/* don't erase yet -- we need to make another pass later */
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* now go back and process the upper bit of MO priority */
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != NULL; rect = rect->next())
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
							m_mob->apply_stain(bitmap, pf, mo, x, y);

						/* if the upper bit of pen data is set, we adjust the final intensity */
						if (mo[x] & 8)
							pf[x] |= (~mo[x] & 0xe0) << 6;
					}
				}
		}
	return 0;
}
