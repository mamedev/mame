// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Uki
/***************************************************************************

    video/aeroboto.c

***************************************************************************/


#include "emu.h"
#include "aeroboto.h"


// how the starfield ROM is interpreted: 0=256x256x1 linear bitmap, 1=8x8x1x1024 tilemap
#define STARS_LAYOUT 1

// scroll speed of the stars: 1=normal, 2=half, 3=one-third...etc.(possitive integers only)
#define SCROLL_SPEED 1

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(aeroboto_state::get_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	tileinfo.set(0,
			code + (m_charbank << 8),
			m_tilecolor[code],
			(m_tilecolor[code] >= 0x33) ? 0 : TILE_FORCE_LAYER0);
}
// transparency should only affect tiles with color 0x33 or higher


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void aeroboto_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aeroboto_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 64);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_rows(64);

	save_item(NAME(m_charbank));
	save_item(NAME(m_starsoff));
	save_item(NAME(m_sx));
	save_item(NAME(m_sy));
	save_item(NAME(m_ox));
	save_item(NAME(m_oy));

	#if STARS_LAYOUT
	{
		int i;

		std::vector<uint8_t> temp(m_stars_length);
		memcpy(&temp[0], m_stars_rom, m_stars_length);

		for (i = 0; i < m_stars_length; i++)
			m_stars_rom[(i & ~0xff) + (i << 5 & 0xe0) + (i >> 3 & 0x1f)] = temp[i];
	}
	#endif
}



/***************************************************************************

  Memory handlers

***************************************************************************/

uint8_t aeroboto_state::aeroboto_in0_r()
{
	return ioport(flip_screen() ? "P2" : "P1")->read();
}

void aeroboto_state::aeroboto_3000_w(uint8_t data)
{
	/* bit 0 selects both flip screen and player1/player2 controls */
	flip_screen_set(data & 0x01);

	/* bit 1 = char bank select */
	if (m_charbank != ((data & 0x02) >> 1))
	{
		m_bg_tilemap->mark_all_dirty();
		m_charbank = (data & 0x02) >> 1;
	}

	/* bit 2 = disable star field? */
	m_starsoff = data & 0x4;
}

void aeroboto_state::aeroboto_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void aeroboto_state::aeroboto_tilecolor_w(offs_t offset, uint8_t data)
{
	if (m_tilecolor[offset] != data)
	{
		m_tilecolor[offset] = data;
		m_bg_tilemap->mark_all_dirty();
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

void aeroboto_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int x = m_spriteram[offs + 3];
		int y = 240 - m_spriteram[offs];

		if (flip_screen())
		{
			x = 248 - x;
			y = 240 - y;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				m_spriteram[offs + 1],
				m_spriteram[offs + 2] & 0x07,
				flip_screen(), flip_screen(),
				((x + 8) & 0xff) - 8, y, 0);
	}
}


uint32_t aeroboto_state::screen_update_aeroboto(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle const splitrect1(0, 255, 0, 39);
	rectangle const splitrect2(0, 255, 40, 255);
	int sky_color, star_color;

	sky_color = star_color = *m_bgcolor << 2;

	// the star field is supposed to be seen through tile pen 0 when active
	if (!m_starsoff)
	{
		if (star_color < 0xd0)
		{
			star_color = 0xd0;
			sky_color = 0;
		}

		star_color += 2;

		bitmap.fill(sky_color, cliprect);

		// actual scroll speed is unknown but it can be adjusted by changing the SCROLL_SPEED constant
		m_sx += char(*m_starx - m_ox);
		m_ox = *m_starx;
		int const x = m_sx / SCROLL_SPEED;

		if (*m_vscroll != 0xff)
			m_sy += char(*m_stary - m_oy);
		m_oy = *m_stary;
		int const y = m_sy / SCROLL_SPEED;

		uint8_t const *const src_base = m_stars_rom;

		for (int i = 0; i < 256; i++)
		{
			int src_offsx = (x + i) & 0xff;
			int const src_colmask = 1 << (src_offsx & 7);
			src_offsx >>= 3;
			uint8_t const *const src_colptr = src_base + src_offsx;
			int const pen = star_color + ((i + 8) >> 4 & 1);

			for (int j = 0; j < 256; j++)
			{
				uint8_t const *const src_rowptr = src_colptr + (((y + j) & 0xff) << 5 );
				if (!((unsigned)*src_rowptr & src_colmask))
					bitmap.pix(j, i) = pen;
			}
		}
	}
	else
	{
		m_sx = m_ox = *m_starx;
		m_sy = m_oy = *m_stary;
		bitmap.fill(sky_color, cliprect);
	}

	for (int y = 0; y < 64; y++)
		m_bg_tilemap->set_scrollx(y, m_hscroll[y]);

	// the playfield is part of a splitscreen and should not overlap with status display
	m_bg_tilemap->set_scrolly(0, *m_vscroll);
	m_bg_tilemap->draw(screen, bitmap, splitrect2, 0, 0);

	draw_sprites(bitmap, cliprect);

	// the status display behaves more closely to a 40-line splitscreen than an overlay
	m_bg_tilemap->set_scrolly(0, 0);
	m_bg_tilemap->draw(screen, bitmap, splitrect1, 0, 0);
	return 0;
}
