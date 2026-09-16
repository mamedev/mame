// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The LISA video board

#ifndef MAME_APPLE_LISAVIDEO_H
#define MAME_APPLE_LISAVIDEO_H

#pragma once

#include "cpu/m68000/m68000.h"

class lisa_video_device : public device_t {
public:
	lisa_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }

	auto write_vint() { return m_vint_cb.bind(); }

	void base_w(u8 data);
	u8 base_r();

	u16 status_r();

	void vtmsk_0();
	void vtmsk_1();

	bool vint_r() { return m_vint_masked; }
	bool sn_r();

protected:
	lisa_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void device_config_complete() override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	struct video_state {
		u16 address;
		u16 x, y;
		bool vsync;
		bool hsync;
		bool csync;
		bool blank;
		bool vsint;
		bool sn;

		video_state(u16 _address, u16 _x, u16 _y, bool _vsync, bool _hsync, bool _csync, bool _blank, bool _vsint, bool _sn) {
			address = _address;
			x = _x;
			y = _y;
			vsync = _vsync;
			hsync = _hsync;
			csync = _csync;
			blank = _blank;
			vsint = _vsint;
			sn = _sn;
		}
	};

	devcb_write_line m_vint_cb;

	required_region_ptr<u8> m_vprom;
	required_shared_ptr<u16> m_ram;
	required_device<screen_device> m_screen;

	int m_max_vx, m_max_vy;
	u32 m_vsint_start;
	emu_timer *m_vint_timer;

	std::vector<video_state> m_video_states;

	u8 m_base;
	bool m_vint_raw, m_vint_masked, m_vtmsk;

	void analyze_video_rom();
	u32 current_state_index() const;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(vint_timer_tick);

	void tick_0();
	void tick_1();
};

class macxl_video_device : public lisa_video_device {
public:
	macxl_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(LISAVIDEO, lisa_video_device)
DECLARE_DEVICE_TYPE(MACXLVIDEO, macxl_video_device)

#endif
