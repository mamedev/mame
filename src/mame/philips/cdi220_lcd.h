// license:BSD-3-Clause
// copyright-holders:Vincent.Halver

/*
    Philips CD-i 220 front panel display.
*/

#ifndef MAME_PHILIPS_CDI220_LCD_H
#define MAME_PHILIPS_CDI220_LCD_H

#pragma once

class cdi220_lcd
{
public:
	static constexpr int WIDTH = 256;
	static constexpr int HEIGHT = 36;
	static constexpr int D_WIDTH = 20;
	static constexpr int D_HEIGHT = 22;

	static void draw_digit(bitmap_rgb32& bitmap, const rectangle& bounds, const uint16_t data, uint8_t idx);
	static void draw(bitmap_rgb32& bitmap, const rectangle& bounds, const uint8_t* lcd_state);
};

#endif // MAME_PHILIPS_CDI220_LCD_H
