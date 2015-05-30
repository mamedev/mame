// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#include "emu.h"
#include "includes/rollrace.h"



#define RA_FGCHAR_BASE  0
#define RA_BGCHAR_BASE  4
#define RA_SP_BASE  5

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Stinger has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT_MEMBER(rollrace_state, rollrace)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;


	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;
		bit0 = (color_prom[palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;
		bit0 = (color_prom[2*palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[2*palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[2*palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[2*palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));

		color_prom++;
	}
}

WRITE8_MEMBER(rollrace_state::charbank_w)
{
	m_charbank[offset&1] = data;
	m_chrbank = m_charbank[0] | (m_charbank[1] << 1) ;
}


WRITE8_MEMBER(rollrace_state::bkgpen_w)
{
	m_bkgpen = data;
}

WRITE8_MEMBER(rollrace_state::spritebank_w)
{
	m_spritebank = data;
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
}

WRITE8_MEMBER(rollrace_state::flipx_w)
{
	m_flipx = data & 0x01;
}

UINT32 rollrace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	int offs;
	int sx, sy;
	int scroll;
	int col;
	const UINT8 *mem = memregion("user1")->base();

	/* fill in background colour*/
	bitmap.fill(m_bkgpen, cliprect);

	/* draw road */
	for (offs = 0x3ff; offs >= 0; offs--)
		{
			if(!(m_bkgflip))
				{
				sy = ( 31 - offs / 32 ) ;
				}
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




	/* draw foreground characters */
	for (offs = 0x3ff; offs >= 0; offs--)
	{
		sx =  offs % 32;
		sy =  offs / 32;

		scroll = ( 8 * sy + m_colorram[2 * sx] ) % 256;
		col = m_colorram[ sx * 2 + 1 ]&0x1f;

		if (!m_flipy)
		{
			scroll = (248 - scroll) % 256;
		}

		if (m_flipx) sx = 31 - sx;

		m_gfxdecode->gfx(RA_FGCHAR_BASE + m_chrbank)  ->transpen(bitmap,cliprect,
			m_videoram[ offs ]  ,
			col,
			m_flipx,m_flipy,
			8*sx,scroll,0);

	}



	return 0;
}
