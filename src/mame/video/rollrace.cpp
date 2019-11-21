// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#include "emu.h"
#include "includes/rollrace.h"


#define RA_FGCHAR_BASE  0
#define RA_BGCHAR_BASE  4
#define RA_SP_BASE  5


TILE_GET_INFO_MEMBER(rollrace_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[(tile_index & 0x1f)*2+1] & 0x1f;

	SET_TILE_INFO_MEMBER(RA_FGCHAR_BASE + m_chrbank,
		code,
		color,
		TILE_FLIPY);
}

void rollrace_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rollrace_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scroll_cols(32);
}

WRITE8_MEMBER(rollrace_state::vram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(rollrace_state::cram_w)
{
	m_colorram[offset] = data;
	if(offset & 1)
	{
		// TODO: optimize
		m_fg_tilemap->mark_all_dirty();
		//for(int x = 0; x < 32; x++)
		//  m_fg_tilemap->mark_tile_dirty(x + ((offset >> 1)*32));
	}
	else
		m_fg_tilemap->set_scrolly(offset >> 1,data);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Stinger has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
void rollrace_state::rollrace_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		bit0 = BIT(color_prom[palette.entries()], 0);
		bit1 = BIT(color_prom[palette.entries()], 1);
		bit2 = BIT(color_prom[palette.entries()], 2);
		bit3 = BIT(color_prom[palette.entries()], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		bit0 = BIT(color_prom[2 * palette.entries()], 0);
		bit1 = BIT(color_prom[2 * palette.entries()], 1);
		bit2 = BIT(color_prom[2 * palette.entries()], 2);
		bit3 = BIT(color_prom[2 * palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));

		color_prom++;
	}
}

WRITE_LINE_MEMBER(rollrace_state::charbank_0_w)
{
	m_chrbank = state | (m_chrbank & 2);
	m_fg_tilemap->mark_all_dirty();
}

WRITE_LINE_MEMBER(rollrace_state::charbank_1_w)
{
	m_chrbank = (m_chrbank & 1) | (state << 1);
	m_fg_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(rollrace_state::bkgpen_w)
{
	m_bkgpen = data;
}

WRITE_LINE_MEMBER(rollrace_state::spritebank_w)
{
	m_spritebank = state;
}

WRITE8_MEMBER(rollrace_state::backgroundpage_w)
{
	m_bkgpage = data & 0x1f;
	m_bkgflip = ( data & 0x80 ) >> 7;

	/* 0x80 flip vertical */
}

WRITE8_MEMBER(rollrace_state::backgroundcolor_w)
{
	m_bkgcol = data;
}

WRITE8_MEMBER(rollrace_state::flipy_w)
{
	m_flipy = data & 0x01;
	// bit 2: cleared at night stage in attract, unknown purpose
}

WRITE_LINE_MEMBER(rollrace_state::flipx_w)
{
	m_flipx = state;
	m_fg_tilemap->set_flip(m_flipx ? TILEMAP_FLIPX|TILEMAP_FLIPY : 0);
}

uint32_t rollrace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram;
	int offs;
	int sx, sy;
	const uint8_t *mem = memregion("user1")->base();

	/* fill in background colour*/
	bitmap.fill(m_bkgpen, cliprect);

	/* draw road */
	for (offs = 0x3ff; offs >= 0; offs--)
	{
		if(!(m_bkgflip))
			sy = ( 31 - offs / 32 ) ;
		else
			sy = ( offs / 32 ) ;

		sx = ( offs%32 ) ;

		if(m_flipx)
			sx = 31-sx ;

		if(m_flipy)
			sy = 31-sy ;

		m_gfxdecode->gfx(RA_BGCHAR_BASE)->transpen(bitmap,
			cliprect,
			mem[offs + ( m_bkgpage * 1024 )]
			+ ((( mem[offs + 0x4000 + ( m_bkgpage * 1024 )] & 0xc0 ) >> 6 ) * 256 ) ,
			m_bkgcol,
			m_flipx,(m_bkgflip^m_flipy),
			sx*8,sy*8,0);
	}

	/* sprites */
	for ( offs = 0x80-4 ; offs >=0x0 ; offs -= 4)
	{
		int s_flipy = 0;
		int bank = 0;

		sy=spriteram[offs] - 16;
		sx=spriteram[offs+3] - 16;

		if(sx && sy)
		{
			if(m_flipx)
				sx = 224 - sx;
			if(m_flipy)
				sy = 224 - sy;

			if(spriteram[offs+1] & 0x80)
				s_flipy = 1;

			bank = (( spriteram[offs+1] & 0x40 ) >> 6 ) ;

			if(bank)
				bank += m_spritebank;

			m_gfxdecode->gfx( RA_SP_BASE + bank )->transpen(bitmap,cliprect,
				spriteram[offs+1] & 0x3f ,
				spriteram[offs+2] & 0x1f,
				m_flipx,!(s_flipy^m_flipy),
				sx,sy,0);
		}
	}

	m_fg_tilemap->draw(screen,bitmap,cliprect,0,0);
	return 0;
}
