// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Skull & Crossbones hardware

****************************************************************************/

#include "emu.h"
#include "includes/skullxbo.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(skullxbo_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = (data ^ 0x400) & 0x7ff;
	int color = (data >> 11) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(skullxbo_state::get_playfield_tile_info)
{
	UINT16 data1 = tilemap.basemem_read(tile_index);
	UINT16 data2 = tilemap.extmem_read(tile_index) & 0xff;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO_MEMBER(1, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config skullxbo_state::s_mob_config =
{
	0,                  /* index to which gfx system */
	2,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	8,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x000,              /* base palette entry */
	0x200,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0x00ff,0,0,0 }}, /* mask for the link */
	{{ 0,0x7fff,0,0 }}, /* mask for the code index */
	{{ 0,0,0x000f,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xff80 }}, /* mask for the Y position */
	{{ 0,0,0,0x0070 }}, /* mask for the width, in tiles*/
	{{ 0,0,0,0x000f }}, /* mask for the height, in tiles */
	{{ 0,0x8000,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0,0,0x0030,0 }}, /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                   /* resulting value to indicate "special" */
};

VIDEO_START_MEMBER(skullxbo_state,skullxbo)
{
}



/*************************************
 *
 *  Video data latch
 *
 *************************************/

WRITE16_MEMBER( skullxbo_state::skullxbo_xscroll_w )
{
	/* combine data */
	UINT16 oldscroll = *m_xscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if something changed, force an update */
	if (oldscroll != newscroll)
		m_screen->update_partial(m_screen->vpos());

	/* adjust the actual scrolls */
	m_playfield_tilemap->set_scrollx(0, 2 * (newscroll >> 7));
	m_mob->set_xscroll(2 * (newscroll >> 7));

	/* update the data */
	*m_xscroll = newscroll;
}


WRITE16_MEMBER( skullxbo_state::skullxbo_yscroll_w )
{
	/* combine data */
	int scanline = m_screen->vpos();
	UINT16 oldscroll = *m_yscroll;
	UINT16 newscroll = oldscroll;
	UINT16 effscroll;
	COMBINE_DATA(&newscroll);

	/* if something changed, force an update */
	if (oldscroll != newscroll)
		m_screen->update_partial(scanline);

	/* adjust the effective scroll for the current scanline */
	if (scanline > m_screen->visible_area().max_y)
		scanline = 0;
	effscroll = (newscroll >> 7) - scanline;

	/* adjust the actual scrolls */
	m_playfield_tilemap->set_scrolly(0, effscroll);
	m_mob->set_yscroll(effscroll & 0x1ff);

	/* update the data */
	*m_yscroll = newscroll;
}



/*************************************
 *
 *  Motion object bank handler
 *
 *************************************/

WRITE16_MEMBER( skullxbo_state::skullxbo_mobmsb_w )
{
	m_screen->update_partial(m_screen->vpos());
	m_mob->set_bank((offset >> 9) & 1);
}



/*************************************
 *
 *  Playfield latch write handler
 *
 *************************************/

WRITE16_MEMBER( skullxbo_state::playfield_latch_w )
{
	m_playfield_latch = data;
}

WRITE16_MEMBER(skullxbo_state::playfield_latched_w)
{
	m_playfield_tilemap->write(space, offset, data, mem_mask);
	if (m_playfield_latch != -1)
	{
		UINT16 oldval = m_playfield_tilemap->extmem_read(offset);
		UINT16 newval = (oldval & ~0x00ff) | (m_playfield_latch & 0x00ff);
		m_playfield_tilemap->extmem_write(offset, newval);
	}
}



/*************************************
 *
 *  Periodic playfield updater
 *
 *************************************/

void skullxbo_state::skullxbo_scanline_update(int scanline)
{
	int x;

	/* keep in range */
	int offset = (scanline / 8) * 64 + 42;
	if (offset >= 0x7c0)
		return;

	/* special case: scanline 0 should re-latch the previous raw scroll */
	if (scanline == 0)
	{
		int newscroll = (*m_yscroll >> 7) & 0x1ff;
		m_playfield_tilemap->set_scrolly(0, newscroll);
		m_mob->set_yscroll(newscroll);
	}

	/* update the current parameters */
	for (x = 42; x < 64; x++)
	{
		UINT16 data = m_alpha_tilemap->basemem_read(offset++);
		UINT16 command = data & 0x000f;

		/* only command I've ever seen */
		if (command == 0x0d)
		{
			/* a new vscroll latches the offset into a counter; we must adjust for this */
			int newscroll = ((data >> 7) - scanline) & 0x1ff;

			/* force a partial update with the previous scroll */
			if (scanline > 0)
				m_screen->update_partial(scanline - 1);

			/* update the new scroll */
			m_playfield_tilemap->set_scrolly(0, newscroll);
			m_mob->set_yscroll(newscroll);

			/* make sure we change this value so that writes to the scroll register */
			/* know whether or not they are a different scroll */
			*m_yscroll = data;
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
					int mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;
					int mopix = mo[x] & 0x1f;
					int pfcolor = (pf[x] >> 4) & 0x0f;
					int pfpix = pf[x] & 0x0f;
					int o17 = ((pf[x] & 0xc8) == 0xc8);

					/* implement the equations */
					if ((mopriority == 0 && !o17 && mopix >= 2) ||
						(mopriority == 1 && mopix >= 2 && !(pfcolor & 0x08)) ||
						((mopriority & 2) && mopix >= 2 && !(pfcolor & 0x0c)) ||
						(!(pfpix & 8) && mopix >= 2))
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;

					if ((mopriority == 0 && !o17 && mopix == 1) ||
						(mopriority == 1 && mopix == 1 && !(pfcolor & 0x08)) ||
						((mopriority & 2) && mopix == 1 && !(pfcolor & 0x0c)) ||
						(!(pfpix & 8) && mopix == 1))
						pf[x] |= 0x400;
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
