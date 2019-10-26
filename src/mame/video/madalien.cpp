// license:GPL-2.0+
// copyright-holders:Norbert Kehrer
/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "emu.h"
#include "includes/madalien.h"
#include "video/mc6845.h"
#include "screen.h"


#define PIXEL_CLOCK (MADALIEN_MAIN_CLOCK / 2)


void madalien_state::madalien_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; i++)
	{
		int r = 0;
		int g = 0;
		int b = 0;

		if (BIT(color_prom[i], 0))
			r += 0x3f;
		if (BIT(color_prom[i], 1))
			r += 0xc0;
		if (BIT(color_prom[i], 2))
			g += 0x3f;
		if (BIT(color_prom[i], 3))
			g += 0xc0;
		if (BIT(color_prom[i], 4))
			b += 0x3f;
		if (BIT(color_prom[i], 5))
			b += 0xc0;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	for (int i = 0x10; i < 0x20; i++)
	{
		uint8_t ctabentry = i - 0x10;

		if (BIT((i - 0x10), 1))
			ctabentry = ctabentry ^ 0x06;

		if (BIT((i - 0x10), 2))
			ctabentry = ctabentry ^ 0x06;

		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x20; i < 0x30; i++)
		palette.set_pen_indirect(i, (i - 0x20) | 0x10);
}


inline int madalien_state::scan_helper(int col, int row, int section)
{
	return (section << 8) | ((~col & 0x0f) << 3) | row;
}


TILEMAP_MAPPER_MEMBER(madalien_state::scan_mode0)
{
	return scan_helper(col, row, 0);
}
TILEMAP_MAPPER_MEMBER(madalien_state::scan_mode1)
{
	return scan_helper(col, row, 1);
}
TILEMAP_MAPPER_MEMBER(madalien_state::scan_mode2)
{
	return scan_helper(col, row, BIT(col, 4) ? 1 : 0);
}
TILEMAP_MAPPER_MEMBER(madalien_state::scan_mode3)
{
	return scan_helper(col, row, BIT(col, 4) ? 0 : 1);
}


TILE_GET_INFO_MEMBER(madalien_state::get_tile_info_BG_1)
{
	uint8_t *map = memregion("user1")->base() + ((*m_video_flags & 0x08) << 6);

	SET_TILE_INFO_MEMBER(1, map[tile_index], BIT(*m_video_flags, 2) ? 2 : 0, 0);
}


TILE_GET_INFO_MEMBER(madalien_state::get_tile_info_BG_2)
{
	uint8_t *map = memregion("user1")->base() + ((*m_video_flags & 0x08) << 6) + 0x80;

	SET_TILE_INFO_MEMBER(1, map[tile_index], BIT(*m_video_flags, 2) ? 2 : 0, 0);
}


TILE_GET_INFO_MEMBER(madalien_state::get_tile_info_FG)
{
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index], 0, 0);
}

WRITE8_MEMBER(madalien_state::madalien_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap_fg->mark_tile_dirty(offset);
}


void madalien_state::video_start()
{
	// can't make this static or it will keep stale pointers after a hard reset
	const tilemap_mapper_delegate scan_functions[4] =
	{
		tilemap_mapper_delegate(*this, FUNC(madalien_state::scan_mode0)),
		tilemap_mapper_delegate(*this, FUNC(madalien_state::scan_mode1)),
		tilemap_mapper_delegate(*this, FUNC(madalien_state::scan_mode2)),
		tilemap_mapper_delegate(*this, FUNC(madalien_state::scan_mode3))
	};

	static constexpr int tilemap_cols[4] = { 16, 16, 32, 32 };

	m_tilemap_fg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(madalien_state::get_tile_info_FG)), TILEMAP_SCAN_COLS_FLIP_X, 8, 8, 32, 32);
	m_tilemap_fg->set_transparent_pen(0);

	for (int i = 0; i < 4; i++)
	{
		m_tilemap_edge1[i] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(madalien_state::get_tile_info_BG_1)), scan_functions[i], 16, 16, tilemap_cols[i], 8);
		m_tilemap_edge2[i] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(madalien_state::get_tile_info_BG_2)), scan_functions[i], 16, 16, tilemap_cols[i], 8);
	}

	m_headlight_bitmap = std::make_unique<bitmap_ind16>(128, 128);

	m_gfxdecode->gfx(0)->set_source(m_charram);

	m_gfxdecode->gfx(2)->opaque(*m_headlight_bitmap,m_headlight_bitmap->cliprect(), 0, 0, 0, 0, 0x00, 0x00);
	m_gfxdecode->gfx(2)->opaque(*m_headlight_bitmap,m_headlight_bitmap->cliprect(), 0, 0, 0, 1, 0x00, 0x40);
}


void madalien_state::draw_edges(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, int scroll_mode)
{
	rectangle clip_edge1(cliprect);
	rectangle clip_edge2(cliprect);

	if (flip)
	{
		clip_edge1.min_y = *m_edge1_pos | 0x80;
		clip_edge2.max_y = (*m_edge2_pos & 0x7f) ^ 0x7f;
	}
	else
	{
		clip_edge1.max_y = (*m_edge1_pos & 0x7f) ^ 0x7f;
		clip_edge2.min_y = *m_edge2_pos | 0x80;
	}

	clip_edge1 &= cliprect;
	clip_edge2 &= cliprect;

	m_tilemap_edge1[scroll_mode]->mark_all_dirty();
	m_tilemap_edge2[scroll_mode]->mark_all_dirty();

	m_tilemap_edge1[scroll_mode]->set_flip(flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap_edge1[scroll_mode]->set_scrollx(0, -(*m_scroll & 0xfc));
	m_tilemap_edge1[scroll_mode]->set_scrolly(0, *m_edge1_pos & 0x7f);

	m_tilemap_edge2[scroll_mode]->set_flip(flip ? TILEMAP_FLIPX : TILEMAP_FLIPY);
	m_tilemap_edge2[scroll_mode]->set_scrollx(0, -(*m_scroll & 0xfc));
	m_tilemap_edge2[scroll_mode]->set_scrolly(0, *m_edge2_pos & 0x7f);

	m_tilemap_edge1[scroll_mode]->draw(screen, bitmap, clip_edge1, 0, 0);
	m_tilemap_edge2[scroll_mode]->draw(screen, bitmap, clip_edge2, 0, 0);
}


void madalien_state::draw_headlight(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	if (BIT(*m_video_flags, 0))
	{
		uint8_t y;

		for (y = 0; y < 0x80; y++)
		{
			uint8_t x;
			uint8_t hy = y - *m_headlight_pos;

			if (flip)
				hy = ~hy;

			if ((hy < cliprect.top()) || (hy > cliprect.bottom()))
				continue;

			for (x = 0; x < 0x80; x++)
			{
				uint8_t hx = x;

				if (flip)
					hx = ~hx;

				if ((hx < cliprect.left()) || (hx > cliprect.right()))
					continue;

				if (m_headlight_bitmap->pix16(y, x) != 0)
					bitmap.pix16(hy, hx) |= 8;
			}
		}
	}
}


void madalien_state::draw_foreground(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	m_tilemap_fg->set_flip(flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 0);
}


WRITE8_MEMBER(madalien_state::madalien_charram_w)
{
	m_charram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty((offset/8) & 0xff);
}


uint32_t madalien_state::screen_update_madalien(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = BIT(ioport("DSW")->read(), 6) && BIT(*m_video_control, 0);

	// bits #0 and #1 define scrolling mode
	//
	// mode 0 - cycle over map section A
	// mode 1 - cycle over map section B
	//
	// mode 2 - transition from B to A
	// mode 3 - transition from A to B
	int scroll_mode = *m_scroll & 3;

	bitmap.fill(0, cliprect);
	draw_edges(screen, bitmap, cliprect, flip, scroll_mode);
	draw_foreground(screen, bitmap, cliprect, flip);

	/* highlight section A (outside of tunnels).
	 * also, bit 1 of the video_flags register (6A) is
	 * combined with the headlight signal through NOR gate 1A,
	 * which is used to light up the tunnel when an alien explodes
	*/
	if (scroll_mode != 1 || *m_video_flags & 2)
	{
		int x;
		int y;

		int min_x = 0;
		int max_x = 0xff;

		if (!(*m_video_flags & 2))
		{
			if (scroll_mode == 2) min_x = (*m_scroll & 0xfc);
			else if (scroll_mode == 3) max_x = (*m_scroll & 0xfc) - 1;
		}

		if (flip)
		{
			int max_x_save = max_x;
			max_x = 0xff - min_x;
			min_x = 0xff - max_x_save;
		}

		for (y = cliprect.top(); y <= cliprect.bottom(); y++)
			for (x = min_x; x <= max_x; x++)
				if ((x >= cliprect.left()) && (x <= cliprect.right()))
					bitmap.pix16(y, x) |= 8;
	}

	draw_headlight(bitmap, cliprect, flip);

	return 0;
}


static const uint32_t headlight_xoffset[] =
{
	STEP8(0x78, 1),
	STEP8(0x70, 1),
	STEP8(0x68, 1),
	STEP8(0x60, 1),
	STEP8(0x58, 1),
	STEP8(0x50, 1),
	STEP8(0x48, 1),
	STEP8(0x40, 1),
	STEP8(0x38, 1),
	STEP8(0x30, 1),
	STEP8(0x28, 1),
	STEP8(0x20, 1),
	STEP8(0x18, 1),
	STEP8(0x10, 1),
	STEP8(0x08, 1),
	STEP8(0x00, 1),
};

static const uint32_t headlight_yoffset[] =
{
	STEP32(0x0000, 0x80), STEP32(0x1000, 0x80)
};

static const gfx_layout headlightlayout =
{
	128, 64,
	1,
	1,
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x2000,
	headlight_xoffset,
	headlight_yoffset
};

static const gfx_layout charlayout =
{
	8, 8,
	256,
	3,
	{ 2*0x4000, 1*0x4000, 0*0x4000 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	64
};

static const gfx_layout tilelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{
		0x30*8+4, 0x30*8+5, 0x30*8+6, 0x30*8+7,
		0x20*8+4, 0x20*8+5, 0x20*8+6, 0x20*8+7,
		0x10*8+4, 0x10*8+5, 0x10*8+6, 0x10*8+7,
		0x00*8+4, 0x00*8+5, 0x00*8+6, 0x00*8+7
	},
	{ STEP16(0, 8) },
	0x200
};


static GFXDECODE_START( gfx_madalien )
	GFXDECODE_ENTRY( nullptr,   0, charlayout,     0x20, 2 ) /* foreground characters, stored in RAM */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,        0, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, headlightlayout,   0, 1 )
GFXDECODE_END


void madalien_state::madalien_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, 336, 0, 256, 288, 0, 256);
	screen.set_screen_update(FUNC(madalien_state::screen_update_madalien));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_madalien);
	PALETTE(config, m_palette, FUNC(madalien_state::madalien_palette), 0x30, 0x20);

	mc6845_device &crtc(MC6845(config, "crtc", PIXEL_CLOCK / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
}
