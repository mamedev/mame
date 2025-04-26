// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/***************************************************************************

  Video Hardware for Double Dragon 3

***************************************************************************/

#include "emu.h"
#include "ddragon3.h"

void ddragon3_state::ddragon3_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 0: COMBINE_DATA(&m_fg_scrollx);  break; // Scroll X, BG1
		case 1: COMBINE_DATA(&m_fg_scrolly);  break; // Scroll Y, BG1
		case 2: COMBINE_DATA(&m_bg_scrollx);  break; // Scroll X, BG0
		case 3: COMBINE_DATA(&m_bg_scrolly);  break; // Scroll Y, BG0
		case 4:                               break; // Unknown write
		case 5: flip_screen_set(data & 0x01); break; // Flip Screen
		case 6:
			COMBINE_DATA(&m_bg_tilebase);            // BG Tile Base
			m_bg_tilebase &= 0x1ff;
			m_bg_tilemap->mark_all_dirty();
			break;
	}
}

uint16_t ddragon3_state::ddragon3_scroll_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_fg_scrollx;
		case 1: return m_fg_scrolly;
		case 2: return m_bg_scrollx;
		case 3: return m_bg_scrolly;
		case 5: return flip_screen();
		case 6: return m_bg_tilebase;
	}

	return 0;
}

/*****************************************************************************************************************************************************/

TILE_GET_INFO_MEMBER(ddragon3_state::get_bg_tile_info)
{
	uint16_t attr = m_bg_videoram[tile_index];
	int code = (attr & 0x0fff) | ((m_bg_tilebase & 0x01) << 12);
	int color = ((attr & 0xf000) >> 12);

	tileinfo.set(0, code, color, 0);
}



void ddragon3_state::ddragon3_bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}



/*****************************************************************************************************************************************************/

TILE_GET_INFO_MEMBER(ddragon3_state::get_fg_tile_info)
{
	uint16_t *tilebase = &m_fg_videoram[tile_index * 2];
	int tileno = (tilebase[1] & 0x1fff);
	int colbank = (tilebase[0] & 0x000f);

	tileinfo.set(1, tileno, colbank, TILE_FLIPYX((tilebase[0] & 0x00c0) >> 6));
}



void ddragon3_state::ddragon3_fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}



/*****************************************************************************************************************************************************/
/* 8x8 text layer - wwfwfest only */

TILE_GET_INFO_MEMBER(wwfwfest_state::get_fg0_tile_info)
{
	uint16_t *tilebase = &m_fg0_videoram[tile_index * 2];
	int tileno = (tilebase[0] & 0x00ff) | ((tilebase[1] & 0x000f) << 8);
	int colbank = (tilebase[1] & 0x00f0) >> 4;

	tileinfo.set(3, tileno, colbank, 0);
}

void wwfwfest_state::wwfwfest_fg0_videoram_w(offs_t offset, uint16_t data)
{
	// Videoram is 8 bit, upper & lower byte writes end up in the same place due to m68k byte smearing
	m_fg0_videoram[offset] = data & 0xff;
	m_fg0_tilemap->mark_tile_dirty(offset / 2);
}

/*****************************************************************************************************************************************************/

void ddragon3_state::video_start()
{
	save_item(NAME(m_pri));

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddragon3_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddragon3_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_sprite_xoff = m_bg0_dx = m_bg1_dx[0] = m_bg1_dx[1] = 0;
	m_bg_tilebase = 0;
}

void wwfwfest_state::video_start()
{
	ddragon3_state::video_start();

	m_fg0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wwfwfest_state::get_fg0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg0_tilemap->set_transparent_pen(0);

}

VIDEO_START_MEMBER(wwfwfest_state,wwfwfstb)
{
	wwfwfest_state::video_start();

	m_sprite_xoff = 2;
	m_bg0_dx = m_bg1_dx[0] = -4;
	m_bg1_dx[1] = -2;
}

/*****************************************************************************************************************************************************/

void ddragon3_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*- SPR RAM Format -**

	  16 bytes per sprite  (8-bit RAM? only every other byte is used)

	  ---- ----  yyyy yyyy  ---- ----  lllF fXYE  ---- ----  nnnn nnnn  ---- ----  NNNN NNNN
	  ---- ----  ---- CCCC  ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ----

	  Yy = sprite Y Position
	  Xx = sprite X Position
	  C  = colour bank
	  f  = flip Y
	  F  = flip X
	  l  = chain sprite
	  E  = sprite enable
	  Nn = Sprite Number

	  other bits unused
	*/

	uint16_t *buffered_spriteram16 = m_spriteram->buffer();
	int length = m_spriteram->bytes();
	gfx_element *gfx = m_gfxdecode->gfx(2);
	uint16_t *source = buffered_spriteram16;
	uint16_t *finish = source + length / 2;

	while (source < finish)
	{
		if (source[1] & 0x0001)
		{
			int xpos = (source[5] & 0x00ff) | (source[1] & 0x0004) << 6;
			if (xpos > 512 - 16) xpos -= 512;
			xpos += m_sprite_xoff;

			int ypos = (source[0] & 0x00ff) | (source[1] & 0x0002) << 7;
			ypos = (256 - ypos) & 0x1ff;
			ypos -= 16;

			int flipx = (source[1] & 0x0010) >> 4;
			int flipy = (source[1] & 0x0008) >> 3;
			int chain = (source[1] & 0x00e0) >> 5;
			int number = (source[2] & 0x00ff) | (source[3] & 0x00ff) << 8;
			int colourbank = (source[4] & 0x000f);

			if (flip_screen())
			{
				flipy = !flipy;
				flipx = !flipx;
				ypos = 240 - ypos - m_sprite_xoff;
				xpos = 304 - xpos;
			}

			for (int count = 0; count <= chain; count++)
			{
				if (flip_screen())
				{
					if (!flipy)
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos + (16 * chain) - (16 * count), 0);
					else
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos + 16 * count, 0);
				}
				else
				{
					if (flipy)
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos - (16 * chain) + (16 * count), 0);
					else
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos - 16 * count, 0);
				}
			}
		}

		source += 8;
	}
}



/*****************************************************************************************************************************************************/

uint32_t ddragon3_state::screen_update_ddragon3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
	m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
	m_fg_tilemap->set_scrolly(0, m_fg_scrolly);

	if ((m_vreg & 0x60) == 0x40)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}
	else if ((m_vreg & 0x60) == 0x60)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	return 0;
}

uint32_t ddragon3_state::screen_update_ctribe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
	m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
	m_fg_tilemap->set_scrolly(0, m_fg_scrolly);

	if (m_vreg & 8)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}
	return 0;
}


uint32_t wwfwfest_state::screen_update_wwfwfest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_pri == 0x78)
	{
		m_fg_tilemap->set_scrolly(0, m_fg_scrolly);
		m_fg_tilemap->set_scrollx(0, m_fg_scrollx + m_bg0_dx);
		m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
		m_bg_tilemap->set_scrollx(0, m_bg_scrollx + m_bg1_dx[0]);
	}
	else
	{
		m_bg_tilemap->set_scrolly(0, m_fg_scrolly);
		m_bg_tilemap->set_scrollx(0, m_fg_scrollx + m_bg1_dx[1]);
		m_fg_tilemap->set_scrolly(0, m_bg_scrolly);
		m_fg_tilemap->set_scrollx(0, m_bg_scrollx + m_bg0_dx);
	}

	// TODO: which bits of pri are significant to the order
	if (m_pri == 0x7b)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}

	if (m_pri == 0x7c)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}

	if (m_pri == 0x78)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}

	m_fg0_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
