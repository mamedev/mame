// license:BSD-3-Clause
// copyright-holders:David Haywood, Bryan McPhail
#include "emu.h"
#include "includes/pktgaldx.h"
#include "screen.h"

/* Video on the orginal */

uint32_t pktgaldx_state::screen_update_pktgaldx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen->pf_control_r(0);

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(!BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(0, cliprect); /* not Confirmed */
	screen.priority().fill(0);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/* Video for the bootleg */

uint32_t pktgaldx_state::screen_update_pktgaldb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	int offset = 0;
	int tileno;
	int colour;

	bitmap.fill(m_palette->black_pen(), cliprect);

	/* the bootleg seems to treat the tilemaps as sprites */
	for (offset = 0; offset < 0x1600 / 2; offset += 8)
	{
		tileno = m_pktgaldb_sprites[offset + 3] | (m_pktgaldb_sprites[offset + 2] << 16);
		colour = m_pktgaldb_sprites[offset + 1] >> 1;
		x = m_pktgaldb_sprites[offset + 0];
		y = m_pktgaldb_sprites[offset + 4];

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tileno ^ 0x1000, colour, 0, 0, x, y, 0);
	}

	for (offset = 0x1600/2; offset < 0x2000 / 2; offset += 8)
	{
		tileno = m_pktgaldb_sprites[offset + 3] | (m_pktgaldb_sprites[offset + 2] << 16);
		colour = m_pktgaldb_sprites[offset + 1] >> 1;
		x = m_pktgaldb_sprites[offset + 0] & 0x1ff;
		y = m_pktgaldb_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tileno ^ 0x4000, colour, 0, 0, x, y, 0);
	}

	for (offset = 0x2000/2; offset < 0x4000 / 2; offset += 8)
	{
		tileno = m_pktgaldb_sprites[offset + 3] | (m_pktgaldb_sprites[offset + 2] << 16);
		colour = m_pktgaldb_sprites[offset + 1] >> 1;
		x = m_pktgaldb_sprites[offset + 0] & 0x1ff;
		y = m_pktgaldb_sprites[offset + 4] & 0x0ff;

		x -= 0xc2;
		y &= 0x1ff;
		y -= 8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tileno ^ 0x3000, colour, 0, 0, x, y, 0);
	}

	return 0;
}
