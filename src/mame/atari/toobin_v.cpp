// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Toobin' hardware

****************************************************************************/

#include "emu.h"
#include "toobin.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(toobin_state::get_alpha_tile_info)
{
	uint16_t data = m_alpha_tilemap->basemem_read(tile_index);
	int code = data & 0x3ff;
	int color = (data >> 12) & 0x0f;
	tileinfo.set(2, code, color, (data >> 10) & 1);
}


TILE_GET_INFO_MEMBER(toobin_state::get_playfield_tile_info)
{
	uint32_t data = m_playfield_tilemap->basemem_read(tile_index);
	int code = data & 0x3fff;
	int color = (data >> 16) & 0x0f;
	tileinfo.set(0, code, color, TILE_FLIPYX(data >> 14));
	tileinfo.category = (data >> 20) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config toobin_state::s_mob_config =
{
	1,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	1,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	1024,               /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x100,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0,0,0x00ff,0 }}, /* mask for the link */
	{{ 0,0x3fff,0,0 }}, /* mask for the code index */
	{{ 0,0,0,0x000f }}, /* mask for the color */
	{{ 0,0,0,0xffc0 }}, /* mask for the X position */
	{{ 0x7fc0,0,0,0 }}, /* mask for the Y position */
	{{ 0x0007,0,0,0 }}, /* mask for the width, in tiles*/
	{{ 0x0038,0,0,0 }}, /* mask for the height, in tiles */
	{{ 0,0x4000,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0x8000,0,0 }}, /* mask for the vertical flip */
	{{ 0 }},            /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0x8000,0,0,0 }}, /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                   /* resulting value to indicate "special" */
};

void toobin_state::video_start()
{
	/* allocate a playfield bitmap for rendering */
	m_screen->register_screen_bitmap(m_pfbitmap);

	save_item(NAME(m_brightness));
}



/*************************************
 *
 *  Palette RAM write handler
 *
 *************************************/

void toobin_state::paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	uint16_t newword = m_paletteram[offset];

	{
		int red =   (((newword >> 10) & 31) * 224) >> 5;
		int green = (((newword >>  5) & 31) * 224) >> 5;
		int blue =  (((newword      ) & 31) * 224) >> 5;

		if (red) red += 38;
		if (green) green += 38;
		if (blue) blue += 38;

		m_palette->set_pen_color(offset & 0x3ff, rgb_t(red, green, blue));
		if (!(newword & 0x8000))
			m_palette->set_pen_contrast(offset & 0x3ff, m_brightness);
		else
			m_palette->set_pen_contrast(offset & 0x3ff, 1.0);
	}
}


void toobin_state::intensity_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_brightness = (double)(~data & 0x1f) / 31.0;

		for (int i = 0; i < 0x400; i++)
			if (!BIT(m_paletteram[i], 15))
				m_palette->set_pen_contrast(i, m_brightness);
	}
}



/*************************************
 *
 *  X/Y scroll handlers
 *
 *************************************/

void toobin_state::xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t oldscroll = *m_xscroll;
	uint16_t newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	/* update the playfield scrolling - hscroll is clocked on the following scanline */
	m_playfield_tilemap->set_scrollx(0, newscroll >> 6);
	m_mob->set_xscroll(newscroll >> 6);

	/* update the data */
	*m_xscroll = newscroll;
}


void toobin_state::yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t oldscroll = *m_yscroll;
	uint16_t newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	/* if bit 4 is zero, the scroll value is clocked in right away */
	m_playfield_tilemap->set_scrolly(0, newscroll >> 6);
	m_mob->set_yscroll((newscroll >> 6) & 0x1ff);

	/* update the data */
	*m_yscroll = newscroll;
}



/*************************************
 *
 *  X/Y scroll handlers
 *
 *************************************/

void toobin_state::slip_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t oldslip = m_mob->slipram(offset);
	uint16_t newslip = oldslip;
	COMBINE_DATA(&newslip);

	/* if the SLIP is changing, force a partial update first */
	if (oldslip != newslip)
		m_screen->update_partial(m_screen->vpos());

	/* update the data */
	m_mob->slipram(offset) = newslip;
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t toobin_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	/* draw the playfield */
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(screen, m_pfbitmap, cliprect, 3, 3);

	/* draw and merge the MO */
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	pen_t const *const palette = m_palette->pens();
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *const dest = &bitmap.pix(y);
		uint16_t const *const mo = &mobitmap.pix(y);
		uint16_t const *const pf = &m_pfbitmap.pix(y);
		uint8_t const *const pri = &priority_bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint16_t pix = pf[x];
			if (mo[x] != 0xffff)
			{
				/* not verified: logic is all controlled in a PAL

				   factors: LBPRI1-0, LBPIX3, ANPIX1-0, PFPIX3, PFPRI1-0,
				            (~LBPIX3 & ~LBPIX2 & ~LBPIX1 & ~LBPIX0)
				*/

				/* only draw if not high priority PF */
				if (!pri[x] || !(pix & 8))
					pix = mo[x];
			}
			dest[x] = palette[pix];
		}
	}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
