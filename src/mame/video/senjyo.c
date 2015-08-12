// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "machine/segacrpt.h"
#include "includes/senjyo.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(senjyo_state::get_fg_tile_info)
{
	UINT8 attr = m_fgcolorram[tile_index];
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	if (m_is_senjyo && (tile_index & 0x1f) >= 32-8)
		flags |= TILE_FORCE_LAYER0;

	SET_TILE_INFO_MEMBER(0,
			m_fgvideoram[tile_index] + ((attr & 0x10) << 4),
			attr & 0x07,
			flags);
}

TILE_GET_INFO_MEMBER(senjyo_state::senjyo_bg1_tile_info)
{
	UINT8 code = m_bg1videoram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code,
			(code & 0x70) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(senjyo_state::starforc_bg1_tile_info)
{
	/* Star Force has more tiles in bg1, so to get a uniform color code spread */
	/* they wired bit 7 of the tile code in place of bit 4 to get the color code */
	static const UINT8 colormap[8] = { 0, 2, 4, 6, 1, 3, 5, 7 };
	UINT8 code = m_bg1videoram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code,
			colormap[(code & 0xe0) >> 5],
			0);
}

TILE_GET_INFO_MEMBER(senjyo_state::get_bg2_tile_info)
{
	UINT8 code = m_bg2videoram[tile_index];

	SET_TILE_INFO_MEMBER(2,
			code,
			(code & 0xe0) >> 5,
			0);
}

TILE_GET_INFO_MEMBER(senjyo_state::get_bg3_tile_info)
{
	UINT8 code = m_bg3videoram[tile_index];

	SET_TILE_INFO_MEMBER(3,
			code,
			(code & 0xe0) >> 5,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void senjyo_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	if (m_is_senjyo)
	{
		m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::senjyo_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 32);
		m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::get_bg2_tile_info),this),    TILEMAP_SCAN_ROWS, 16, 16, 16, 48);   /* only 16x32 used by Star Force */
		m_bg3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::get_bg3_tile_info),this),    TILEMAP_SCAN_ROWS, 16, 16, 16, 56);   /* only 16x32 used by Star Force */
	}
	else
	{
		m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::starforc_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 32);
		m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::get_bg2_tile_info),this),      TILEMAP_SCAN_ROWS, 16, 16, 16, 32); /* only 16x32 used by Star Force */
		m_bg3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(senjyo_state::get_bg3_tile_info),this),      TILEMAP_SCAN_ROWS, 16, 16, 16, 32); /* only 16x32 used by Star Force */
	}

	m_fg_tilemap->set_transparent_pen(0);
	m_bg1_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);
	m_bg3_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scroll_cols(32);
}

PALETTE_DECODER_MEMBER( senjyo_state, IIBBGGRR )
{
	UINT8 i = (raw >> 6) & 3;
	UINT8 r = (raw << 2) & 0x0c;
	UINT8 g = (raw     ) & 0x0c;
	UINT8 b = (raw >> 2) & 0x0c;

	return rgb_t(pal4bit(r ? (r | i) : 0), pal4bit(g ? (g | i) : 0), pal4bit(b ? (b | i) : 0));
}

PALETTE_INIT_MEMBER( senjyo_state, radar )
{
	// two colors for the radar dots (verified on the real board)
	m_radar_palette->set_pen_color(0, rgb_t(0xff, 0x00, 0x00));  // red for enemies
	m_radar_palette->set_pen_color(1, rgb_t(0xff, 0xff, 0x00));  // yellow for player
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(senjyo_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}
WRITE8_MEMBER(senjyo_state::fgcolorram_w)
{
	m_fgcolorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}
WRITE8_MEMBER(senjyo_state::bg1videoram_w)
{
	m_bg1videoram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset);
}
WRITE8_MEMBER(senjyo_state::bg2videoram_w)
{
	m_bg2videoram[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset);
}
WRITE8_MEMBER(senjyo_state::bg3videoram_w)
{
	m_bg3videoram[offset] = data;
	m_bg3_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************

  Display refresh

***************************************************************************/

void senjyo_state::draw_bgbitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_bgstripes == 0xff) /* off */
		bitmap.fill(m_palette->pen_color(0), cliprect);
	else
	{
		int flip = flip_screen();

		int pen = 0;
		int count = 0;
		int strwid = m_bgstripes;
		if (strwid == 0) strwid = 0x100;
		if (flip) strwid ^= 0xff;

		for (int x = 0;x < 256;x++)
		{
			if (flip)
				for (int y = 0;y < 256;y++)
					bitmap.pix32(y, 255 - x) = m_palette->pen_color(384 + pen);
			else
				for (int y = 0;y < 256;y++)
					bitmap.pix32(y, x) = m_palette->pen_color(384 + pen);

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
					bitmap.pix32(sy, sx) =  m_radar_palette->pen_color(offs < 0x200 ? 0 : 1);
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
			sx = m_spriteram[offs+3];
			if (big)
				sy = 224-m_spriteram[offs+2];
			else
				sy = 240-m_spriteram[offs+2];
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

UINT32 senjyo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

#if 0
{
	char baf[80];
	UINT8 *senjyo_scrolly3 = m_scrolly3;

	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x00],
		senjyo_scrolly3[0x01],
		senjyo_scrolly3[0x02],
		senjyo_scrolly3[0x03],
		senjyo_scrolly3[0x04],
		senjyo_scrolly3[0x05],
		senjyo_scrolly3[0x06],
		senjyo_scrolly3[0x07]);
	ui_draw_text(baf,0,0);
	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x08],
		senjyo_scrolly3[0x09],
		senjyo_scrolly3[0x0a],
		senjyo_scrolly3[0x0b],
		senjyo_scrolly3[0x0c],
		senjyo_scrolly3[0x0d],
		senjyo_scrolly3[0x0e],
		senjyo_scrolly3[0x0f]);
	ui_draw_text(baf,0,10);
	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x10],
		senjyo_scrolly3[0x11],
		senjyo_scrolly3[0x12],
		senjyo_scrolly3[0x13],
		senjyo_scrolly3[0x14],
		senjyo_scrolly3[0x15],
		senjyo_scrolly3[0x16],
		senjyo_scrolly3[0x17]);
	ui_draw_text(baf,0,20);
	sprintf(baf,"%02x %02x %02x %02x %02x %02x %02x %02x",
		senjyo_scrolly3[0x18],
		senjyo_scrolly3[0x19],
		senjyo_scrolly3[0x1a],
		senjyo_scrolly3[0x1b],
		senjyo_scrolly3[0x1c],
		senjyo_scrolly3[0x1d],
		senjyo_scrolly3[0x1e],
		senjyo_scrolly3[0x1f]);
	ui_draw_text(baf,0,30);
}
#endif
	return 0;
}
