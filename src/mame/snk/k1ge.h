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
	k1ge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual u32 palette_entries() const noexcept override { return (4 * 2 * 3) + 2; }
	virtual u32 palette_indirect_entries() const noexcept override { return PALETTE_SIZE; }

	devcb_write_line m_vblank_pin_w;
	devcb_write_line m_hblank_pin_w;
	std::unique_ptr<u8[]> m_vram;
	u8 m_wba_h = 0, m_wba_v = 0, m_wsi_h = 0, m_wsi_v = 0;

	emu_timer *m_timer = nullptr;
	emu_timer *m_hblank_on_timer = nullptr;
	std::unique_ptr<bitmap_ind16> m_bitmap;

	virtual void draw(int line);

	void draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, int pal_base);
	void draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y);
	TIMER_CALLBACK_MEMBER(hblank_on_timer_callback);
	TIMER_CALLBACK_MEMBER(timer_callback);
	virtual void palette_init();

private:
	static constexpr int PALETTE_SIZE = 8;
	static constexpr int BG_COLOR = (4 * 2 * 3);
	static constexpr int OOW_COLOR = BG_COLOR + 1;
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
	virtual u32 palette_entries() const noexcept override { return 192 + (4 * 2 * 3) + 2; }
	virtual u32 palette_indirect_entries() const noexcept override { return PALETTE_SIZE; }

	virtual void draw(int line) override;

	void draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, u16 pal_base);
	void draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y);
	void k1ge_draw_scroll_plane(u16 *p, u16 base, int line, int scroll_x, int scroll_y, u16 pal_base);
	void k1ge_draw_sprite_plane(u16 *p, u16 priority, int line, int scroll_x, int scroll_y);
	virtual void palette_init() override;

private:
	static constexpr int PALETTE_SIZE = 256;
	static constexpr int MONO_COLOR = 192;
	static constexpr int BG_COLOR = MONO_COLOR + (4 * 2 * 3);
	static constexpr int OOW_COLOR = BG_COLOR + 1;
};

DECLARE_DEVICE_TYPE(K1GE, k1ge_device)
DECLARE_DEVICE_TYPE(K2GE, k2ge_device)

#endif // MAME_SNK_K1GE_H
