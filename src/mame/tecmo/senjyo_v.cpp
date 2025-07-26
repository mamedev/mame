// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "senjyo.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(senjyo_state::get_fg_tile_info)
{
	uint8_t attr = m_fgcolorram[tile_index];
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	if (m_is_senjyo && (tile_index & 0x1f) >= 32-8)
		flags |= TILE_FORCE_LAYER0;

	tileinfo.set(0,
			m_fgvideoram[tile_index] + ((attr & 0x10) << 4),
			attr & 0x07,
			flags);
}

TILE_GET_INFO_MEMBER(senjyo_state::senjyo_bg1_tile_info)
{
	uint8_t code = m_bg1videoram[tile_index];

	tileinfo.set(1,
			code,
			(code & 0x70) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(senjyo_state::starforc_bg1_tile_info)
{
	/* Star Force has more tiles in bg1, so to get a uniform color code spread */
	/* they wired bit 7 of the tile code in place of bit 4 to get the color code */
	uint8_t code = m_bg1videoram[tile_index];

	tileinfo.set(1,
			code,
			bitswap<3>(((code & 0xe0) >> 5), 1, 0, 2),
			0);
}

TILE_GET_INFO_MEMBER(senjyo_state::get_bg2_tile_info)
{
	uint8_t code = m_bg2videoram[tile_index];

	tileinfo.set(2,
			code,
			(code & 0xe0) >> 5,
			0);
}

TILE_GET_INFO_MEMBER(senjyo_state::get_bg3_tile_info)
{
	uint8_t code = m_bg3videoram[tile_index];

	tileinfo.set(3,
			code,
			(code & 0xe0) >> 5,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void senjyo_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	if (m_is_senjyo)
	{
		m_bg1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::senjyo_bg1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 32);
		m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::get_bg2_tile_info)),    TILEMAP_SCAN_ROWS, 16, 16, 16, 48);   // only 16x32 used by Star Force
		m_bg3_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::get_bg3_tile_info)),    TILEMAP_SCAN_ROWS, 16, 16, 16, 56);   // only 16x32 used by Star Force
	}
	else
	{
		m_bg1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::starforc_bg1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 32);
		m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::get_bg2_tile_info)),      TILEMAP_SCAN_ROWS, 16, 16, 16, 32); // only 16x32 used by Star Force
		m_bg3_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(senjyo_state::get_bg3_tile_info)),      TILEMAP_SCAN_ROWS, 16, 16, 16, 32); // only 16x32 used by Star Force
	}

	m_fg_tilemap->set_transparent_pen(0);
	m_bg1_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);
	m_bg3_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scroll_cols(32);
}

rgb_t senjyo_state::IIBBGGRR(uint32_t raw)
{
	uint8_t const i = (raw >> 6) & 0x03;
	uint8_t const r = (raw << 2) & 0x0c;
	uint8_t const g = (raw     ) & 0x0c;
	uint8_t const b = (raw >> 2) & 0x0c;

	return rgb_t(pal4bit(r ? (r | i) : 0), pal4bit(g ? (g | i) : 0), pal4bit(b ? (b | i) : 0));
}

void senjyo_state::radar_palette(palette_device &palette) const
{
	// two colors for the radar dots (verified on the real board)
	palette.set_pen_color(0, rgb_t(0xff, 0x00, 0x00));  // red for enemies
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0x00));  // yellow for player
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void senjyo_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void senjyo_state::fgcolorram_w(offs_t offset, uint8_t data)
{
	m_fgcolorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void senjyo_state::bg1videoram_w(offs_t offset, uint8_t data)
{
	m_bg1videoram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset);
}

void senjyo_state::bg2videoram_w(offs_t offset, uint8_t data)
{
	m_bg2videoram[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset);
}

void senjyo_state::bg3videoram_w(offs_t offset, uint8_t data)
{
	m_bg3videoram[offset] = data;
	m_bg3_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************

  Display refresh

***************************************************************************/

void senjyo_state::draw_bgbitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// assume +1 from disabling layer being 0xff
	uint8_t stripe_width = m_bgstripesram[0]+1;
	if (stripe_width == 0)
		bitmap.fill(m_palette->pen_color(0), cliprect);
	else
	{
		int flip = flip_screen();

		int pen = 0;
		int count = 0;
		int strwid = stripe_width;
		if (strwid == 0) strwid = 0x100;
		if (flip) strwid ^= 0xff;

		for (int x = 0;x < 256;x++)
		{
			if (flip)
			{
				for (int y = 0;y < 256;y++)
					bitmap.pix(y, 255 - x) = m_palette->pen_color(384 + pen);
			}
			else
			{
				for (int y = 0;y < 256;y++)
					bitmap.pix(y, x) = m_palette->pen_color(384 + pen);
			}

			count += 0x10;
			if (count >= strwid)
			{
				pen = (pen + 1) & 0x0f;
				count -= strwid;
			}
		}
	}
}

void senjyo_state::draw_radar(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0;offs < 0x400;offs++)
		for (int x = 0;x < 8;x++)
			if (m_radarram[offs] & (1 << x))
			{
				int sx, sy;

				sx = (8 * (offs % 8) + x) + 256-64;
				sy = ((offs & 0x1ff) / 8) + 96;

				if (flip_screen())
				{
					sx = 255 - sx;
					sy = 255 - sy;
				}

				if (cliprect.contains(sx, sy))
					bitmap.pix(sy, sx) =  m_radar_palette->pen_color(offs < 0x200 ? 0 : 1);
			}
}

void senjyo_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int big,sx,sy,flipx,flipy;

		if (((m_spriteram[offs+1] & 0x30) >> 4) == priority)
		{
			if (m_is_senjyo) /* Senjyo */
				big = (m_spriteram[offs] & 0x80);
			else    /* Star Force */
				big = ((m_spriteram[offs] & 0xc0) == 0xc0);

			if (big)
				sy = 224-m_spriteram[offs+2];
			else
				sy = 240-m_spriteram[offs+2];

			sx = m_spriteram[offs+3];
			flipx = m_spriteram[offs+1] & 0x40;
			flipy = m_spriteram[offs+1] & 0x80;

			if (flip_screen())
			{
				flipx = !flipx;
				flipy = !flipy;

				if (big)
				{
					sx = 224 - sx;
					sy = 226 - sy;
				}
				else
				{
					sx = 240 - sx;
					sy = 242 - sy;
				}
			}

			m_gfxdecode->gfx(big ? 5 : 4)->transpen(bitmap,cliprect,
					m_spriteram[offs],
					m_spriteram[offs + 1] & 0x07,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

uint32_t senjyo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int flip = flip_screen();

	for (int i = 0;i < 32;i++)
		m_fg_tilemap->set_scrolly(i, m_fgscroll[i]);

	int scrollx = m_scrollx1[0];
	int scrolly = m_scrolly1[0] + 256 * m_scrolly1[1];
	if (flip)
		scrollx = -scrollx;
	m_bg1_tilemap->set_scrollx(0, scrollx);
	m_bg1_tilemap->set_scrolly(0, scrolly);

	scrollx = m_scrollx2[0];
	scrolly = m_scrolly2[0] + 256 * m_scrolly2[1];
	if (m_scrollhack)   /* Star Force, but NOT the encrypted version */
	{
		scrollx = m_scrollx1[0];
		scrolly = m_scrolly1[0] + 256 * m_scrolly1[1];
	}
	if (flip)
		scrollx = -scrollx;
	m_bg2_tilemap->set_scrollx(0, scrollx);
	m_bg2_tilemap->set_scrolly(0, scrolly);

	scrollx = m_scrollx3[0];
	scrolly = m_scrolly3[0] + 256 * m_scrolly3[1];
	if (flip)
		scrollx = -scrollx;
	m_bg3_tilemap->set_scrollx(0, scrollx);
	m_bg3_tilemap->set_scrolly(0, scrolly);

	draw_bgbitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect, 0);
	m_bg3_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 1);
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 2);
	m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 3);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_radar(bitmap, cliprect);

	return 0;
}
