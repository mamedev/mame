// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        MZ80 driver by Miodrag Milanovic

        22/11/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "mz80.h"

const gfx_layout mz80k_charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0, 1, 2, 3, 4, 5, 6, 7},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

const gfx_layout mz80kj_charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters + 256 blanks */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

/* Video hardware */
uint32_t mz80_state::screen_update_mz80k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mz80k_vertical ^= 1;
	m_mz80k_cursor_cnt++;
	uint16_t sy=0,ma=0;

	for(uint8_t y = 0; y < 25; y++ )
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 40; x++)
			{
				uint8_t chr = m_p_videoram[x];
				uint8_t gfx = m_p_chargen[(chr<<3) | ra];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=40;
	}
	return 0;
}

// same as above except bits are in reverse order
uint32_t mz80_state::screen_update_mz80kj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mz80k_vertical ^= 1;
	m_mz80k_cursor_cnt++;
	uint16_t sy=0,ma=0;

	for(uint8_t y = 0; y < 25; y++ )
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 40; x++)
			{
				uint8_t chr = m_p_videoram[x];
				uint8_t gfx = m_p_chargen[(chr<<3) | ra];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
		ma+=40;
	}
	return 0;
}

// has twice as much video ram and uses a scroll register
uint32_t mz80_state::screen_update_mz80a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mz80k_vertical ^= 1;
	m_mz80k_cursor_cnt++;
	uint16_t sy=0, ma=m_p_ram[0x17d] | (m_p_ram[0x17e] << 8);

	for(uint8_t y = 0; y < 25; y++ )
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 40; x++)
			{
				uint8_t chr = m_p_videoram[x&0x7ff];
				uint8_t gfx = m_p_chargen[(chr<<3) | ra];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=40;
	}
	return 0;
}
