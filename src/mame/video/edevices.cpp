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
*/


#include "emu.h"
#include "screen.h"
#include "emupal.h"
#include "edevices.h"


DEFINE_DEVICE_TYPE(EDEVICES_VID, edevices_device, "edevices_vid", "Electronic Devices Video")

edevices_device::edevices_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EDEVICES_VID, tag, owner, clock),
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
	m_palette(*this, finder_base::DUMMY_TAG)
{
}

void edevices_device::device_start()
{
	m_bg_tilemap    = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(edevices_device::get_bg_tile_info),this),    TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_mlow_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(edevices_device::get_mlow_tile_info),this),  TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_mhigh_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(edevices_device::get_mhigh_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_tx_tilemap    = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(edevices_device::get_tx_tile_info),this),    TILEMAP_SCAN_ROWS,  8,  8, 64, 32);

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
	if (m_which)
	{
		int i;

		switch (data)
		{
		case 0:
			/* clear sprites on screen */
			for (i = 0; i < 0x800; i++)
			{
				m_sprites_buffer[i] = 0;
			}
			m_which = 0;
			break;

		default:
			logerror("used unknown sprites command %02X\n",data);
		case 0xf:
			/* refresh sprites on screen */
			for (i = 0; i < 0x800; i++)
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


void edevices_device::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const uint16_t *source = m_sprites_buffer + 0x800 - 4;
	const uint16_t *finish = m_sprites_buffer;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int x, y, color, flipx, dy, pri, pri_mask, i;

	while (source >= finish)
	{
		/* draw sprite */
		if (source[0] & 0x0800)
		{
			y = 512 - (source[0] & 0x01ff);
			x = (source[3] & 0x3ff) - 9;

			color = source[1] & 0x000f;
			flipx = source[1] & 0x0200;

			dy = (source[0] & 0xf000) >> 12;

			pri = ((source[1] & 0x3c00) >> 10); // Priority (1 = Low)
			pri_mask = ~((1 << (pri + 1)) - 1);     // Above the first "pri" levels

			for (i = 0; i <= dy; i++)
			{
				gfx->prio_transpen(bitmap,
							cliprect,
							source[2]+i,
							color,
							flipx,0,
							x,y+i*16,
							screen.priority(),pri_mask,0 );

				/* wrap around x */
				gfx->prio_transpen(bitmap,
							cliprect,
							source[2]+i,
							color,
							flipx,0,
							x-1024,y+i*16,
							screen.priority(),pri_mask,0 );

				/* wrap around y */
				gfx->prio_transpen(bitmap,
							cliprect,
							source[2]+i,
							color,
							flipx,0,
							x,y-512+i*16,
							screen.priority(),pri_mask,0 );

				/* wrap around x & y */
				gfx->prio_transpen(bitmap,
							cliprect,
							source[2]+i,
							color,
							flipx,0,
							x-1024,y-512+i*16,
							screen.priority(),pri_mask,0 );
			}
		}

		source -= 0x4;
	}
}

uint32_t edevices_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	screen.priority().fill(0, cliprect);

	if (BIT(m_vidattrram[6], 0))
	{
		for (i = 0; i < 256; i++)
			m_bg_tilemap->set_scrollx(i, m_bg_scrollram[i] + 20);
	}
	else
	{
		for (i = 0; i < 256; i++)
			m_bg_tilemap->set_scrollx(i, m_bg_scrollram[0] + 19);
	}

	if (BIT(m_vidattrram[6], 2))
	{
		for (i = 0; i < 256; i++)
			m_mlow_tilemap->set_scrollx(i, m_mlow_scrollram[i] + 19);
	}
	else
	{
		for (i = 0; i < 256; i++)
			m_mlow_tilemap->set_scrollx(i, m_mlow_scrollram[0] + 19);
	}

	if (BIT(m_vidattrram[6], 4))
	{
		for (i = 0; i < 256; i++)
			m_mhigh_tilemap->set_scrollx(i, m_mhigh_scrollram[i] + 19);
	}
	else
	{
		for (i = 0; i < 256; i++)
			m_mhigh_tilemap->set_scrollx(i, m_mhigh_scrollram[0] + 19);
	}

	m_bg_tilemap->set_scrolly(0, m_vidattrram[1] + 1);
	m_mlow_tilemap->set_scrolly(0, m_vidattrram[2] + 1);
	m_mhigh_tilemap->set_scrolly(0, m_vidattrram[3] + 1);

	m_tx_tilemap->set_scrollx(0, m_vidattrram[0] + 16);
	m_tx_tilemap->set_scrolly(0, m_vidattrram[4] + 1);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0x01);
	m_mlow_tilemap->draw(screen, bitmap, cliprect, 0, 0x02);
	m_mhigh_tilemap->draw(screen, bitmap, cliprect, 0, 0x04);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0x10);
	draw_sprites(screen, bitmap, cliprect);
	return 0;
}
