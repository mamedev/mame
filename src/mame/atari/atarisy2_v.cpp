// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari System 2 hardware

****************************************************************************/

#include "emu.h"
#include "includes/atarisy2.h"


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarisy2_state::get_alpha_tile_info)
{
	uint16_t data = m_alpha_tilemap->basemem_read(tile_index);
	int code = data & 0x3ff;
	int color = (data >> 13) & 0x07;
	tileinfo.set(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(atarisy2_state::get_playfield_tile_info)
{
	uint16_t data = tile_index < 020000/2 ? m_playfieldt[tile_index] : m_playfieldb[tile_index & (020000/2 - 1)];
	int code = (m_playfield_tile_bank[(data >> 10) & 1] << 10) | (data & 0x3ff);
	int color = (data >> 11) & 7;
	tileinfo.set(0, code, color, 0);
	tileinfo.category = (~data >> 14) & 3;
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config atarisy2_state::s_mob_config =
{
	1,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	0,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x00,               /* base palette entry */
	0x40,               /* maximum number of colors */
	15,                 /* transparent pen index */

	{{ 0,0,0,0x07f8 }}, /* mask for the link */
	{{ 0,0x07ff,0,0 }, { 0x0007,0,0,0 }}, /* mask for the code index */
	{{ 0,0,0,0x3000 }}, /* mask for the color */
	{{ 0,0,0xffc0,0 }}, /* mask for the X position */
	{{ 0x7fc0,0,0,0 }}, /* mask for the Y position */
	{{ 0 }},            /* mask for the width, in tiles*/
	{{ 0,0x3800,0,0 }}, /* mask for the height, in tiles */
	{{ 0,0x4000,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0,0,0,0xc000 }}, /* mask for the priority */
	{{ 0,0x8000,0,0 }}, /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                  /* resulting value to indicate "special" */
};

void atarisy2_state::video_start()
{
	// reset the statics
	m_yscroll_reset_timer = timer_alloc(FUNC(atarisy2_state::reset_yscroll_callback), this);

	// save states
	save_item(NAME(m_playfield_tile_bank));
}


/*************************************
 *
 *  Scroll/playfield bank write
 *
 *************************************/

void atarisy2_state::xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t oldscroll = *m_xscroll;
	uint16_t newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	/* update the playfield scrolling - hscroll is clocked on the following scanline */
	m_playfield_tilemap->set_scrollx(0, newscroll >> 6);

	/* update the playfield banking */
	if (m_playfield_tile_bank[0] != (newscroll & 0x0f))
	{
		m_playfield_tile_bank[0] = (newscroll & 0x0f);
		m_playfield_tilemap->mark_all_dirty();
	}

	/* update the data */
	*m_xscroll = newscroll;
}


TIMER_CALLBACK_MEMBER(atarisy2_state::reset_yscroll_callback)
{
	m_playfield_tilemap->set_scrolly(0, param);
}


void atarisy2_state::yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t oldscroll = *m_yscroll;
	uint16_t newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	/* if bit 4 is zero, the scroll value is clocked in right away */
	if (!(newscroll & 0x10))
		m_playfield_tilemap->set_scrolly(0, (newscroll >> 6) - m_screen->vpos());
	else
		m_yscroll_reset_timer->adjust(m_screen->time_until_pos(0), newscroll >> 6);

	/* update the playfield banking */
	if (m_playfield_tile_bank[1] != (newscroll & 0x0f))
	{
		m_playfield_tile_bank[1] = (newscroll & 0x0f);
		m_playfield_tilemap->mark_all_dirty();
	}

	/* update the data */
	*m_yscroll = newscroll;
}


/*************************************
 *
 *  Palette RAM to RGB converter
 *
 *************************************/

rgb_t atarisy2_state::RRRRGGGGBBBBIIII(uint32_t raw)
{
	static constexpr int ZB = 115, Z3 = 78, Z2 = 37, Z1 = 17, Z0 = 9;

	static constexpr int intensity_table[16] =
	{
		0, ZB+Z0, ZB+Z1, ZB+Z1+Z0, ZB+Z2, ZB+Z2+Z0, ZB+Z2+Z1, ZB+Z2+Z1+Z0,
		ZB+Z3, ZB+Z3+Z0, ZB+Z3+Z1, ZB+Z3+Z1+Z0,ZB+ Z3+Z2, ZB+Z3+Z2+Z0, ZB+Z3+Z2+Z1, ZB+Z3+Z2+Z1+Z0
	};

	static constexpr int color_table[16] =
	{
		0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xe, 0xf, 0xf
	};

	int const i = intensity_table[raw & 15];
	uint8_t const r = (color_table[(raw >> 12) & 15] * i) >> 4;
	uint8_t const g = (color_table[(raw >> 8) & 15] * i) >> 4;
	uint8_t const b = (color_table[(raw >> 4) & 15] * i) >> 4;

	return rgb_t(r, g, b);
}


/*************************************
 *
 *  Video RAM read/write handlers
 *
 *************************************/

void atarisy2_state::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* force an update if the link of object 0 is about to change */
	if (offset == 0x0003)
		m_screen->update_partial(m_screen->vpos());
	COMBINE_DATA(&m_mob->spriteram()[offset]);
}

void atarisy2_state::playfieldt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_playfieldt + offset);
	m_playfield_tilemap->mark_tile_dirty(offset);
}

void atarisy2_state::playfieldb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_playfieldb + offset);
	m_playfield_tilemap->mark_tile_dirty(offset + 020000/2);
}


/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t atarisy2_state::screen_update_atarisy2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// reset priorities
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 3, 3);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			uint8_t const *const pri = &priority_bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// high priority PF?
					if ((mopriority + pri[x]) & 2)
					{
						// only gets priority if PF pen is less than 8
						if (!(pf[x] & 0x08))
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}

					// low priority
					else
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
				}
		}

	// add the alpha on top
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
