// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Pierpaolo Prazzoli
/***************************************************************************

  Portraits
  video hardware emulation

***************************************************************************/

#include "emu.h"
#include "portrait.h"


void portrait_state::bgvideo_write(offs_t offset, uint8_t data)
{
	m_background->mark_tile_dirty(offset/2);
	m_bgvideoram[offset] = data;
}

void portrait_state::fgvideo_write(offs_t offset, uint8_t data)
{
	m_foreground->mark_tile_dirty(offset/2);
	m_fgvideoram[offset] = data;
}

inline void portrait_state::get_tile_info( tile_data &tileinfo, int tile_index, const uint8_t *source )
{
	int attr    = source[tile_index*2+0];
	int tilenum = source[tile_index*2+1];
	int flags   = 0;
	int color   = 0;

	// TODO: always set with bit 4
	if( attr & 0x20 ) flags = TILE_FLIPY;

	if (attr & 1)
		tilenum += 0x200;
	if (attr & 2)
		tilenum += 0x100;
	if (attr & 4)
		tilenum ^= 0x300;

	// TODO: bits 7-6, color upper banks?

	// TODO: kludgy, likely not right
	if (tilenum<0x100)
		color = ((tilenum&0xff)>>1)+0x00;
	else
		color = ((tilenum&0xff)>>1)+0x80;

	tileinfo.set(0, tilenum, color, flags );
}

TILE_GET_INFO_MEMBER(portrait_state::get_bg_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_bgvideoram );
}

TILE_GET_INFO_MEMBER(portrait_state::get_fg_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_fgvideoram );
}

void portrait_state::video_start()
{
	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(portrait_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(portrait_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_foreground->set_transparent_pen(7);

	save_item(NAME(m_scroll));
}

void portrait_state::portrait_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; i++)
	{
		int const data = (color_prom[i + 0] << 0) | (color_prom[i + 0x20] << 8);
		
		// [+0x00] bit 0-3, bit 6-4
		// [+0x20] bit 0-2, bit 7-5

		int ii = (data >> 0) & 0xf;
		//int b = ((data >> 4) & 0x7) * ii;
		//int r = ((data >> 8) & 0x7) * ii;
		//int g = ((data >> 13) & 0x7) * ii;
		int r = ii * 0x11;
		int g = ii * 0x11;
		int b = ii * 0x11;

		palette.set_indirect_color(i, rgb_t(r, g, b));

		// ?? the lookup seems to reference 0x3f colours, unless 1 bit is priority or similar?
		palette.set_indirect_color(i + 0x20, rgb_t(r, g, b));
	}

	uint8_t const *const lookup = memregion("tileattr")->base();
	for (int i = 0; i < 0x800; i++)
	{
		uint8_t const ctabentry = lookup[i] & 0x3f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

/*
 * [2]
 * x--- ---- priority?
 * -x-x ---- ?
 * --x- ---- flipy
 * ---- x--- msb Y position?
 * ---- -x-- msb X position?
 */
void portrait_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u8 priority)
{
	uint8_t *source = m_spriteram;
	uint8_t *finish = source + 0x200;

	for( ; source < finish; source += 0x10 )
	{
		int attr    = source[2];
		if (BIT(attr, 7) != priority)
			continue;
		int sy      = source[0];
		int sx      = source[1];
		int tilenum = source[3];

		int color = ((tilenum&0xff)>>1)+0x00;

		int fy = attr & 0x20;

		if(attr & 0x04) sx |= 0x100;
		if(attr & 0x08) sy |= 0x100;

		sx += (source - m_spriteram) - 8;
		sx &= 0x1ff;

		sy = (512 - 64) - sy;

		m_gfxdecode->gfx(0)->transpen(
			bitmap, cliprect,
			tilenum, color,
			0, fy,
			sx, sy,
			7);
	}
}

uint32_t portrait_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle cliprect_scroll, cliprect_no_scroll;

	cliprect_scroll = cliprect_no_scroll = cliprect;

	// TODO: make clipping areas more readable
	cliprect_no_scroll.min_x = cliprect_no_scroll.max_x - 111;
	cliprect_scroll.max_x    = cliprect_scroll.min_x    + 319;

	m_background->set_scrolly(0, 0);
	m_foreground->set_scrolly(0, 0);
	m_background->draw(screen, bitmap, cliprect_no_scroll, 0, 0);
	m_foreground->draw(screen, bitmap, cliprect_no_scroll, 0, 0);

	m_background->set_scrolly(0, m_scroll);
	m_foreground->set_scrolly(0, m_scroll);
	m_background->draw(screen, bitmap, cliprect_scroll, 0, 0);
	draw_sprites(bitmap, cliprect_scroll, 0);
	m_foreground->draw(screen, bitmap, cliprect_scroll, 0, 0);
	draw_sprites(bitmap, cliprect_scroll, 1);

	return 0;
}
