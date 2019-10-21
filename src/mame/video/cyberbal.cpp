// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Cyberball hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/cyberbal.h"


#define SCREEN_WIDTH        (42*16)



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(cyberbal_base_state::get_alpha_tile_info)
{
	uint16_t data = m_alpha->basemem_read(tile_index);
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO_MEMBER(2, code, color, (data >> 15) & 1);
}


TILE_GET_INFO_MEMBER(cyberbal_state::get_alpha2_tile_info)
{
	uint16_t data = m_alpha2->basemem_read(tile_index);
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO_MEMBER(2, code, color, (data >> 15) & 1);
}


TILE_GET_INFO_MEMBER(cyberbal_base_state::get_playfield_tile_info)
{
	uint16_t data = m_playfield->basemem_read(tile_index);
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}


TILE_GET_INFO_MEMBER(cyberbal_state::get_playfield2_tile_info)
{
	uint16_t data = m_playfield2->basemem_read(tile_index);
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}




/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config cyberbal_base_state::s_mob_config =
{
	1,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	1,                  /* does the neighbor bit affect the next object? */
	1024,               /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x600,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0,0,0x07f8,0 }}, /* mask for the link */
	{{ 0x7fff,0,0,0 }}, /* mask for the code index */
	{{ 0,0,0,0x000f }}, /* mask for the color */
	{{ 0,0,0,0xffc0 }}, /* mask for the X position */
	{{ 0,0xff80,0,0 }}, /* mask for the Y position */
	{{ 0 }},            /* mask for the width, in tiles*/
	{{ 0,0x000f,0,0 }}, /* mask for the height, in tiles */
	{{ 0x8000,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0 }},            /* mask for the priority */
	{{ 0,0,0,0x0010 }}, /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0,                  /* resulting value to indicate "special" */
};

void cyberbal_base_state::video_start()
{
	/* initialize the motion objects */
	m_mob->set_slipram(&m_current_slip[0]);

	/* save states */
	save_item(NAME(m_current_slip));
	save_item(NAME(m_playfield_palette_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
}


void cyberbal_state::video_start()
{
	m_playfield2->set_palette(*m_rpalette);
	m_alpha2->set_palette(*m_rpalette);

	cyberbal_base_state::video_start();

	m_mob2->set_slipram(&m_current_slip[1]);

	/* adjust the sprite positions */
	m_mob->set_xscroll(4);
	m_mob2->set_xscroll(4);
}


void cyberbal2p_state::video_start()
{
	cyberbal_base_state::video_start();

	/* adjust the sprite positions */
	m_mob->set_xscroll(5);
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void cyberbal_base_state::scanline_update_one(screen_device &screen, int scanline, int i, tilemap_t &curplayfield, tilemap_device &curalpha)
{
	/* keep in range */
	int offset = ((scanline - 8) / 8) * 64 + 47;
	if (offset < 0)
		offset += 0x800;
	else if (offset >= 0x800)
		return;

	/* update the current parameters */
	uint16_t word = curalpha.basemem_read(offset + 3);
	if (!(word & 1))
	{
		if (((word >> 1) & 7) != m_playfield_palette_bank[i])
		{
			if (scanline > 0)
				screen.update_partial(scanline - 1);
			m_playfield_palette_bank[i] = (word >> 1) & 7;
			curplayfield.set_palette_offset(m_playfield_palette_bank[i] << 8);
		}
	}
	word = curalpha.basemem_read(offset + 4);
	if (!(word & 1))
	{
		int newscroll = 2 * (((word >> 7) + 4) & 0x1ff);
		if (newscroll != m_playfield_xscroll[i])
		{
			if (scanline > 0)
				screen.update_partial(scanline - 1);
			curplayfield.set_scrollx(0, newscroll);
			m_playfield_xscroll[i] = newscroll;
		}
	}
	word = curalpha.basemem_read(offset + 5);
	if (!(word & 1))
	{
		/* a new vscroll latches the offset into a counter; we must adjust for this */
		int newscroll = ((word >> 7) - (scanline)) & 0x1ff;
		if (newscroll != m_playfield_yscroll[i])
		{
			if (scanline > 0)
				screen.update_partial(scanline - 1);
			curplayfield.set_scrolly(0, newscroll);
			m_playfield_yscroll[i] = newscroll;
		}
	}
	word = curalpha.basemem_read(offset + 7);
	if (!(word & 1))
	{
		if (m_current_slip[i] != word)
		{
			if (scanline > 0)
				screen.update_partial(scanline - 1);
			m_current_slip[i] = word;
		}
	}
	i++;
}

void cyberbal_state::scanline_update(screen_device &screen, int scanline)
{
	scanline_update_one(*m_lscreen, scanline, 0, *m_playfield, *m_alpha);
	scanline_update_one(*m_rscreen, scanline, 1, *m_playfield2, *m_alpha2);
}

void cyberbal2p_state::scanline_update(screen_device &screen, int scanline)
{
	scanline_update_one(*m_screen, scanline, 0, *m_playfield, *m_alpha);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t cyberbal_base_state::update_one_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, atari_motion_objects_device &curmob, tilemap_t &curplayfield, tilemap_t &curalpha)
{
	// start drawing
	curmob.draw_async(cliprect);

	/* draw the playfield */
	curplayfield.draw(screen, bitmap, cliprect, 0, 0);

	/* draw and merge the MO */
	bitmap_ind16 &mobitmap = curmob.bitmap();
	for (const sparse_dirty_rect *rect = curmob.first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t *mo = &mobitmap.pix16(y);
			uint16_t *pf = &bitmap.pix16(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					/* not verified: logic is all controlled in a PAL
					*/
					pf[x] = mo[x];
				}
		}

	/* add the alpha on top */
	curalpha.draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


uint32_t cyberbal_state::screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return update_one_screen(screen, bitmap, cliprect, *m_mob, *m_playfield, *m_alpha);
}

uint32_t cyberbal_state::screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return update_one_screen(screen, bitmap, cliprect, *m_mob2, *m_playfield2, *m_alpha2);
}

uint32_t cyberbal2p_state::screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return update_one_screen(screen, bitmap, cliprect, *m_mob, *m_playfield, *m_alpha);
}
