/***************************************************************************

    Atari Skull & Crossbones hardware

****************************************************************************/

#include "emu.h"
#include "video/atarimo.h"
#include "includes/skullxbo.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(skullxbo_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = (data ^ 0x400) & 0x7ff;
	int color = (data >> 11) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(skullxbo_state::get_playfield_tile_info)
{
	UINT16 data1 = m_playfield[tile_index];
	UINT16 data2 = m_playfield_upper[tile_index] & 0xff;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO_MEMBER(1, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(skullxbo_state,skullxbo)
{
	static const atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		2,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x000,				/* base palette entry */
		0x200,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x00ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x000f }},	/* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0x0030,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0,					/* callback routine for special entries */
	};

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(skullxbo_state::get_playfield_tile_info),this), TILEMAP_SCAN_COLS,  16,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);

	/* initialize the alphanumerics */
	m_alpha_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(skullxbo_state::get_alpha_tile_info),this), TILEMAP_SCAN_ROWS,  16,8, 64,32);
	m_alpha_tilemap->set_transparent_pen(0);
}



/*************************************
 *
 *  Video data latch
 *
 *************************************/

WRITE16_HANDLER( skullxbo_xscroll_w )
{
	skullxbo_state *state = space.machine().driver_data<skullxbo_state>();

	/* combine data */
	UINT16 oldscroll = *state->m_xscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if something changed, force an update */
	if (oldscroll != newscroll)
		space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

	/* adjust the actual scrolls */
	state->m_playfield_tilemap->set_scrollx(0, 2 * (newscroll >> 7));
	atarimo_set_xscroll(0, 2 * (newscroll >> 7));

	/* update the data */
	*state->m_xscroll = newscroll;
}


WRITE16_HANDLER( skullxbo_yscroll_w )
{
	skullxbo_state *state = space.machine().driver_data<skullxbo_state>();

	/* combine data */
	int scanline = space.machine().primary_screen->vpos();
	UINT16 oldscroll = *state->m_yscroll;
	UINT16 newscroll = oldscroll;
	UINT16 effscroll;
	COMBINE_DATA(&newscroll);

	/* if something changed, force an update */
	if (oldscroll != newscroll)
		space.machine().primary_screen->update_partial(scanline);

	/* adjust the effective scroll for the current scanline */
	if (scanline > space.machine().primary_screen->visible_area().max_y)
		scanline = 0;
	effscroll = (newscroll >> 7) - scanline;

	/* adjust the actual scrolls */
	state->m_playfield_tilemap->set_scrolly(0, effscroll);
	atarimo_set_yscroll(0, effscroll & 0x1ff);

	/* update the data */
	*state->m_yscroll = newscroll;
}



/*************************************
 *
 *  Motion object bank handler
 *
 *************************************/

WRITE16_HANDLER( skullxbo_mobmsb_w )
{
	space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());
	atarimo_set_bank(0, (offset >> 9) & 1);
}



/*************************************
 *
 *  Playfield latch write handler
 *
 *************************************/

WRITE16_HANDLER( skullxbo_playfieldlatch_w )
{
	skullxbo_state *state = space.machine().driver_data<skullxbo_state>();
	state->set_playfield_latch(data);
}



/*************************************
 *
 *  Periodic playfield updater
 *
 *************************************/

void skullxbo_scanline_update(running_machine &machine, int scanline)
{
	skullxbo_state *state = machine.driver_data<skullxbo_state>();
	UINT16 *base = &state->m_alpha[(scanline / 8) * 64 + 42];
	int x;

	/* keep in range */
	if (base >= &state->m_alpha[0x7c0])
		return;

	/* special case: scanline 0 should re-latch the previous raw scroll */
	if (scanline == 0)
	{
		int newscroll = (*state->m_yscroll >> 7) & 0x1ff;
		state->m_playfield_tilemap->set_scrolly(0, newscroll);
		atarimo_set_yscroll(0, newscroll);
	}

	/* update the current parameters */
	for (x = 42; x < 64; x++)
	{
		UINT16 data = *base++;
		UINT16 command = data & 0x000f;

		/* only command I've ever seen */
		if (command == 0x0d)
		{
			/* a new vscroll latches the offset into a counter; we must adjust for this */
			int newscroll = ((data >> 7) - scanline) & 0x1ff;

			/* force a partial update with the previous scroll */
			if (scanline > 0)
				machine.primary_screen->update_partial(scanline - 1);

			/* update the new scroll */
			state->m_playfield_tilemap->set_scrolly(0, newscroll);
			atarimo_set_yscroll(0, newscroll);

			/* make sure we change this value so that writes to the scroll register */
			/* know whether or not they are a different scroll */
			*state->m_yscroll = data;
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 skullxbo_state::screen_update_skullxbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
					/* verified from the GALs on the real PCB; equations follow

                        --- O17 is an intermediate value
                        O17=PFPIX3*PFPAL2S*PFPAL3S

                        --- CRAM.A10 controls the high bit of the palette select; used for shadows
                        CRAM.A10=BA11*CRAMD
                            +!CRAMD*!LBPRI0*!LBPRI1*!O17*(LBPIX==1)*(ANPIX==0)
                            +!CRAMD*LBPRI0*!LBPRI1*(LBPIX==1)*(ANPIX==0)*!PFPAL3S
                            +!CRAMD*LBPRI1*(LBPIX==1)*(ANPIX==0)*!PFPAL2S*!PFPAL3S
                            +!CRAMD*!PFPIX3*(LBPIX==1)*(ANPIX==0)

                        --- SA and SB are the mux select lines:
                        ---     0 = motion objects
                        ---     1 = playfield
                        ---     2 = alpha
                        ---     3 = color RAM access from CPU
                        !SA=!CRAMD*(ANPIX!=0)
                            +!CRAMD*!LBPRI0*!LBPRI1*!O17*(LBPIX!=1)*(LBPIX!=0)
                            +!CRAMD*LBPRI0*!LBPRI1*(LBPIX!=1)*(LBPIX!=0)*!PFPAL3S
                            +!CRAMD*LBPRI1*(LBPIX!=1)*(LBPIX!=0)*!PFPAL2S*!PFPAL3S
                            +!CRAMD*!PFPIX3*(LBPIX!=1)*(LBPIX!=0)

                        !SB=!CRAMD*(ANPIX==0)
                            +!CRAMD*LBMISC*(LBPIX!=0)

                    */
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;
					int mopix = mo[x] & 0x1f;
					int pfcolor = (pf[x] >> 4) & 0x0f;
					int pfpix = pf[x] & 0x0f;
					int o17 = ((pf[x] & 0xc8) == 0xc8);

					/* implement the equations */
					if ((mopriority == 0 && !o17 && mopix >= 2) ||
						(mopriority == 1 && mopix >= 2 && !(pfcolor & 0x08)) ||
						((mopriority & 2) && mopix >= 2 && !(pfcolor & 0x0c)) ||
						(!(pfpix & 8) && mopix >= 2))
						pf[x] = mo[x] & ATARIMO_DATA_MASK;

					if ((mopriority == 0 && !o17 && mopix == 1) ||
						(mopriority == 1 && mopix == 1 && !(pfcolor & 0x08)) ||
						((mopriority & 2) && mopix == 1 && !(pfcolor & 0x0c)) ||
						(!(pfpix & 8) && mopix == 1))
						pf[x] |= 0x400;

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
