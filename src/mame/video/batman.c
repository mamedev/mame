/***************************************************************************

    Atari Batman hardware

****************************************************************************/

#include "emu.h"
#include "video/atarimo.h"
#include "includes/batman.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(batman_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = ((data & 0x400) ? (m_alpha_tile_bank * 0x400) : 0) + (data & 0x3ff);
	int color = (data >> 11) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(batman_state::get_playfield_tile_info)
{
	UINT16 data1 = m_playfield[tile_index];
	UINT16 data2 = m_playfield_upper[tile_index] & 0xff;
	int code = data1 & 0x7fff;
	int color = 0x10 + (data2 & 0x0f);
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
	tileinfo.category = (data2 >> 4) & 3;
}


TILE_GET_INFO_MEMBER(batman_state::get_playfield2_tile_info)
{
	UINT16 data1 = m_playfield2[tile_index];
	UINT16 data2 = m_playfield_upper[tile_index] >> 8;
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

VIDEO_START_MEMBER(batman_state,batman)
{
	static const atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		1,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x03ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xff80,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x0007 }},	/* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0x0070,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		NULL				/* callback routine for special entries */
	};

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(batman_state::get_playfield_tile_info),this), TILEMAP_SCAN_COLS,  8,8, 64,64);

	/* initialize the second playfield */
	m_playfield2_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(batman_state::get_playfield2_tile_info),this), TILEMAP_SCAN_COLS,  8,8, 64,64);
	m_playfield2_tilemap->set_transparent_pen(0);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);

	/* initialize the alphanumerics */
	m_alpha_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(batman_state::get_alpha_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 64,32);
	m_alpha_tilemap->set_transparent_pen(0);
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void batman_state::scanline_update(screen_device &screen, int scanline)
{
	/* update the scanline parameters */
	if (scanline <= screen.visible_area().max_y && m_atarivc_state.rowscroll_enable)
	{
		UINT16 *base = &m_alpha[scanline / 8 * 64 + 48];
		int scan, i;

		for (scan = 0; scan < 8; scan++, scanline++)
			for (i = 0; i < 2; i++)
			{
				int data = *base++;
				switch (data & 15)
				{
					case 9:
						if (scanline > 0)
							screen.update_partial(scanline - 1);
						m_atarivc_state.mo_xscroll = (data >> 7) & 0x1ff;
						atarimo_set_xscroll(0, m_atarivc_state.mo_xscroll);
						break;

					case 10:
						if (scanline > 0)
							screen.update_partial(scanline - 1);
						m_atarivc_state.pf1_xscroll_raw = (data >> 7) & 0x1ff;
						atarivc_update_pf_xscrolls(this);
						m_playfield_tilemap->set_scrollx(0, m_atarivc_state.pf0_xscroll);
						m_playfield2_tilemap->set_scrollx(0, m_atarivc_state.pf1_xscroll);
						break;

					case 11:
						if (scanline > 0)
							screen.update_partial(scanline - 1);
						m_atarivc_state.pf0_xscroll_raw = (data >> 7) & 0x1ff;
						atarivc_update_pf_xscrolls(this);
						m_playfield_tilemap->set_scrollx(0, m_atarivc_state.pf0_xscroll);
						break;

					case 13:
						if (scanline > 0)
							screen.update_partial(scanline - 1);
						m_atarivc_state.mo_yscroll = (data >> 7) & 0x1ff;
						atarimo_set_yscroll(0, m_atarivc_state.mo_yscroll);
						break;

					case 14:
						if (scanline > 0)
							screen.update_partial(scanline - 1);
						m_atarivc_state.pf1_yscroll = (data >> 7) & 0x1ff;
						m_playfield2_tilemap->set_scrolly(0, m_atarivc_state.pf1_yscroll);
						break;

					case 15:
						if (scanline > 0)
							screen.update_partial(scanline - 1);
						m_atarivc_state.pf0_yscroll = (data >> 7) & 0x1ff;
						m_playfield_tilemap->set_scrolly(0, m_atarivc_state.pf0_yscroll);
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

UINT32 batman_state::screen_update_batman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind8 &priority_bitmap = machine().priority_bitmap;
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(bitmap, cliprect, 0, 0x00);
	m_playfield_tilemap->draw(bitmap, cliprect, 1, 0x01);
	m_playfield_tilemap->draw(bitmap, cliprect, 2, 0x02);
	m_playfield_tilemap->draw(bitmap, cliprect, 3, 0x03);
	m_playfield2_tilemap->draw(bitmap, cliprect, 0, 0x80);
	m_playfield2_tilemap->draw(bitmap, cliprect, 1, 0x84);
	m_playfield2_tilemap->draw(bitmap, cliprect, 2, 0x88);
	m_playfield2_tilemap->draw(bitmap, cliprect, 3, 0x8c);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
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
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

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
							pf[x] = mo[x] & ATARIMO_DATA_MASK;

						/* otherwise, we need to compare */
						else if (mopriority >= pfpriority)
							pf[x] = mo[x] & ATARIMO_DATA_MASK;
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
							pf[x] = mo[x] & ATARIMO_DATA_MASK;
					}

					/* don't erase yet -- we need to make another pass later */
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);

	/* now go back and process the upper bit of MO priority */
	rectlist.rect -= rectlist.numrects;
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

					/* upper bit of MO priority might mean palette kludges */
					if (mopriority & 4)
					{
						/* if bit 2 is set, start setting high palette bits */
						if (mo[x] & 2)
							atarimo_mark_high_palette(bitmap, pf, mo, x, y);
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
