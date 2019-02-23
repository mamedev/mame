// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Novag chess computers base driver

TODO:
- printer port

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"


// machine start/reset

void novagbase_state::machine_start()
{
	chessbase_state::machine_start();

	// zerofill/register for savestates
	m_lcd_control = 0;
	m_lcd_data = 0;

	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_data));
}

void novagbase_state::machine_reset()
{
	chessbase_state::machine_reset();
}


/***************************************************************************
    Helper Functions
***************************************************************************/

// LCD

void novagbase_state::novag_lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(novagbase_state::novag_lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 8)
	{
		// internal: (8+8)*1, external: 1*16
		bitmap.pix16(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
	}
}
