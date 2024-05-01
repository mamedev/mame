// license:BSD-3-Clause
// copyright-holders:Victor Trucco,Steve Ellenoff,Phil Stroffolino,Tatsuyuki Satoh,Tomasz Slanina,Nicola Salmoria
/***************************************************************************

    Video Hardware description for Taito Gladiator

***************************************************************************/

#include "emu.h"
#include "gladiatr.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(gladiatr_state_base::bg_get_tile_info)
{
	u8 const attr = m_colorram[tile_index];

	tileinfo.set(1,
			m_videoram[tile_index] + ((attr & 0x07) << 8) + (m_bg_tile_bank << 11),
			(attr >> 3) ^ 0x1f,
			0);
}

TILE_GET_INFO_MEMBER(gladiatr_state_base::fg_get_tile_info)
{
	tileinfo.set(0,
			m_textram[tile_index] + (m_fg_tile_bank << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void ppking_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ppking_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ppking_state::fg_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 64);

	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scroll_cols(0x10);

	m_sprite_bank = 1;

	save_item(NAME(m_video_attributes));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_sprite_buffer));
	save_item(NAME(m_fg_tile_bank));
}

void gladiatr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gladiatr_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gladiatr_state::fg_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-0x30, 0x12f);
	m_fg_tilemap->set_scrolldx(-0x30, 0x12f);

	m_sprite_bank = 2;

	save_item(NAME(m_video_attributes));
	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_sprite_buffer));
	save_item(NAME(m_fg_tile_bank));
	save_item(NAME(m_bg_tile_bank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void gladiatr_state_base::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gladiatr_state_base::colorram_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gladiatr_state_base::textram_w(offs_t offset, u8 data)
{
	m_textram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


void gladiatr_state_base::spritebuffer_w(int state)
{
	m_sprite_buffer = state;
}

void gladiatr_state::spritebank_w(int state)
{
	m_sprite_bank = state ? 4 : 2;
}


void ppking_state::ppking_video_registers_w(offs_t offset, u8 data)
{
	switch (offset & 0x300)
	{
		case 0x000:
			m_bg_tilemap->set_scrolly(offset & 0x0f, 0x100 - data);
			break;
		case 0x200:
			if (data & 0x80)
				m_fg_scrolly = data + 0x100;
			else
				m_fg_scrolly = data;
			break;
		case 0x300:
			if (m_fg_tile_bank != (data & 0x03))
			{
				m_fg_tile_bank = data & 0x03;
				m_fg_tilemap->mark_all_dirty();
			}
			m_video_attributes = data;
			break;
	}

//popmessage("%02x %02x",m_fg_scrolly, m_video_attributes);
}

void gladiatr_state::gladiatr_video_registers_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x000:
			m_fg_scrolly = data;
			break;
		case 0x080:
			if (m_fg_tile_bank != (data & 0x03))
			{
				m_fg_tile_bank = data & 0x03;
				m_fg_tilemap->mark_all_dirty();
			}
			if (m_bg_tile_bank != ((data & 0x10) >> 4))
			{
				m_bg_tile_bank = (data & 0x10) >> 4;
				m_bg_tilemap->mark_all_dirty();
			}
			m_video_attributes = data;
			break;
		case 0x100:
			m_fg_scrollx = data;
			break;
		case 0x200:
			m_bg_scrolly = data;
			break;
		case 0x300:
			m_bg_scrollx = data;
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

void gladiatr_state_base::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int tile_offset[2][2] =
	{
		{0x0,0x1},
		{0x2,0x3},
	};

	for (int offs = 0; offs < 0x80; offs += 2)
	{
		u8 const *const src = &m_spriteram[offs + (m_sprite_buffer << 7)];
		u8 const attributes = src[0x800];
		u8 const size = BIT(attributes, 4);
		u32 const bank = BIT(attributes, 0) + (BIT(attributes, 1) ? m_sprite_bank : 0);
		u32 const tile_number = (src[0] + 256 * bank);
		int sx = src[0x400 + 1] + 256 * (src[0x801] & 1) - 0x38;
		int sy = 240 - src[0x400] - (size ? 16 : 0);
		bool xflip = BIT(attributes, 2);
		bool yflip = BIT(attributes, 3);
		u32 const color = src[1] & 0x1f;

		if (flip_screen())
		{
			xflip = !xflip;
			yflip = !yflip;
		}

		for (int y = 0; y <= size; y++)
		{
			for (int x = 0; x <= size; x++)
			{
				int const ex = xflip ? (size - x) : x;
				int const ey = yflip ? (size - y) : y;

				u32 const t = tile_offset[ey][ex] + tile_number;

				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
						t,
						color,
						xflip, yflip,
						sx + x * 16, sy + y * 16, 0);
				// wraparound, used by Ping Pong King when scrolling from right to left
				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
						t,
						color,
						xflip, yflip,
						sx + x * 16, sy + y * 16 + 256, 0);
			}
		}
	}
}



u32 ppking_state::screen_update_ppking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);

	/* the fg layer just selects the upper palette bank on underlying pixels */
	{
		int sx = cliprect.min_x;
		int sy = cliprect.min_y;

		m_fg_tilemap->pixmap();
		bitmap_ind8 &flagsbitmap = m_fg_tilemap->flagsmap();

		while (sy <= cliprect.max_y)
		{
			int x = sx;
			int y = (sy + m_fg_scrolly) & 0x1ff;

			uint16_t *dest = &bitmap.pix(sy, sx);
			while (x <= cliprect.max_x)
			{
				if (flagsbitmap.pix(y, x) & TILEMAP_PIXEL_LAYER0)
				{
					*dest += 512;
				}
				x++;
				dest++;
			} /* next x */
			sy++;
		} /* next y */
	}
	return 0;
}

u32 gladiatr_state::screen_update_gladiatr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_video_attributes, 5))
	{
		int scroll;

		scroll = m_bg_scrollx + ((m_video_attributes & 0x04) << 6);
		m_bg_tilemap->set_scrollx(0, scroll ^ (flip_screen() ? 0x0f : 0));
		scroll = m_fg_scrollx + ((m_video_attributes & 0x08) << 5);
		m_fg_tilemap->set_scrollx(0, scroll ^ (flip_screen() ? 0x0f : 0));

		// always 0 anyway
		m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
		m_fg_tilemap->set_scrolly(0, m_fg_scrolly);

		m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect );
	return 0;
}
