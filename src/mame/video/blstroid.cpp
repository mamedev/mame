// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Blasteroids hardware

****************************************************************************/

#include "emu.h"
#include "includes/blstroid.h"
#include "cpu/m68000/m68000.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(blstroid_state::get_playfield_tile_info)
{
	uint16_t data = m_playfield_tilemap->basemem_read(tile_index);
	int code = data & 0x1fff;
	int color = (data >> 13) & 0x07;
	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config blstroid_state::s_mob_config =
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

	0x000,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0,0,0x0ff8,0 }}, /* mask for the link */
	{{ 0,0x3fff,0,0 }}, /* mask for the code index */
	{{ 0,0,0,0x000f }}, /* mask for the color */
	{{ 0,0,0,0xffc0 }}, /* mask for the X position */
	{{ 0xff80,0,0,0 }}, /* mask for the Y position */
	{{ 0 }},            /* mask for the width, in tiles*/
	{{ 0x000f,0,0,0 }}, /* mask for the height, in tiles */
	{{ 0,0x8000,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0x4000,0,0 }}, /* mask for the vertical flip */
	{{ 0 }},            /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                  /* resulting value to indicate "special" */
};

void blstroid_state::video_start()
{
	m_irq_off_timer = timer_alloc(TIMER_IRQ_OFF);
	m_irq_on_timer = timer_alloc(TIMER_IRQ_ON);

	m_scanline_int_state = false;

	save_item(NAME(m_scanline_int_state));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void blstroid_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TIMER_IRQ_OFF:
			/* clear the interrupt */
			m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
			break;
		case TIMER_IRQ_ON:
			/* generate the interrupt */
			m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
			break;
		default:
			atarigen_state::device_timer(timer, id, param);
			break;
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(blstroid_state::scanline_update)
{
	int scanline = param;
	int offset = (scanline / 8) * 64 + 40;

	/* check for interrupts */
	if (offset < 0x1000)
		if (m_playfield_tilemap->basemem_read(offset) & 0x8000)
		{
			/* FIXME: - the only thing this IRQ does it tweak the starting MO link */
			/* unfortunately, it does it too early for the given MOs! */
			/* perhaps it is not actually hooked up on the real PCB... */
			return;

			/* set a timer to turn the interrupt on at HBLANK of the 7th scanline */
			/* and another to turn it off one scanline later */
			int width = m_screen->width();
			int vpos  = m_screen->vpos();
			attotime period_on  = m_screen->time_until_pos(vpos + 7, width * 0.9);
			attotime period_off = m_screen->time_until_pos(vpos + 8, width * 0.9);

			m_irq_on_timer->adjust(period_on);
			m_irq_off_timer->adjust(period_off);
		}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t blstroid_state::screen_update_blstroid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	/* draw the playfield */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw and merge the MO */
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					/* verified via schematics

					    priority address = HPPPMMMM
					*/
					int const priaddr = ((pf[x] & 8) << 4) | (pf[x] & 0x70) | ((mo[x] & 0xf0) >> 4);
					if (m_priorityram[priaddr] & 1)
						pf[x] = mo[x];
				}
		}
	return 0;
}
