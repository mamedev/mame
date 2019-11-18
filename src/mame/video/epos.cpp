// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Epos games

***************************************************************************/

#include "emu.h"
#include "includes/epos.h"

/***************************************************************************

  These games has one 32 byte palette PROM, connected to the RGB output this way:

  bit 7 -- 240 ohm resistor  -- RED
        -- 510 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
        -- 240 ohm resistor  -- GREEN
        -- 510 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 240 ohm resistor  -- BLUE
  bit 0 -- 510 ohm resistor  -- BLUE

***************************************************************************/

void epos_state::epos_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	int const len = memregion("proms")->bytes();

	for (offs_t i = 0; i < len; i++)
		set_pal_color(palette, i, color_prom[i]);
}

void epos_state::set_pal_color(palette_device &palette, uint8_t offset, uint8_t data)
{
	int bit0, bit1, bit2;

	bit0 = BIT(data, 7);
	bit1 = BIT(data, 6);
	bit2 = BIT(data, 5);
	int const r = 0x92 * bit0 + 0x4a * bit1 + 0x23 * bit2;

	bit0 = BIT(data, 4);
	bit1 = BIT(data, 3);
	bit2 = BIT(data, 2);
	int const g = 0x92 * bit0 + 0x4a * bit1 + 0x23 * bit2;

	bit0 = BIT(data, 1);
	bit1 = BIT(data, 0);
	int const b = 0xad * bit0 + 0x52 * bit1;

	palette.set_pen_color(offset, rgb_t(r, g, b));
}

// later (tristar 9000) games uses a dynamic palette instead of prom
WRITE8_MEMBER(epos_state::dealer_pal_w)
{
	set_pal_color(*m_palette, offset, data);
}

WRITE8_MEMBER(epos_state::port_1_w)
{
	/* D0 - start light #1
	   D1 - start light #2
	   D2 - coin counter
	   D3 - palette select
	   D4-D7 - unused
	 */

	m_leds[0] = BIT(data, 0);
	m_leds[1] = BIT(data, 1);

	machine().bookkeeping().coin_counter_w(0, (data >> 2) & 0x01);

	m_palette_bank = (data >> 3) & 0x01;
}


uint32_t epos_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t const data = m_videoram[offs];

		int x = (offs % 136) * 2;
		int y = (offs / 136);

		if (flip_screen())
		{
			bitmap.pix32(240 - y, 270 - x + 1) = m_palette->pen((m_palette_bank << 4) | (data & 0x0f));
			bitmap.pix32(240 - y, 270 - x + 0) = m_palette->pen((m_palette_bank << 4) | (data >> 4));
		}
		else
		{
			bitmap.pix32(y, x + 0) = m_palette->pen((m_palette_bank << 4) | (data & 0x0f));
			bitmap.pix32(y, x + 1) = m_palette->pen((m_palette_bank << 4) | (data >> 4));
		}
	}

	return 0;
}
