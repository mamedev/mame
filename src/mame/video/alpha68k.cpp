// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail, Stephane Humbert, Angelo Salese
/***************************************************************************

   Alpha 68k video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "includes/alpha68k.h"

// TODO: used by alpha68k_i.cpp and _k.cpp, move to own file
void alpha68k_prom_state::palette_init(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();
	const u8 *clut_proms = memregion("clut_proms")->base();
	const u32 clut_romsize =  memregion("clut_proms")->bytes()/2;

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < clut_romsize; i++)
	{
		u8 const ctabentry = ((clut_proms[i + clut_romsize] & 0x0f) << 4) | (clut_proms[i] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}
}

void alpha68k_state::flipscreen_w(int flip)
{
	m_flipscreen = flip;
}

void alpha68k_state::video_bank_w(u8 data)
{
	if ((m_bank_base ^ data) & 0xf)
	{
		m_bank_base = data & 0xf;
		m_fix_tilemap->mark_all_dirty();
	}
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(alpha68k_state::get_tile_info)
{
	const u8 tile = m_videoram[2 * tile_index] & 0xff;
	const u8 attr =  m_videoram[2 * tile_index + 1] & 0xff;
	const u8 color = attr & 0x0f;
	const bool opaque = BIT(attr, 4);

	SET_TILE_INFO_MEMBER(0, tile | (m_bank_base << 8), color, opaque ? TILE_FORCE_LAYER0 : 0);
}

void alpha68k_state::videoram_w(offs_t offset, u16 data)
{
	/* 8 bit RAM, upper & lower byte writes end up in the same place due to m68k byte smearing */
	m_videoram[offset] = data & 0xff;

	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

VIDEO_START_MEMBER(alpha68k_state,alpha68k)
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(alpha68k_state::get_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/

//AT
void alpha68k_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e)
{
	for (int offs = s; offs < e; offs += 0x40)
	{
		int my = m_spriteram[offs + 3 + (j << 1)];
		int mx = m_spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
		my = -my & 0x1ff;
		mx = ((mx + 0x100) & 0x1ff) - 0x100;
		if (j == 0 && s == 0x7c0)
			my++;
//ZT
		if (m_flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		for (int i = 0; i < 0x40; i += 2)
		{
			u16 tile        = m_spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			const u8 color = m_spriteram[offs + i + (0x800 * j) + 0x800] & 0x7f;

			int fy = tile & 0x8000;
			int fx = tile & 0x4000;
			tile &= 0x3fff;

			if (m_flipscreen)
			{
				if (fx) fx = 0; else fx = 1;
				if (fy) fy = 0; else fy = 1;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tile,
				color,
				fx,fy,
				mx,my,0);

			if (m_flipscreen)
				my = (my - 16) & 0x1ff;
			else
				my = (my + 16) & 0x1ff;
		}
	}
}

/******************************************************************************/

u32 alpha68k_state::screen_update_alpha68k_II(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(2047, cliprect);
//AT
	draw_sprites(bitmap, cliprect, 0, 0x07c0, 0x0800);
	draw_sprites(bitmap, cliprect, 1, 0x0000, 0x0800);
	draw_sprites(bitmap, cliprect, 2, 0x0000, 0x0800);
	draw_sprites(bitmap, cliprect, 0, 0x0000, 0x07c0);
//ZT
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/

/*
    Video banking:

    Write to these locations in this order for correct bank:

    20 28 30 for Bank 0
    60 28 30 for Bank 1
    20 68 30 etc
    60 68 30
    20 28 70
    60 28 70
    20 68 70
    60 68 70 for Bank 7

    Actual data values written don't matter!

*/

/* Graphics flags?  Not related to fix chars anyway */
WRITE_LINE_MEMBER(alpha68k_state::video_control2_w)
{
	logerror("%s: Q2 changed to %d\n", machine().describe_context(), state);
}

WRITE_LINE_MEMBER(alpha68k_state::video_control3_w)
{
	logerror("%s: Q3 changed to %d\n", machine().describe_context(), state);
}

/******************************************************************************/

/*
 * Alpha 68k V sprite system
 * tile-based, with 8-bit accesses. 
 * Two banks, first at 0x0000-0x1000, second at 0x1000 until end of VRAM.
 * First bank is processed by 64 bytes stepping starting from address $4, 
 * then once it reaches end it restarts at $8, finally at $c.
 *
 * 0x0000-0x1000
 * [0]
 * ???? ???? untested in POST, actually written to by Gang Wars (likely NOP)
 * [1]
 * XXXX XXXX lower X offset
 * [2]
 * X--- ---- upper X offset
 * -*** ---- unknown, more fractional X?
 * ---- ---Y upper Y offset
 * [3]
 * YYYY YYYY lower Y offset
 *
 * Second bank has the actual tile info, and is arranged in vertical strips.
 * [0]
 * ???? ????  untested in POST, actually written to by Sky Adventure (likely NOP)
 * [1]
 * cccc cccc color entry
 * [2]
 * uuu- ---- user selectable, either flipx/flipy or tile bank
 * ---t tttt high tile offset
 * [3]
 * tttt tttt low tile offset
 *
 * TODO:
 * - Currently lags compared to itself, examples:
 *   - player death animation has first frame with inverted horizontal halves;
 *   - stage 1 priest desyncs with background;
 *   - glitchy first frame on title screen;
 *   Given how this and the actual HW works it is pretty likely this having a consistent delay, 
 *   however it isn't known how exactly DMA triggers, and one frame of bufferd spriteram isn't enough.
 * - Why entry 0x7c0 requires a one line and a priority hack?
 *
 */
void alpha68k_state::draw_sprites_V(bitmap_ind16 &bitmap, const rectangle &cliprect, int j, int s, int e, u16 fx_mask, u16 fy_mask, u16 sprite_mask)
{
	for (int offs = s; offs < e; offs += 0x40)
	{
		int my = m_spriteram[offs + 3 + (j << 1)];
		int mx = m_spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
		my = -my & 0x1ff;
		mx = ((mx + 0x100) & 0x1ff) - 0x100;
		// TODO: remove this hack
		if (j == 0 && s == 0x7c0)
			my++;

		if (m_flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		for (int i = 0; i < 0x40; i += 2)
		{
			u16 tile        = m_spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			const u8 color  = m_spriteram[offs + 0 + i + (0x800 * j) + 0x800] & 0xff;

			int fx = tile & fx_mask;
			int fy = tile & fy_mask;
			tile = tile & sprite_mask;

			if (m_flipscreen)
			{
				if (fx) fx = 0; else fx = 1;
				if (fy) fy = 0; else fy = 1;
			}

			// color 0 is actually selectable, cfr. Sky Adventure service mode or Gold Medalist player 1 status bar on long jump
			// TODO: are there any actual sprite disable conditions?
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				tile,
				color,
				fx,fy,
				mx,my,0);

			if (m_flipscreen)
				my = (my - 16) & 0x1ff;
			else
				my = (my + 16) & 0x1ff;
		}
	}
}

#ifdef UNUSED_FUNCTION
// AT: *KLUDGE* fixes priest priority in level 1(could be a game bug)
// Update: it is a btanb according to PCB reference, priest effectively goes above big ship (!)
// left here for reference
if (m_spriteram[0x1bde] == 0x24 && (m_spriteram[0x1bdf] >> 8) == 0x3b)
{
	draw_sprites_V(bitmap, cliprect, 2, 0x03c0, 0x0800, 0, 0x8000, 0x7fff);
	draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x03c0, 0, 0x8000, 0x7fff);
}
else
#endif

u32 alpha68k_state::screen_update_alpha68k_V(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: should be an user selectable feature instead of using the MCU ID, it's also repeated below.
	bool is_skyadventure = (m_microcontroller_id == 0x8814);
	const u16 flipxmask = is_skyadventure ? 0 : 0x8000;
	const u16 flipymask = is_skyadventure ? 0x8000 : 0;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	
	bitmap.fill(4095, cliprect);

	draw_sprites_V(bitmap, cliprect, 0, 0x07c0, 0x0800, flipxmask, flipymask, 0x7fff);
	draw_sprites_V(bitmap, cliprect, 1, 0x0000, 0x0800, flipxmask, flipymask, 0x7fff);
	draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x0800, flipxmask, flipymask, 0x7fff);
	draw_sprites_V(bitmap, cliprect, 0, 0x0000, 0x07c0, flipxmask, flipymask, 0x7fff);

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

u32 alpha68k_state::screen_update_alpha68k_V_sb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(4095, cliprect);

	draw_sprites_V(bitmap, cliprect, 0, 0x07c0, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(bitmap, cliprect, 1, 0x0000, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(bitmap, cliprect, 2, 0x0000, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(bitmap, cliprect, 0, 0x0000, 0x07c0, 0x4000, 0x8000, 0x3fff);

	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
