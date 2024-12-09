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
	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: k1ge_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void update( bitmap_ind16 &bitmap, const rectangle &cliprect );

	// Static methods
	auto vblank_callback() { return m_vblank_pin_w.bind(); }
	auto hblank_callback() { return m_hblank_pin_w.bind(); }

	static const int K1GE_SCREEN_HEIGHT = 199;

protected:
	k1ge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint32_t palette_entries() const noexcept override { return PALETTE_SIZE; }

	devcb_write_line m_vblank_pin_w;
	devcb_write_line m_hblank_pin_w;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t m_wba_h = 0, m_wba_v = 0, m_wsi_h = 0, m_wsi_v = 0;

	emu_timer *m_timer = nullptr;
	emu_timer *m_hblank_on_timer = nullptr;
	std::unique_ptr<bitmap_ind16> m_bitmap;

	virtual void draw(int line);

	void draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, int pal_base );
	void draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y );
	TIMER_CALLBACK_MEMBER( hblank_on_timer_callback );
	TIMER_CALLBACK_MEMBER( timer_callback );
	virtual void palette_init();

private:
	static constexpr int PALETTE_SIZE = 8;
};


class k2ge_device : public k1ge_device
{
public:
	template <typename T>
	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: k2ge_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint32_t palette_entries() const noexcept override { return PALETTE_SIZE; }

	virtual void draw(int line) override;

	void draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, uint16_t pal_base );
	void draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y );
	void k1ge_draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, uint16_t pal_lut_base, uint16_t k2ge_lut_base );
	void k1ge_draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y );
	virtual void palette_init() override;

private:
	static constexpr int PALETTE_SIZE = 4096;
};

DECLARE_DEVICE_TYPE(K1GE, k1ge_device)
DECLARE_DEVICE_TYPE(K2GE, k2ge_device)

#endif // MAME_SNK_K1GE_H
