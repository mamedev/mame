// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari System 2 hardware

****************************************************************************/

#include "emu.h"
#include "video/atarimo.h"
#include "includes/slapstic.h"
#include "includes/atarisy2.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarisy2_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = data & 0x3ff;
	int color = (data >> 13) & 0x07;
	SET_TILE_INFO_MEMBER(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(atarisy2_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = m_playfield_tile_bank[(data >> 10) & 1] + (data & 0x3ff);
	int color = (data >> 11) & 7;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
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

VIDEO_START_MEMBER(atarisy2_state,atarisy2)
{
	/* initialize banked memory */
	m_alpha_tilemap->basemem().set(&m_vram[0x0000], 0x2000, 16, ENDIANNESS_NATIVE, 2);
	m_playfield_tilemap->basemem().set(&m_vram[0x2000], 0x2000, 16, ENDIANNESS_NATIVE, 2);
	m_mob->set_spriteram(&m_vram[0x0c00], 0x0400);

	/* reset the statics */
	m_yscroll_reset_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(atarisy2_state::reset_yscroll_callback),this));
	m_videobank = 0;

	/* save states */
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_videobank));
	save_item(NAME(m_vram));
}



/*************************************
 *
 *  Scroll/playfield bank write
 *
 *************************************/

WRITE16_MEMBER( atarisy2_state::xscroll_w )
{
	UINT16 oldscroll = *m_xscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		m_screen->update_partial(m_screen->vpos());

	/* update the playfield scrolling - hscroll is clocked on the following scanline */
	m_playfield_tilemap->set_scrollx(0, newscroll >> 6);

	/* update the playfield banking */
	if (m_playfield_tile_bank[0] != (newscroll & 0x0f) * 0x400)
	{
		m_playfield_tile_bank[0] = (newscroll & 0x0f) * 0x400;
		m_playfield_tilemap->mark_all_dirty();
	}

	/* update the data */
	*m_xscroll = newscroll;
}


TIMER_CALLBACK_MEMBER(atarisy2_state::reset_yscroll_callback)
{
	m_playfield_tilemap->set_scrolly(0, param);
}


WRITE16_MEMBER( atarisy2_state::yscroll_w )
{
	UINT16 oldscroll = *m_yscroll;
	UINT16 newscroll = oldscroll;
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
	if (m_playfield_tile_bank[1] != (newscroll & 0x0f) * 0x400)
	{
		m_playfield_tile_bank[1] = (newscroll & 0x0f) * 0x400;
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

PALETTE_DECODER_MEMBER( atarisy2_state, RRRRGGGGBBBBIIII )
{
	static const int ZB = 115, Z3 = 78, Z2 = 37, Z1 = 17, Z0 = 9;

	static const int intensity_table[16] =
	{
		0, ZB+Z0, ZB+Z1, ZB+Z1+Z0, ZB+Z2, ZB+Z2+Z0, ZB+Z2+Z1, ZB+Z2+Z1+Z0,
		ZB+Z3, ZB+Z3+Z0, ZB+Z3+Z1, ZB+Z3+Z1+Z0,ZB+ Z3+Z2, ZB+Z3+Z2+Z0, ZB+Z3+Z2+Z1, ZB+Z3+Z2+Z1+Z0
	};

	static const int color_table[16] =
	{
		0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xe, 0xf, 0xf
	};

	int i = intensity_table[raw & 15];
	UINT8 r = (color_table[(raw >> 12) & 15] * i) >> 4;
	UINT8 g = (color_table[(raw >> 8) & 15] * i) >> 4;
	UINT8 b = (color_table[(raw >> 4) & 15] * i) >> 4;

	return rgb_t(r, g, b);
}



/*************************************
 *
 *  Video RAM bank read/write handlers
 *
 *************************************/

READ16_MEMBER( atarisy2_state::slapstic_r )
{
	int result = m_slapstic_base[offset];
	m_slapstic->slapstic_tweak(space, offset);

	/* an extra tweak for the next opcode fetch */
	m_videobank = m_slapstic->slapstic_tweak(space, 0x1234) * 0x1000;
	return result;
}


WRITE16_MEMBER( atarisy2_state::slapstic_w )
{
	m_slapstic->slapstic_tweak(space, offset);

	/* an extra tweak for the next opcode fetch */
	m_videobank = m_slapstic->slapstic_tweak(space, 0x1234) * 0x1000;
}



/*************************************
 *
 *  Video RAM read/write handlers
 *
 *************************************/

READ16_MEMBER( atarisy2_state::videoram_r )
{
	int offs = offset | m_videobank;
	return m_vram[offs];
}


WRITE16_MEMBER( atarisy2_state::videoram_w )
{
	int offs = offset | m_videobank;

	/* alpharam? */
	if (offs < 0x0c00)
		m_alpha_tilemap->write(space, offs, data, mem_mask);

	/* spriteram? */
	else if (offs < 0x1000)
	{
		/* force an update if the link of object 0 is about to change */
		if (offs == 0x0c03)
			m_screen->update_partial(m_screen->vpos());
		COMBINE_DATA(&m_mob->spriteram()[offs - 0x0c00]);
	}

	/* playfieldram? */
	else if (offs >= 0x2000)
		m_playfield_tilemap->write(space, offs - 0x2000, data, mem_mask);

	/* generic case */
	else
	{
		COMBINE_DATA(&m_vram[offs]);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 atarisy2_state::screen_update_atarisy2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// reset priorities
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);

	/* draw the playfield */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 3, 3);

	/* draw and merge the MO */
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap.pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
				if (mo[x] != 0xffff)
				{
					int mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					/* high priority PF? */
					if ((mopriority + pri[x]) & 2)
					{
						/* only gets priority if PF pen is less than 8 */
						if (!(pf[x] & 0x08))
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
					}

					/* low priority */
					else
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
