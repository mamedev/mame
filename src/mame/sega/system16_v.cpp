// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Phil Stroffolino, Mirko Buffoni
/***************************************************************************

    System 16 / 18 bootleg video

    Bugs to check against real HW

    System16A Tilemap bootlegs

    - Shinobi (Datsu bootleg) has a black bar down the right hand side,
      which turns red on the high score table.  This is in the Text layer.
      According to other games this should be the correct alignment for
      this bootleg HW.

    - After inserting a coin in Passing Shot (2p version) the layers are
      misaligned by 1 pixel (happens on non-bootlegs too)

    - Causing a 'fault' in the Passing Shot 2p bootleg (not hitting the ball
      on your serve) causes the tilemaps to be erased and not updated
      properly (mirroring?, bootleg protection?, missed case in bootleg?)

    System18 Tilemap bootlegs

    - Alien Storm Credit text for Player 1 is not displayed after inserting
      a coin.

***************************************************************************/
#include "emu.h"
#include "system16.h"
#include "video/resnet.h"
#include "segaic16.h"


void segas1x_bootleg_state::setup_system16_bootleg_spritebanking(  )
{
	if (m_spritebank_type == 0)
	{
		static constexpr uint8_t alternate_banklist[] = { 0,255,255,255, 255,255,255,3, 255,255,255,2, 255,1,0,255 };
		for (int i = 0; i < 16; i++)
			m_sprites->set_bank(i, alternate_banklist[i]);

	}

}


#define MAXCOLOURS 0x2000 /* 8192 */


/***************************************************************************/

TILEMAP_MAPPER_MEMBER(segas1x_bootleg_state::sys16_bg_map)
{
	int page = 0;
	if (row < 32)
	{
		/* top */
		if (col < 64)
			page = 0;
		else
			page = 1;
	}
	else
	{
		/* bottom */
		if (col < 64)
			page = 2;
		else
			page = 3;
	}

	row = row % 32;
	col = col % 64;
	return page * 64 * 32 + row * 64 + col;
}

TILEMAP_MAPPER_MEMBER(segas1x_bootleg_state::sys16_text_map)
{
	return row * 64 + col + (64 - 40);
}


/*
    Color generation details

    Each color is made up of 5 bits, connected through one or more resistors like so:

    Bit 0 = 1 x 3.9K ohm
    Bit 1 = 1 x 2.0K ohm
    Bit 2 = 1 x 1.0K ohm
    Bit 3 = 2 x 1.0K ohm
    Bit 4 = 4 x 1.0K ohm

    Another data bit is connected by a tristate buffer to the color output through a 470 ohm resistor.
    The buffer allows the resistor to have no effect (tristate), halve brightness (pull-down) or double brightness (pull-up).
    The data bit source is a PPI pin in some of the earlier hardware (Hang-On, Pre-System 16) or bit 15 of each
    color RAM entry (Space Harrier, System 16B and most later boards).
*/

static const int resistances_normal[6] = {3900, 2000, 1000, 1000/2, 1000/4, 0};
static const int resistances_sh[6] = {3900, 2000, 1000, 1000/2, 1000/4, 470};

#ifdef UNUSED_CODE
void segas1x_bootleg_state::sys16_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t newword;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	newword = m_generic_paletteram_16[offset];

	/*  sBGR BBBB GGGG RRRR */
	/*  x000 4321 4321 4321 */
	{
		int r, g, b, rs, gs, bs/*, rh, gh, bh*/;
		int r0 = (newword >> 12) & 1;
		int r1 = (newword >>  0) & 1;
		int r2 = (newword >>  1) & 1;
		int r3 = (newword >>  2) & 1;
		int r4 = (newword >>  3) & 1;
		int g0 = (newword >> 13) & 1;
		int g1 = (newword >>  4) & 1;
		int g2 = (newword >>  5) & 1;
		int g3 = (newword >>  6) & 1;
		int g4 = (newword >>  7) & 1;
		int b0 = (newword >> 14) & 1;
		int b1 = (newword >>  8) & 1;
		int b2 = (newword >>  9) & 1;
		int b3 = (newword >> 10) & 1;
		int b4 = (newword >> 11) & 1;

		/* Normal colors */
		r = combine_weights(m_weights[0][0], r0, r1, r2, r3, r4, 0);
		g = combine_weights(m_weights[0][1], g0, g1, g2, g3, g4, 0);
		b = combine_weights(m_weights[0][2], b0, b1, b2, b3, b4, 0);

		/* Shadow colors */
		rs = combine_weights(m_weights[1][0], r0, r1, r2, r3, r4, 0);
		gs = combine_weights(m_weights[1][1], g0, g1, g2, g3, g4, 0);
		bs = combine_weights(m_weights[1][2], b0, b1, b2, b3, b4, 0);

		/* Highlight colors */
		//rh = combine_weights(m_weights[1][0], r0, r1, r2, r3, r4, 1);
		//gh = combine_weights(m_weights[1][1], g0, g1, g2, g3, g4, 1);
		//bh = combine_weights(m_weights[1][2], b0, b1, b2, b3, b4, 1);

		m_palette->set_pen_color(offset, rgb_t(r, g, b) );

		m_palette->set_pen_color(offset + m_palette->entries()/2, rgb_t(rs,gs,bs));
	}
}
#endif

void segas1x_bootleg_state::update_page(  )
{
	int all_dirty = 0;

	if (m_old_tile_bank[1] != m_tile_bank[1])
	{
		all_dirty = 1;
		m_old_tile_bank[1] = m_tile_bank[1];
	}

	if (m_old_tile_bank[0] != m_tile_bank[0])
	{
		all_dirty = 1;
		m_old_tile_bank[0] = m_tile_bank[0];
		m_text_layer->mark_all_dirty();
	}

	if (all_dirty)
	{
		m_background[0]->mark_all_dirty();
		m_foreground[0]->mark_all_dirty();

		if (m_system18)
		{
			m_background[1]->mark_all_dirty();
			m_foreground[1]->mark_all_dirty();
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			int page0 = 64 * 32 * i;
			if (m_old_bg_page[0][i] != m_bg_page[0][i])
			{
				m_old_bg_page[0][i] = m_bg_page[0][i];
				for (int offset = page0; offset < page0 + 64 * 32; offset++)
					m_background[0]->mark_tile_dirty(offset);
			}

			if (m_old_fg_page[0][i] != m_fg_page[0][i])
			{
				m_old_fg_page[0][i] = m_fg_page[0][i];
				for (int offset = page0; offset < page0 + 64 * 32; offset++)
					m_foreground[0]->mark_tile_dirty(offset);
			}

			if (m_system18)
			{
				if (m_old_bg_page[1][i] != m_bg_page[1][i])
				{
					m_old_bg_page[1][i] = m_bg_page[1][i];
					for (int offset = page0; offset < page0 + 64 * 32; offset++)
						m_background[1]->mark_tile_dirty(offset);
				}

				if (m_old_fg_page[1][i] != m_fg_page[1][i])
				{
					m_old_fg_page[1][i] = m_fg_page[1][i];
					for (int offset = page0; offset < page0 + 64 * 32; offset++)
						m_foreground[1]->mark_tile_dirty(offset);
				}
			}
		}
	}
}

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_bg_tile_info)
{
	uint16_t const *const source = &m_tileram[m_bg_page[0][tile_index >> 11] << 11];
	int data = source[tile_index & 0x7ff];
	int tile_number = (data & 0xfff) | (((data & m_tilebank_switch) ? m_tile_bank[1] : m_tile_bank[0]) << 12);

	tileinfo.set(0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_fg_tile_info)
{
	uint16_t const *const source = &m_tileram[m_fg_page[0][tile_index >> 11] << 11];
	int data = source[tile_index & 0x7ff];
	int tile_number = (data & 0xfff) | (((data & m_tilebank_switch) ? m_tile_bank[1] : m_tile_bank[0]) << 12);

	tileinfo.set(0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_bg2_tile_info)
{
	uint16_t const *const source = &m_tileram[m_bg_page[1][tile_index >> 11] << 11];
	int data = source[tile_index & 0x7ff];
	int tile_number = (data & 0xfff) | (m_tile_bank[(data & 0x1000) >> 12] << 12);

	tileinfo.set(0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_fg2_tile_info)
{
	uint16_t const *const source = &m_tileram[m_fg_page[1][tile_index >> 11] << 11];
	int data = source[tile_index & 0x7ff];
	int tile_number = (data & 0xfff) | (m_tile_bank[(data & 0x1000) >> 12] << 12);

	tileinfo.set(0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

void segas1x_bootleg_state::sys16_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldword = m_tileram[offset];

	COMBINE_DATA(&m_tileram[offset]);

	if (oldword != m_tileram[offset])
	{
		int page = offset >> 11;
		offset = offset & 0x7ff;

		if (m_bg_page[0][0] == page) m_background[0]->mark_tile_dirty(offset);
		if (m_bg_page[0][1] == page) m_background[0]->mark_tile_dirty(offset | (1 << 11));
		if (m_bg_page[0][2] == page) m_background[0]->mark_tile_dirty(offset | (2 << 11));
		if (m_bg_page[0][3] == page) m_background[0]->mark_tile_dirty(offset | (3 << 11));

		if (m_fg_page[0][0] == page) m_foreground[0]->mark_tile_dirty(offset);
		if (m_fg_page[0][1] == page) m_foreground[0]->mark_tile_dirty(offset | (1 << 11));
		if (m_fg_page[0][2] == page) m_foreground[0]->mark_tile_dirty(offset | (2 << 11));
		if (m_fg_page[0][3] == page) m_foreground[0]->mark_tile_dirty(offset | (3 << 11));

		if (m_system18)
		{
			if (m_bg_page[1][0] == page) m_background[1]->mark_tile_dirty(offset);
			if (m_bg_page[1][1] == page) m_background[1]->mark_tile_dirty(offset | (1 << 11));
			if (m_bg_page[1][2] == page) m_background[1]->mark_tile_dirty(offset | (2 << 11));
			if (m_bg_page[1][3] == page) m_background[1]->mark_tile_dirty(offset | (3 << 11));

			if (m_fg_page[1][0] == page) m_foreground[1]->mark_tile_dirty(offset);
			if (m_fg_page[1][1] == page) m_foreground[1]->mark_tile_dirty(offset | (1 << 11));
			if (m_fg_page[1][2] == page) m_foreground[1]->mark_tile_dirty(offset | (2 << 11));
			if (m_fg_page[1][3] == page) m_foreground[1]->mark_tile_dirty(offset | (3 << 11));
		}
	}
}

/***************************************************************************/

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_text_tile_info)
{
	uint16_t const *const source = m_textram;
	int tile_number = source[tile_index];
	int pri = tile_number >> 8;

	if (!m_shinobl_kludge)
	{
		tileinfo.set(0,
				(tile_number & 0x1ff) | (m_tile_bank[0] << 12),
				(tile_number >> 9) % 8,
				0);
	}
	else
	{
		tileinfo.set(0,
				(tile_number & 0xff) | (m_tile_bank[0] << 12),
				(tile_number >> 8) % 8,
				0);
	}

	if (pri >= m_textlayer_lo_min && pri <= m_textlayer_lo_max)
		tileinfo.category = 1;
	if (pri >= m_textlayer_hi_min && pri <= m_textlayer_hi_max)
		tileinfo.category = 0;
}

void segas1x_bootleg_state::sys16_textram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

/***************************************************************************/

VIDEO_START_MEMBER(segas1x_bootleg_state,system16)
{
	/* Normal colors */
	compute_resistor_weights(0, 255, -1.0,
			6, resistances_normal, m_weights[0][0], 0, 0,
			6, resistances_normal, m_weights[0][1], 0, 0,
			6, resistances_normal, m_weights[0][2], 0, 0);

	/* Shadow/Highlight colors */
	compute_resistor_weights(0, 255, -1.0,
			6, resistances_sh, m_weights[1][0], 0, 0,
			6, resistances_sh, m_weights[1][1], 0, 0,
			6, resistances_sh, m_weights[1][2], 0, 0);

	if (!m_bg1_trans)
		m_background[0] = &machine().tilemap().create(
				*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(segas1x_bootleg_state::sys16_bg_map)),
				8,8,
				64*2,32*2);
	else
		m_background[0] = &machine().tilemap().create(
				*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(segas1x_bootleg_state::sys16_bg_map)),
				8,8,
				64*2,32*2);

	m_foreground[0] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(segas1x_bootleg_state::sys16_bg_map)),
			8,8,
			64*2,32*2);

	m_text_layer = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_text_tile_info)), tilemap_mapper_delegate(*this, FUNC(segas1x_bootleg_state::sys16_text_map)),
			8,8,
			40,28);

	if (m_bg1_trans) m_background[0]->set_transparent_pen(0);
	m_foreground[0]->set_transparent_pen(0);
	m_text_layer->set_transparent_pen(0);

	m_tile_bank[0] = 0;
	m_tile_bank[1] = 1;

	m_fg_scrollx = 0;
	m_fg_scrolly = 0;

	m_bg_scrollx = 0;
	m_bg_scrolly = 0;

	m_refreshenable = 1;

	for (int i = 0; i < 2; i++)
	{
		std::fill(std::begin(m_bg_page[i]), std::end(m_bg_page[i]), 0);
		std::fill(std::begin(m_fg_page[i]), std::end(m_fg_page[i]), 0);
	}

	/* common defaults */
	m_tilebank_switch = 0x1000;

	// Defaults for sys16 games
	m_textlayer_lo_min = 0;
	m_textlayer_lo_max = 0x7f;
	m_textlayer_hi_min = 0x80;
	m_textlayer_hi_max = 0xff;

	m_system18 = 0;

	if (m_sprites.found())
		setup_system16_bootleg_spritebanking();
}

VIDEO_START_MEMBER(segas1x_bootleg_state,system18old)
{
	VIDEO_START_CALL_MEMBER(system16);

	m_bg1_trans = 1;

	m_background[1] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_bg2_tile_info)), tilemap_mapper_delegate(*this, FUNC(segas1x_bootleg_state::sys16_bg_map)),
			8,8,
			64*2,32*2);

	m_foreground[1] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_fg2_tile_info)), tilemap_mapper_delegate(*this, FUNC(segas1x_bootleg_state::sys16_bg_map)),
			8,8,
			64*2,32*2);

	m_foreground[1]->set_transparent_pen(0);

	if (m_splittab_fg_x)
	{
		m_foreground[0] ->set_scroll_rows(64);
		m_foreground[1] ->set_scroll_rows(64);
	}

	if (m_splittab_bg_x)
	{
		m_background[0] ->set_scroll_rows(64);
		m_background[1] ->set_scroll_rows(64);
	}

	m_textlayer_lo_min = 0;
	m_textlayer_lo_max = 0x1f;
	m_textlayer_hi_min = 0x20;
	m_textlayer_hi_max = 0xff;

	m_system18 = 1;
}


/*****************************************************************************************
 System 16A bootleg video

 The System16A bootlegs have extra RAM for 2 tilemaps.  The game code copies data from
 the usual 'tilemap ram' area to this new RAM and sets scroll registers as appropriate
 using additional registers not present on the original System 16A hardware.

 For some unknown reason the 2p Passing Shot bootleg end up blanking this area at times,
 this could be an emulation flaw or a problem with the original bootlegs.  Inserting a
 coin at the incorrect time can also cause missing graphics on the initial entry screen.
 See note at top of driver

 Sprites:
  ToDo

*****************************************************************************************/


TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_s16a_bootleg_tile_infotxt)
{
	int data, tile_number;

	data = m_textram[tile_index];
	tile_number = data & 0x1ff;

	tileinfo.set(0,
			tile_number,
			((data >> 9) & 0x7),
			0);
}

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_s16a_bootleg_tile_info0)
{
	int data, tile_number;

	data = m_bg0_tileram[tile_index];
	tile_number = data & 0x1fff;


	tileinfo.set(0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

TILE_GET_INFO_MEMBER(segas1x_bootleg_state::get_s16a_bootleg_tile_info1)
{
	int data, tile_number;

	data = m_bg1_tileram[tile_index];
	tile_number = data & 0x1fff;

	tileinfo.set(0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

void segas1x_bootleg_state::s16a_bootleg_bgscrolly_w(uint16_t data)
{
	m_bg_scrolly = data;
}

void segas1x_bootleg_state::s16a_bootleg_bgscrollx_w(uint16_t data)
{
	m_bg_scrollx = data;
}

void segas1x_bootleg_state::s16a_bootleg_fgscrolly_w(uint16_t data)
{
	m_fg_scrolly = data;
}

void segas1x_bootleg_state::s16a_bootleg_fgscrollx_w(uint16_t data)
{
	m_fg_scrollx = data;
}

void segas1x_bootleg_state::s16a_bootleg_tilemapselect_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tilemapselect);
	//printf("system16 bootleg tilemapselect %04x\n", m_tilemapselect);
}


VIDEO_START_MEMBER(segas1x_bootleg_state,s16a_bootleg)
{
	/* Normal colors */
	compute_resistor_weights(0, 255, -1.0,
			6, resistances_normal, m_weights[0][0], 0, 0,
			6, resistances_normal, m_weights[0][1], 0, 0,
			6, resistances_normal, m_weights[0][2], 0, 0
			);

	/* Shadow/Highlight colors */
	compute_resistor_weights(0, 255, -1.0,
			6, resistances_sh, m_weights[1][0], 0, 0,
			6, resistances_sh, m_weights[1][1], 0, 0,
			6, resistances_sh, m_weights[1][2], 0, 0
			);



	m_text_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_s16a_bootleg_tile_infotxt)), TILEMAP_SCAN_ROWS, 8,8, 64,32);

	// the system16a bootlegs have simple tilemaps instead of the paged system
	m_bg_tilemaps[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_s16a_bootleg_tile_info0)), TILEMAP_SCAN_ROWS, 8,8, 64,32);
	m_bg_tilemaps[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(segas1x_bootleg_state::get_s16a_bootleg_tile_info1)), TILEMAP_SCAN_ROWS, 8,8, 64,32);

	m_text_tilemap->set_transparent_pen(0);
	m_bg_tilemaps[0]->set_transparent_pen(0);
	m_bg_tilemaps[1]->set_transparent_pen(0);
}

VIDEO_START_MEMBER(segas1x_bootleg_state,s16a_bootleg_wb3bl)
{
	VIDEO_START_CALL_MEMBER(s16a_bootleg);
	setup_system16_bootleg_spritebanking();
}

VIDEO_START_MEMBER(segas1x_bootleg_state,s16a_bootleg_shinobi)
{
	VIDEO_START_CALL_MEMBER(s16a_bootleg);
	setup_system16_bootleg_spritebanking();
}

VIDEO_START_MEMBER(segas1x_bootleg_state,s16a_bootleg_passsht)
{
	VIDEO_START_CALL_MEMBER(s16a_bootleg);
	setup_system16_bootleg_spritebanking();
}

// Passing Shot (2 player), Shinobi (Datsu), Wonderboy 3
uint32_t segas1x_bootleg_state::screen_update_s16a_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// passing shot
	int offset_txtx = 192;
	int offset_txty = 0;
	int offset_bg1x = 190;
	int offset_bg1y = 0;
	int offset_bg0x = 187;
	int offset_bg0y = 0;

	bitmap.fill(m_palette->black_pen(), cliprect);

	// start the sprites drawing
	m_sprites->draw_async(cliprect);

	// I can't bring myself to care about dirty tile marking on something which runs at a bazillion % speed anyway, clean code is better
	m_bg_tilemaps[0]->mark_all_dirty();
	m_bg_tilemaps[1]->mark_all_dirty();
	m_text_tilemap->mark_all_dirty();

	m_text_tilemap->set_scrollx(0, offset_txtx);
	m_text_tilemap->set_scrolly(0, offset_txty);

	if ((m_tilemapselect & 0xff) == 0x12)
	{
		m_bg_tilemaps[1]->set_scrollx(0, m_bg_scrollx + offset_bg1x);
		m_bg_tilemaps[1]->set_scrolly(0, m_bg_scrolly + offset_bg1y + m_back_yscroll);
		m_bg_tilemaps[0]->set_scrollx(0, m_fg_scrollx + offset_bg0x);
		m_bg_tilemaps[0]->set_scrolly(0, m_fg_scrolly + offset_bg0y + m_fore_yscroll);

		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0, 0);

		m_text_tilemap->set_scrolly(0, m_text_yscroll);

		m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else if ((m_tilemapselect & 0xff) == 0x21)
	{
		m_bg_tilemaps[0]->set_scrollx(0, m_bg_scrollx + 187 );
		m_bg_tilemaps[0]->set_scrolly(0, m_bg_scrolly + m_back_yscroll );
		m_bg_tilemaps[1]->set_scrollx(0, m_fg_scrollx + 187 );
		m_bg_tilemaps[1]->set_scrolly(0, m_fg_scrolly + 1 + m_fore_yscroll );

		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, 0, 0);

		m_text_tilemap->set_scrolly(0, m_text_yscroll);

		m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	m_sprites->iterate_dirty_rects(
			cliprect,
			[this, &bitmap, &sprites] (rectangle const &rect)
			{
				for (int y = rect.min_y; y <= rect.max_y; y++)
				{
					uint16_t *const dest = &bitmap.pix(y);
					uint16_t const *const src = &sprites.pix(y);
					//uint8_t const *const pri = &screen.priority().pix(y);
					for (int x = rect.min_x; x <= rect.max_x; x++)
					{
						// only process written pixels
						uint16_t const pix = src[x];
						if (pix != 0xffff)
						{
							// compare sprite priority against tilemap priority
							//int const priority = (pix >> 12) & 3;
							if (1)
							{
								// if the color is set to maximum, shadow pixels underneath us
								if ((pix & 0x03f0) == 0x03f0)
									dest[x] += m_palette_entries;

								// otherwise, just add in sprite palette base
								else
									dest[x] = 0x400 | (pix & 0x3ff);
							}
						}
					}
				}
			});

	return 0;
}

/* The Passing Shot 4 Player bootleg has weird scroll registers (different offsets, ^0x7 xor) */
uint32_t segas1x_bootleg_state::screen_update_s16a_bootleg_passht4b(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// passing shot
	int offset_txtx = 192;
	int offset_txty = 0;
	int offset_bg1x = 3;
	int offset_bg1y = 32;
	int offset_bg0x = 5;
	int offset_bg0y = 32;

	bitmap.fill(m_palette->black_pen(), cliprect);

	// start the sprites drawing
	m_sprites->draw_async(cliprect);

	// I can't bring myself to care about dirty tile marking on something which runs at a bazillion % speed anyway, clean code is better
	m_bg_tilemaps[0]->mark_all_dirty();
	m_bg_tilemaps[1]->mark_all_dirty();
	m_text_tilemap->mark_all_dirty();

	m_text_tilemap->set_scrollx(0, offset_txtx);
	m_text_tilemap->set_scrolly(0, offset_txty);

	if ((m_tilemapselect & 0xff) == 0x12)
	{
		m_bg_tilemaps[1]->set_scrollx(0, (m_bg_scrollx ^ 0x7) + offset_bg1x);
		m_bg_tilemaps[1]->set_scrolly(0, m_bg_scrolly + offset_bg1y);
		m_bg_tilemaps[0]->set_scrollx(0, (m_fg_scrollx ^ 0x7) + offset_bg0x);
		m_bg_tilemaps[0]->set_scrolly(0, m_fg_scrolly + offset_bg0y);

		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	m_sprites->iterate_dirty_rects(
			cliprect,
			[this, &bitmap, &sprites] (rectangle const &rect)
			{
				for (int y = rect.min_y; y <= rect.max_y; y++)
				{
					uint16_t *const dest = &bitmap.pix(y);
					uint16_t const *const src = &sprites.pix(y);
					//uint8_t const *const pri = &screen.priority().pix(y);
					for (int x = rect.min_x; x <= rect.max_x; x++)
					{
						// only process written pixels
						uint16_t const pix = src[x];
						if (pix != 0xffff)
						{
							// compare sprite priority against tilemap priority
							//int const priority = (pix >> 12) & 3;
							if (1)
							{
								// if the color is set to maximum, shadow pixels underneath us
								if ((pix & 0x03f0) == 0x03f0)
									dest[x] += m_palette_entries;

								// otherwise, just add in sprite palette base
								else
									dest[x] = 0x400 | (pix & 0x3ff);
							}
						}
					}
				}
			});


	return 0;
}


/***************************************************************************/

uint32_t segas1x_bootleg_state::screen_update_system16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_refreshenable)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	// start the sprites drawing
	if (m_sprites.found())
		m_sprites->draw_async(cliprect);

	update_page();

	screen.priority().fill(0, cliprect);

	m_background[0]->set_scrollx(0, -320 - m_bg_scrollx);
	m_background[0]->set_scrolly(0, -256 + m_bg_scrolly + m_back_yscroll);
	m_foreground[0]->set_scrollx(0, -320 - m_fg_scrollx);
	m_foreground[0]->set_scrolly(0, -256 + m_fg_scrolly + m_fore_yscroll);

	m_text_layer->set_scrollx(0, 0);
	m_text_layer->set_scrolly(0, 0 + m_text_yscroll);

	/* Background */
	m_background[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0x00);

	/* Foreground */
	m_foreground[0]->draw(screen, bitmap, cliprect, 0, 0x03);
	m_foreground[0]->draw(screen, bitmap, cliprect, 1, 0x07);


	/* Text Layer */
	if (m_textlayer_lo_max != 0)
	{
		m_text_layer->draw(screen, bitmap, cliprect, 1, 7);// needed for Body Slam
	}

	m_text_layer->draw(screen, bitmap, cliprect, 0, 0xf);

	//draw_sprites(machine(), bitmap, cliprect,0);


	// mix in sprites
	if (!m_sprites)
		return 0;
	bitmap_ind16 &sprites = m_sprites->bitmap();
	m_sprites->iterate_dirty_rects(
			cliprect,
			[this, &bitmap, &sprites] (rectangle const &rect)
			{
				for (int y = rect.min_y; y <= rect.max_y; y++)
				{
					uint16_t *const dest = &bitmap.pix(y);
					uint16_t const *const src = &sprites.pix(y);
					//uint8_t const *const pri = &screen.priority().pix(y);
					for (int x = rect.min_x; x <= rect.max_x; x++)
					{
						// only process written pixels
						uint16_t const pix = src[x];
						if (pix != 0xffff)
						{
							// compare sprite priority against tilemap priority
							//int const priority = (pix >> 12) & 3;
							if (1)
							{
								// if the color is set to maximum, shadow pixels underneath us
								if ((pix & 0x03f0) == 0x03f0)
									dest[x] += m_palette_entries;

								// otherwise, just add in sprite palette base
								else
									dest[x] = 0x400 | (pix & 0x3ff);
							}
						}
					}
				}
			});

	return 0;
}


uint32_t segas1x_bootleg_state::screen_update_system18old(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_refreshenable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	// start the sprites drawing
	m_sprites->draw_async(cliprect);

	update_page();

	screen.priority().fill(0);

	bitmap.fill(0, cliprect);

	m_background[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_background[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 0);   //??
	m_background[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 2, 0);   //??
	m_background[0]->draw(screen, bitmap, cliprect, 1, 0x1);
	m_background[0]->draw(screen, bitmap, cliprect, 2, 0x3);

	m_foreground[0]->draw(screen, bitmap, cliprect, 0, 0x3);
	m_foreground[0]->draw(screen, bitmap, cliprect, 1, 0x7);

	m_text_layer->draw(screen, bitmap, cliprect, 1, 0x7);
	m_text_layer->draw(screen, bitmap, cliprect, 0, 0xf);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	m_sprites->iterate_dirty_rects(
			cliprect,
			[this, &bitmap, &sprites] (rectangle const &rect)
			{
				for (int y = rect.min_y; y <= rect.max_y; y++)
				{
					uint16_t *const dest = &bitmap.pix(y);
					uint16_t const *const src = &sprites.pix(y);
					//uint8_t const *const pri = &screen.priority().pix(y);
					for (int x = rect.min_x; x <= rect.max_x; x++)
					{
						// only process written pixels
						uint16_t const pix = src[x];
						if (pix != 0xffff)
						{
							// compare sprite priority against tilemap priority
							//int const priority = (pix >> 12) & 3;
							if (1)
							{
								// if the color is set to maximum, shadow pixels underneath us
								if ((pix & 0x03f0) == 0x03f0)
									dest[x] += m_palette_entries;

								// otherwise, just add in sprite palette base
								else
									dest[x] = 0x400 | (pix & 0x3ff);
							}
						}
					}
				}
			});

	return 0;
}
