// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*
    Electronic Devices video system

    used by
    stlforce.cpp
    mwarr.cpp

    TODO:
    Check if the sprites are an obvious clone of anything else and split out if neccessary
    Steel Force doesn't use the variable height sprites, does the hardware support them?
    Verify offsets / visible areas / overscan etc. especially text layer as Steel Force ending does not quite fit on screen
*/


#include "emu.h"
#include "edevices.h"

DEFINE_DEVICE_TYPE(EDEVICES_VID, edevices_device, "edevices_vid", "Electronic Devices Video (Mighty Warriors)")
DEFINE_DEVICE_TYPE(EDEVICES_SFORCE_VID, edevices_sforce_device, "edevices_sforce_vid", "Electronic Devices Video (Steel Forces)")


edevices_device::edevices_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_bg_videoram(*this, finder_base::DUMMY_TAG),
	m_mlow_videoram(*this, finder_base::DUMMY_TAG),
	m_mhigh_videoram(*this, finder_base::DUMMY_TAG),
	m_tx_videoram(*this, finder_base::DUMMY_TAG),
	m_bg_scrollram(*this, finder_base::DUMMY_TAG),
	m_mlow_scrollram(*this, finder_base::DUMMY_TAG),
	m_mhigh_scrollram(*this, finder_base::DUMMY_TAG),
	m_vidattrram(*this, finder_base::DUMMY_TAG),
	m_spriteram(*this, finder_base::DUMMY_TAG),
	m_gfxdecode(*this, finder_base::DUMMY_TAG),
	m_palette(*this, finder_base::DUMMY_TAG),
	m_spritexoffs(0) // might come from the clock, twinbrat has different video timings and resolution
{
}


edevices_device::edevices_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	edevices_device(mconfig, EDEVICES_VID, tag, owner, clock)
{
}

edevices_sforce_device::edevices_sforce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	edevices_device(mconfig, EDEVICES_SFORCE_VID, tag, owner, clock)
{
}


void edevices_device::device_start()
{
	m_bg_tilemap    = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(edevices_device::get_bg_tile_info)),    TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_mlow_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(edevices_device::get_mlow_tile_info)),  TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_mhigh_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(edevices_device::get_mhigh_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_tx_tilemap    = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(edevices_device::get_tx_tile_info)),    TILEMAP_SCAN_ROWS,  8,  8, 64, 32);

	m_mlow_tilemap->set_transparent_pen(0);
	m_mhigh_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scroll_rows(256);
	m_mlow_tilemap->set_scroll_rows(256);
	m_mhigh_tilemap->set_scroll_rows(256);

	save_item(NAME(m_sprites_buffer));
	save_item(NAME(m_which));
}

void edevices_device::device_reset()
{
	m_which = 0;
}

WRITE16_MEMBER(edevices_device::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(edevices_device::mlow_videoram_w)
{
	COMBINE_DATA(&m_mlow_videoram[offset]);
	m_mlow_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(edevices_device::mhigh_videoram_w)
{
	COMBINE_DATA(&m_mhigh_videoram[offset]);
	m_mhigh_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(edevices_device::tx_videoram_w)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(edevices_device::sprites_commands_w)
{
	// TODO: I'm not convinced by this, from mwarr.cpp driver
	if (m_which)
	{
		int i;

		switch (data & 0xf)
		{
		case 0:
			/* clear sprites on screen */
			for (i = 0; i < 0x400; i++)
			{
				m_sprites_buffer[i] = 0;
			}
			m_which = 0;
			break;

		default:
			// allow everything else to fall through, other games are writing other values
		case 0xf: // mwarr
			/* refresh sprites on screen */
			for (i = 0; i < 0x400; i++)
			{
				m_sprites_buffer[i] = m_spriteram[i];
			}
			break;

		case 0xd:
			/* keep sprites on screen */
			break;
		}
	}

	m_which ^= 1;
}

TILE_GET_INFO_MEMBER(edevices_device::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index] & 0x1fff;
	int colour = (m_bg_videoram[tile_index] & 0xe000) >> 13;

	SET_TILE_INFO_MEMBER(4, tileno, colour, 0);
}

TILE_GET_INFO_MEMBER(edevices_device::get_mlow_tile_info)
{
	int tileno = m_mlow_videoram[tile_index] & 0x1fff;
	int colour = (m_mlow_videoram[tile_index] & 0xe000) >> 13;

	SET_TILE_INFO_MEMBER(3, tileno, colour, 0);
}

TILE_GET_INFO_MEMBER(edevices_device::get_mhigh_tile_info)
{
	int tileno = m_mhigh_videoram[tile_index] & 0x1fff;
	int colour = (m_mhigh_videoram[tile_index] & 0xe000) >> 13;

	SET_TILE_INFO_MEMBER(2, tileno, colour, 0);
}

TILE_GET_INFO_MEMBER(edevices_device::get_tx_tile_info)
{
	int tileno = m_tx_videoram[tile_index] & 0x1fff;
	int colour = (m_tx_videoram[tile_index] & 0xe000) >> 13;

	SET_TILE_INFO_MEMBER(1, tileno, colour, 0);
}


int edevices_device::get_priority(const uint16_t *source)
{
	return ((source[1] & 0x3c00) >> 10); // Priority (1 = Low)
}

// the Steel Force type hardware uses an entirely different bit for priority and only appears to have 2 levels
int edevices_sforce_device::get_priority(const uint16_t *source)
{
	return (source[1] & 0x0020) ? 0xc : 0x2;
}

void edevices_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint16_t *source = m_sprites_buffer + 0x400 - 4;
	const uint16_t *finish = m_sprites_buffer;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int x, y, color, flipx, dy, pri, pri_mask, i;

	while (source >= finish)
	{
		/* draw sprite */
		if (source[0] & 0x0800)
		{
			y = 0x1ff - (source[0] & 0x01ff);
			x = (source[3] & 0x3ff) - 9;

			color = source[1] & 0x000f;
			flipx = source[1] & 0x0200;

			dy = (source[0] & 0xf000) >> 12;

			pri = get_priority(source);
			pri_mask = ~((1 << (pri + 1)) - 1);     // Above the first "pri" levels

			x += m_spritexoffs;

			for (i = 0; i <= dy; i++)
			{
				gfx->prio_transpen(bitmap,
					cliprect,
					source[2] + i,
					color,
					flipx, 0,
					x, y + i * 16,
					screen.priority(), pri_mask, 0);

				/* wrap around x */
				gfx->prio_transpen(bitmap,
					cliprect,
					source[2] + i,
					color,
					flipx, 0,
					x - 1024, y + i * 16,
					screen.priority(), pri_mask, 0);

				/* wrap around y */
				gfx->prio_transpen(bitmap,
					cliprect,
					source[2] + i,
					color,
					flipx, 0,
					x, y - 512 + i * 16,
					screen.priority(), pri_mask, 0);

				/* wrap around x & y */
				gfx->prio_transpen(bitmap,
					cliprect,
					source[2] + i,
					color,
					flipx, 0,
					x - 1024, y - 512 + i * 16,
					screen.priority(), pri_mask, 0);
			}
		}

		source -= 0x4;
	}
}

/*
    m_vidattrram
    0 tx xscroll (or global x scroll?)
    1 back yscroll
    2 mlow yscroll
    3 mhigh yscroll
    4 tx yscroll
    5 ---- ---- ---s tMmB  layer enables (s = sprites, t = tx, M = highmid, m = lowmid, B = back)
    6 ---- ---- ---M -m-B rowscroll enables  (B = back, m = lowmid, M = highmid)

*/

uint32_t edevices_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	/* xscrolls - Steel Force clearly shows that each layer needs -1 scroll compared to the previous, do enable flags change this?  */
	if (BIT(m_vidattrram[6], 0))
	{
		for (i = 0; i < 256; i++)
			m_bg_tilemap->set_scrollx(i, m_bg_scrollram[i] + 18);
	}
	else
	{
		for (i = 0; i < 256; i++)
			m_bg_tilemap->set_scrollx(i, m_bg_scrollram[0] + 18);
	}

	if (BIT(m_vidattrram[6], 2))
	{
		for (i = 0; i < 256; i++)
			m_mlow_tilemap->set_scrollx(i, m_mlow_scrollram[i] + 17);
	}
	else
	{
		for (i = 0; i < 256; i++)
			m_mlow_tilemap->set_scrollx(i, m_mlow_scrollram[0] + 17);
	}

	if (BIT(m_vidattrram[6], 4))
	{
		for (i = 0; i < 256; i++)
			m_mhigh_tilemap->set_scrollx(i, m_mhigh_scrollram[i] + 16);
	}
	else
	{
		for (i = 0; i < 256; i++)
			m_mhigh_tilemap->set_scrollx(i, m_mhigh_scrollram[0] + 16);
	}

	m_tx_tilemap->set_scrollx(0, m_vidattrram[0] + 15);

	/* yscrolls */
	m_bg_tilemap->set_scrolly(0, m_vidattrram[1] + 1);
	m_mlow_tilemap->set_scrolly(0, m_vidattrram[2] + 1);
	m_mhigh_tilemap->set_scrolly(0, m_vidattrram[3] + 1);
	m_tx_tilemap->set_scrolly(0, m_vidattrram[4] + 1);


	if (BIT(m_vidattrram[5], 0))
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0x01);
	}

	if (BIT(m_vidattrram[5], 1))
	{
		m_mlow_tilemap->draw(screen, bitmap, cliprect, 0, 0x02);
	}

	if (BIT(m_vidattrram[5], 2))
	{
		m_mhigh_tilemap->draw(screen, bitmap, cliprect, 0, 0x04);
	}

	if (BIT(m_vidattrram[5], 3))
	{
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0x10);
	}

	if (BIT(m_vidattrram[5], 4))
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}
