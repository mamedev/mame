// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*****************************************************************************
 *
 * video/abc802.c
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/abc80x.h"
#include "screen.h"



//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc802_update_row )
//-------------------------------------------------

MC6845_UPDATE_ROW( abc802_state::abc802_update_row )
{
	/*

	    PAL16R4 equation:

	    IF (VCC)    *OS   = FC + RF / RC
	                *RG:  = HS / *RG + *ATE / *RG + ATD / *RG + LL /
	                        *RG + AT1 / *RG + AT0 / ATE + *ATD + *LL +
	                        *AT1 + *AT0
	                *RI:  = *RI + *INV / *RI + LL / *INV + *LL
	                *RF:  = HS / *RF + *ATE / *RF + ATD / *RF + LL /
	                        *RF + AT1 / *RF + AT0 / ATE + *ATD + *LL +
	                        *AT1 + AT0
	                *RC:  = HS / *RC + *ATE / *RC + *ATD / *RC + LL /
	                        *RC + *ATI / *RC + AT0 / ATE + *LL + *AT1 +
	                        *AT0
	    IF (VCC)    *O0   = *CUR + *AT0 / *CUR + ATE
	                *O1   = *CUR + *AT1 / *CUR + ATE


	    + = AND
	    / = OR
	    * = Inverted

	    ATD     Attribute data
	    ATE     Attribute enable
	    AT0,AT1 Attribute address
	    CUR     Cursor
	    FC      FLSH clock
	    HS      Horizontal sync
	    INV     Inverted signal input
	    LL      Load when Low
	    OEL     Output Enable when Low
	    RC      Row clear
	    RF      Row flash
	    RG      Row graphic
	    RI      Row inverted

	*/

	const pen_t *pen = m_palette->pens();

	int rf = 0, rc = 0, rg = 0;

	y += vbp;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_char_ram[(ma + column) & 0x7ff];
		uint16_t address = code << 4;
		uint8_t ra_latch = ra;
		uint8_t data;

		int ri = (code & ABC802_INV) ? 1 : 0;

		if (column == cursor_x)
		{
			ra_latch = 0x0f;
		}

		if ((m_flshclk && rf) || rc)
		{
			ra_latch = 0x0e;
		}

		if (rg)
		{
			address |= 0x800;
		}

		data = m_char_rom->base()[(address + ra_latch) & 0xfff];

		if (data & ABC802_ATE)
		{
			int attr = data & 0x03;
			int value = (data & ABC802_ATD) ? 1 : 0;

			switch (attr)
			{
			case 0x00:
				// Row Graphic
				rg = value;
				break;

			case 0x01:
				// Row Flash
				rf = value;
				break;

			case 0x02:
				// Row Clear
				rc = value;
				break;

			case 0x03:
				// undefined
				break;
			}
		}
		else
		{
			data <<= 2;

			if (m_80_40_mux)
			{
				for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
				{
					int x = hbp + ((column + 3) * ABC800_CHAR_WIDTH) + bit;
					int color = (BIT(data, 7) ^ ri) && de;

					bitmap.pix32(y, x) = pen[color];

					data <<= 1;
				}
			}
			else
			{
				for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
				{
					int x = hbp + ((column + 3) * ABC800_CHAR_WIDTH) + (bit << 1);
					int color = (BIT(data, 7) ^ ri) && de;

					bitmap.pix32(y, x) = pen[color];
					bitmap.pix32(y, x + 1) = pen[color];

					data <<= 1;
				}

				column++;
			}
		}
	}
}


//-------------------------------------------------
//  vs_w - vertical sync write
//-------------------------------------------------

WRITE_LINE_MEMBER( abc802_state::vs_w )
{
	if (!state)
	{
		// flash clock
		if (m_flshclk_ctr & 0x20)
		{
			m_flshclk = !m_flshclk;
			m_flshclk_ctr = 0;
		}
		else
		{
			m_flshclk_ctr++;
		}
	}
}


//-------------------------------------------------
//  machine_config( abc802_video )
//-------------------------------------------------

void abc802_state::abc802_video(machine_config &config)
{
	mc6845_device &mc6845(MC6845(config, MC6845_TAG, ABC800_CCLK));
	mc6845.set_screen(SCREEN_TAG);
	mc6845.set_show_border_area(true);
	mc6845.set_char_width(ABC800_CHAR_WIDTH);
	mc6845.set_update_row_callback(FUNC(abc802_state::abc802_update_row));
	mc6845.out_vsync_callback().set(FUNC(abc802_state::vs_w));
	mc6845.out_vsync_callback().append(m_dart, FUNC(z80dart_device::rib_w)).invert();

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::amber()));
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen.set_raw(XTAL(12'000'000), 0x300, 0, 0x1e0, 0x13a, 0, 0xf0);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}
