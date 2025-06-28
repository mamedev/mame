// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

  K1GE/K2GE graphics emulation

******************************************************************************/

#ifndef MAME_SNK_K1GE_H
#define MAME_SNK_K1GE_H

#pragma once

#include "emupal.h"

class k1ge_device : public device_t, public device_video_interface, public device_palette_interface
{
public:
	template <typename T>
	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag)
		: k1ge_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// Static methods
	auto vblank_callback() { return m_vblank_pin_w.bind(); }
	auto hblank_callback() { return m_hblank_pin_w.bind(); }

	static const int K1GE_SCREEN_HEIGHT = 199;

protected:
	k1ge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool color);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual u32 palette_entries() const noexcept override { return mono_color() + (4 * 2 * 3) + 2; }
	virtual u32 palette_indirect_entries() const noexcept override { return m_is_color ? 256 : 8; }

	int mono_color() const noexcept { return m_is_color ? 192 : 0; }
	int bg_color() const noexcept { return mono_color() + (4 * 2 * 3); }
	int oow_color() const noexcept { return bg_color() + 1; }

	struct sprite_t
	{
		u16 spr_data;
		u8 x;
		u8 y;
		u8 index;
	};

	devcb_write_line m_vblank_pin_w;
	devcb_write_line m_hblank_pin_w;
	std::unique_ptr<u8[]> m_vram;
	u8 m_wba_h = 0, m_wba_v = 0, m_wsi_h = 0, m_wsi_v = 0;
	bool m_compat = false;
	bool m_is_color = false;

	emu_timer *m_timer = nullptr;
	emu_timer *m_hblank_on_timer = nullptr;
	bitmap_ind16 m_bitmap;

	void draw(int line);

	void get_tile_addr(u16 data, bool &hflip, u16 &tile_addr);
	u16 get_pixel(bool hflip, u16 &tile_data);
	void write_pixel(u16 &p, u16 pcode, u16 col);
	u16 get_tile_pcode(u16 map_data, int pal_base);
	void get_tile_data(int offset_x, u16 base, int line, int scroll_y, int pal_base, u16 &pcode, bool &hflip, u16 &tile_addr, u16 &tile_data);
	void draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, int pal_base);

	u16 get_sprite_pcode(u16 spr_data, u8 spr_index);
	void draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y);

	TIMER_CALLBACK_MEMBER(hblank_on_timer_callback);
	TIMER_CALLBACK_MEMBER(timer_callback);
	virtual void palette_init();
};


class k2ge_device : public k1ge_device
{
public:
	template <typename T>
	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag)
		: k2ge_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write(offs_t offset, u8 data) override;

protected:
	virtual void palette_init() override;
};

DECLARE_DEVICE_TYPE(K1GE, k1ge_device)
DECLARE_DEVICE_TYPE(K2GE, k2ge_device)

#endif // MAME_SNK_K1GE_H
