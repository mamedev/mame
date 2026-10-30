// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail, Stephane Humbert, Angelo Salese
/***************************************************************************

   Alpha 68k video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "alpha68k.h"

// TODO: used by alpha68k_i.cpp and _n.cpp, move to own file
void alpha68k_prom_state::palette_init(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();
	const u8 *clut_proms = memregion("clut_proms")->base();
	const u32 clut_romsize =  memregion("clut_proms")->bytes()/2;

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x100; i++)
	{
		const int r = pal4bit(color_prom[i + 0x000]);
		const int g = pal4bit(color_prom[i + 0x100]);
		const int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < clut_romsize; i++)
	{
		const u8 ctabentry = ((clut_proms[i + clut_romsize] & 0x0f) << 4) | (clut_proms[i] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}

void alpha68k_II_state::flipscreen_w(int flip)
{
	m_flipscreen = flip;
	m_sprites->set_flip(flip);
}

void alpha68k_II_state::video_bank_w(u8 data)
{
	if ((m_bank_base ^ data) & 0xf)
	{
		m_bank_base = data & 0xf;
		m_fix_tilemap->mark_all_dirty();
	}
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(alpha68k_II_state::get_tile_info)
{
	const u8 tile = m_videoram[2 * tile_index] & 0xff;
	const u8 attr =  m_videoram[2 * tile_index + 1] & 0xff;
	const u8 color = attr & 0x0f;
	const bool opaque = BIT(attr, 4);

	tileinfo.set(0, tile | (m_bank_base << 8), color, opaque ? TILE_FORCE_LAYER0 : 0);
}

void alpha68k_II_state::videoram_w(offs_t offset, u16 data)
{
	/* 8 bit RAM, upper & lower byte writes end up in the same place due to m68k byte smearing */
	m_videoram[offset] = data & 0xff;

	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

VIDEO_START_MEMBER(alpha68k_II_state,alpha68k)
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(alpha68k_II_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/

// TODO: sprite flip select as in snk/snk68.cpp, palette bank for V games if they ever trigger it
void alpha68k_II_state::video_control2_w(int state)
{
	logerror("%s: Q2 changed to %d\n", machine().describe_context(), state);
}

void alpha68k_II_state::video_control3_w(int state)
{
	logerror("%s: Q3 changed to %d\n", machine().describe_context(), state);
}

void alpha68k_II_state::tile_callback(u32 &tile, bool &fx, bool &fy, u8 &region, u32 &color)
{
	fx = tile & 0x4000;
	fy = tile & 0x8000;
	tile &= 0x3fff;
	region = 0;
	color &= m_color_entry_mask;
}

void alpha68k_II_state::tile_callback_noflipx(u32 &tile, bool &fx, bool &fy, u8 &region, u32 &color)
{
	fx = 0;
	fy = tile & 0x8000;
	tile &= 0x7fff;
	region = 0;
	color &= m_color_entry_mask;
}

void alpha68k_II_state::tile_callback_noflipy(u32 &tile, bool &fx, bool &fy, u8 &region, u32 &color)
{
	fx = tile & 0x8000;
	fy = 0;
	tile &= 0x7fff;
	region = 0;
	color &= m_color_entry_mask;
}

u32 alpha68k_II_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->get_backdrop_pen(), cliprect);

	m_sprites->draw_sprites_alt(bitmap, cliprect);

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
