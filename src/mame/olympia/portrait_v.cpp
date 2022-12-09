// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Pierpaolo Prazzoli, Angelo Salese
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

// NB: undisplayed areas gets filled at POST but never really used
// $8x36-$8x3f / $8x76-$8x7f / $8xb6-$8xbf / $8xf6-$8xff
// given that tilemap doesn't really cope well with live editing your best bet in debugging this is
// potentially to subscribe these unused areas to a mark_all_dirty() fn.
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

	// TODO: is the wild arrangement above related to how color upper banks should work?
	// cfr. trees in stage 4 leaving holes against the other layer

	// TODO: kludgy
	// at worst this is modified at mixing time, outputting sprite color for the status bar.
	if (tilenum<0x100)
		color = ((tilenum&0xfe) >> 1) + 0x00;
	else
		color = ((tilenum&0xfe) >> 1) + 0x80;

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

/* tileattr rom

  this appears to be divided into 2 0x400 banks

  0x000 - 0x3ff relates to tiles 0x000-0x0ff

  0x400 - 0x7ff relates to tiles 0x100-0x1ff, 0x200-0x2ff, and 0x300-0x3ff

  every 2 tiles are somehow related to 8 bytes in the data

   so tiles 0x00 and 0x01 use bytes 0x000-0x007
            0x02                    0x008
            0x04                    0x010
            0x06                    0x018
            0x08                    0x020
            0x0a                    0x028
            0x0c                    0x030
            0x0e                    0x038
            0x10                    0x040
               .......
            0xfe and 0xff use bytes 0x3f8-0x3ff
            etc.

    it's probably some kind of lookup table for the colours (6bpp = 8 colours, maybe every 2 tiles share the same 8 colours)
    I guess either the bank (0/1) can be selected, or bank 0 is hardcoded to tiles 0x000-0x0ff (because tilemaps can use
     these tiles too, so it's not a case of it being a sprite/tilemap lookup split)

    anyway.. this is why the portraits logo is broken across 3 areas (0x1f2, 0x2f2, 0x3f2) so that they can share the same
    attributes from this rom

*/
void portrait_state::portrait_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; i++)
	{
		int const data = (color_prom[i + 0] << 0) | (color_prom[i + 0x20] << 8);

		// TODO: experimental workbench, not using pal*bit intentionally.
		// 13 valid bits:
		// [+0x00] bit 0-3, bit 6-4
		// [+0x20] bit 0-2, bit 7-5
		// Only bit 0-3 seems to have a valid color ramp, is the rest actually bit mixed?

		int ii = (data >> 0) & 0x0f;
		//int b = ((data >> 4) & 0x7) * ii;
		//int r = ((data >> 8) & 0x7) * ii;
		//int g = ((data >> 13) & 0x7) * ii;
		int r = ii * 0x11;
		int g = ii * 0x11;
		int b = ii * 0x11;

		palette.set_indirect_color(i, rgb_t(r, g, b));

		ii = (data >> 1) & 0x0f;
		r = ii * 0x11;
		g = ii * 0x11;
		b = ii * 0x11;

		// ?? the lookup seems to reference 0x3f colours, unless 1 bit is something else (priority?)
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
 * -x-- ---- more priority?
 *           \- eagle sprite in stage 4 sets this only,
 *              drawing should really go above the fg layer mountains
 *              (missing tile category?)
 * --x- ---- flipy
 * ---x ---- ?
 * ---- x--- msb Y position?
 * ---- -x-- msb X position?
 */
void portrait_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u8 priority)
{
	uint8_t *source = m_spriteram;
	uint8_t *finish = source + 0x200;

	// TODO: is anything beyond byte [3] really just work RAM buffer?
	for( ; source < finish; source += 0x10 )
	{
		u8 attr    = source[2];
		if (BIT(attr, 7) != priority)
			continue;
		u16 sy     = source[0];
		u16 sx     = source[1];
		u8 tilenum = source[3];

		// TODO: may be given by source[4] and/or source[5] instead
		u8 color = ((tilenum&0xff)>>1)+0x00;

		int fy = BIT(attr, 5);

		if (BIT(attr, 2)) sx |= 0x100;
		if (BIT(attr, 3)) sy |= 0x100;

		sx += (source - m_spriteram) - 8;
		sx &= 0x1ff;

		// TODO: confirm Y calculation
		// it expects to follow up whatever is the current scroll value (cfr. monkeys climbing trees in stage 1)
		// but then there are various misc sprites that breaks this rule. Examples are:
		// - player photo flash;
		// - death animation sprites;
		// - capturing photo frame in gameplay;
		// PC=0x2828 is where all of these odd sprites happens, where:
		// HL=ROM pointer for destination sprite pointer, IY=sprite pointer source
		// where they copy the origin of the given sprite, read scroll buffer $9235-36 then apply offset,
		// with [2] bits 7-6 set high and bits 5-4 copied from the source sprite.
		// Note that this will break elsewhere by logically using any of the [2] bits,
		// arguably SW does a very limited use to pinpoint what's the actual scroll disable condition,
		// it just implicitly don't setup [4] to [7] ...
		if ((source[5] & 0xf) == 0)
			sy = (511 - 16) - sy;
		else
			sy = ((511 - m_scroll) - 16) - sy;

		sy &= 0x1ff;

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

	// status bar
	m_background->set_scrolly(0, 0);
	m_foreground->set_scrolly(0, 0);
	m_background->draw(screen, bitmap, cliprect_no_scroll, 0, 0);
	m_foreground->draw(screen, bitmap, cliprect_no_scroll, 0, 0);

	// playfield
	m_background->set_scrolly(0, m_scroll);
	m_foreground->set_scrolly(0, m_scroll);
	m_background->draw(screen, bitmap, cliprect_scroll, 0, 0);
	draw_sprites(bitmap, cliprect_scroll, 0);
	m_foreground->draw(screen, bitmap, cliprect_scroll, 0, 0);
	draw_sprites(bitmap, cliprect_scroll, 1);

	return 0;
}
