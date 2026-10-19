// license:BSD-3-Clause
// copyright-holders:Vincent.Halver

/*
    Philips CD-i 220 front panel display.
*/

#ifndef MAME_PHILIPS_CDI220_LCD_H
#define MAME_PHILIPS_CDI220_LCD_H

#pragma once

#include "screen.h"

class cdi220_lcd : public device_t
{
public:
	static constexpr int WIDTH = 256;
	static constexpr int HEIGHT = 36;
	static constexpr int D_WIDTH = 20;
	static constexpr int D_HEIGHT = 22;

	static constexpr int STATE_SIZE = 16;

	cdi220_lcd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void state_w(offs_t offset, uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void draw(bitmap_rgb32& bitmap, const rectangle& bounds);
	void draw_lcd_text(bitmap_rgb32& bitmap, const rectangle& bounds, int x, int y, const char* text);
	void draw_digit(bitmap_rgb32& bitmap, const rectangle& bounds, const uint16_t data, uint8_t idx);

	required_device<screen_device> m_screen;

	uint8_t lcd_state[STATE_SIZE];
};

DECLARE_DEVICE_TYPE(CDI220_LCD, cdi220_lcd)

#endif // MAME_PHILIPS_CDI220_LCD_H
