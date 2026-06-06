// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay,Brice Onken

// Sony NEWS laptop graphics subsystem
// Uses a basic framebuffer and monochrome 1120x780 screen

#ifndef MAME_SONY_NEWS_LCDFB_H
#define MAME_SONY_NEWS_LCDFB_H

#pragma once

#include "screen.h"

class news_lcd_device : public device_t {
public:
	news_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner) :
	 news_lcd_device(mconfig, tag, owner, 0)
	{
	}

	news_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_vram(T &&tag) { m_vram.set_tag(std::forward<T>(tag)); }

	void map_lctc(address_map &map) ATTR_COLD;

	void lcd_enable_w(u8 data);
	void lcd_dim_w(u8 data);

protected:
	void device_start() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	u8 lctc_r(offs_t offset);
	void lctc_w(offs_t offset, u8 data);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	required_device<screen_device> m_lcd;
	required_shared_ptr<u32> m_vram;

	bool m_lcd_enable = false;
	bool m_lcd_dim = false;
};


DECLARE_DEVICE_TYPE(NEWS_LCD, news_lcd_device)

#endif
