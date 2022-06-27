// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Phil Bennett
/***************************************************************************

    Atari Cyberstorm hardware

****************************************************************************/

#include "emu.h"
#include "cybstorm.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(cybstorm_state::get_alpha_tile_info)
{
	uint16_t data = m_vad->alpha().basemem_read(tile_index);
	int code = ((data & 0x400) ? (m_alpha_tile_bank * 0x400) : 0) + (data & 0x3ff);
	int color = (data >> 11) & 0x0f;
	int opaque = data & 0x8000;
	tileinfo.set(2, code, color, opaque ? TILEMAP_PIXEL_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(cybstorm_state::get_playfield_tile_info)
{
	uint16_t data1 = m_vad->playfield().basemem_read(tile_index);
	uint16_t data2 = m_vad->playfield().extmem_read(tile_index) & 0xff;
	int code = data1;
	int color = 8 + (data2 & 0x07);
	tileinfo.set(0, code, color, data2 & 0x80 ? TILE_FLIPX : 0);
	tileinfo.category = (data2 >> 4) & 3;
}


TILE_GET_INFO_MEMBER(cybstorm_state::get_playfield2_tile_info)
{
	uint16_t data1 = m_vad->playfield2().basemem_read(tile_index);
	uint16_t data2 = m_vad->playfield2().extmem_read(tile_index) >> 8;
	int code = data1;
	int color = data2 & 0x07;
	tileinfo.set(0, code, color, data2 & 0x80 ? TILE_FLIPX : 0);
	tileinfo.category = (data2 >> 4) & 3;
}

TILEMAP_MAPPER_MEMBER(cybstorm_state::playfield_scan)
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config cybstorm_state::s_mob_config =
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

	0x1000,             /* base palette entry */
	0x1000,             /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0x03ff,0,0,0 }}, /* mask for the link */
	{{ 0,0x7fff,0,0 }, { 0x3c00,0,0,0 }},   /* mask for the code index */
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

void cybstorm_state::video_start()
{
	/* motion objects have 256 color granularity */
	m_gfxdecode->gfx(1)->set_granularity(256);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t cybstorm_state::screen_update_cybstorm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_vad->mob().draw_async(cliprect);

	/* draw the playfield */
	bitmap_ind8 &priority_bitmap = screen.priority();
	priority_bitmap.fill(0, cliprect);
	m_vad->playfield().draw(screen, bitmap, cliprect, 0, 0x00);
	m_vad->playfield().draw(screen, bitmap, cliprect, 1, 0x01);
	m_vad->playfield().draw(screen, bitmap, cliprect, 2, 0x02);
	m_vad->playfield().draw(screen, bitmap, cliprect, 3, 0x03);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 0, 0x80);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 1, 0x84);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 2, 0x88);
	m_vad->playfield2().draw(screen, bitmap, cliprect, 3, 0x8c);

	/* draw and merge the MO */
	bitmap_ind16 &mobitmap = m_vad->mob().bitmap();
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			uint8_t const *const pri = &priority_bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x])
				{
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
					{
						if ((mopriority & 0x3) != 0)
							continue;
					}

					/* foreground playfield case */
					if (pri[x] & 0x80)
					{
						int const pfpriority = (pri[x] >> 2) & 3;

						if (mopriority > pfpriority)
							pf[x] = (mo[x] & atari_motion_objects_device::DATA_MASK) | 0x1000;
					}

					/* background playfield case */
					else
					{
						int const pfpriority = pri[x] & 3;

						/* playfield priority 3 always wins */
						if (pfpriority == 3)
							;

						/* otherwise, MOs get shown */
						else
							pf[x] = (mo[x] & atari_motion_objects_device::DATA_MASK) | 0x1000;
					}

					/* don't erase yet -- we need to make another pass later */
				}
		}

	/* now go back and process the upper bit of MO priority */
	for (const sparse_dirty_rect *rect = m_vad->mob().first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			int count = 0;
			for (int x = rect->left(); x <= rect->right() || (count && x < bitmap.width()); x++)
			{
				const uint16_t START_MARKER = ((4 << atari_motion_objects_device::PRIORITY_SHIFT) | 3);
				const uint16_t END_MARKER =   ((4 << atari_motion_objects_device::PRIORITY_SHIFT) | 7);
				const uint16_t MASK = ((4 << atari_motion_objects_device::PRIORITY_SHIFT) | 0x3f);

				// TODO: Stain pixels should not overlap sprites!
				if ((mo[x] & MASK) == START_MARKER)
				{
					count++;
				}

				if (count)
				{
					// Only applies to PF pixels
					if ((pf[x] & 0x1000) == 0)
					{
						pf[x] |= 0x2000;
					}
				}

				if ((mo[x] & MASK) == END_MARKER)
				{
					count--;
				}

				/* erase behind ourselves */
				mo[x] = 0;
			}
		}

	/* add the alpha on top */
	m_vad->alpha().draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
