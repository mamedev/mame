// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_ATARI_STVIDEO_H
#define MAME_ATARI_STVIDEO_H

#pragma once

#include "stmmu.h"

class st_video_device : public device_t, public device_video_interface, public device_palette_interface {
public:
	st_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_mmu(T &&tag) { m_mmu.set_tag(std::forward<T>(tag)); }

	void map(address_map &map) ATTR_COLD;

	auto de_cb()    { return m_de_cb.bind();    }
	auto hsync_cb() { return m_hsync_cb.bind(); }
	auto vsync_cb() { return m_vsync_cb.bind(); }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_config_complete() override;
	ioport_constructor device_input_ports() const override;
	u32 palette_entries() const noexcept override { return 16; }

private:
	enum {
		M_50,
		M_60,
		M_71
	};

	static const u16 base_hsc[3];
	static const u16 base_vsc[3];
	static const u16 vsync_start[3];
	static const u16 vde_start[3];
	static const u16 vde_end[3];
	static const u32 cycles_per_screen[3];

	required_ioport m_phase;
	devcb_write_line m_de_cb;
	devcb_write_line m_hsync_cb;
	devcb_write_line m_vsync_cb;

	required_shared_ptr<u16> m_ram;
	required_device<st_mmu_device> m_mmu;
	bitmap_rgb32 m_screen_bitmap;

	emu_timer *m_event_timer;
	emu_timer *m_de_timer;

	u64 m_start_screen_time, m_hsc_base, m_hdec_base, m_shifter_base_x, m_shifter_update_time;
	u64 m_prev_glue_tick;
	u64 m_load_current, m_load_end;
	u16 m_ir[4], m_rr[4];
	u32 m_adr_base, m_adr_live;
	u16 m_vsc, m_vdec, m_shifter_y;
	u16 m_palette[16];
	u8 m_sync, m_res, m_vsync, m_hsync, m_vde, m_hde, m_de;
	u8 m_mode;
	u8 m_pixcnt, m_rdelay;
	bool m_load_1, m_load_2, m_pixcnt_en, m_reload;

	TIMER_CALLBACK_MEMBER(timer_event);
	TIMER_CALLBACK_MEMBER(de_event);

	void adr_base_h_w(u8 data);
	u8 adr_base_h_r();
	void adr_base_m_w(u8 data);
	u8 adr_base_m_r();
	u8 adr_live_h_r();
	u8 adr_live_m_r();
	u8 adr_live_l_r();

	void sync_w(u8 data);
	u8 sync_r();

	void res_w(u8 data);
	u8 res_r();

	void palette_w(offs_t offset, u16 data, u16 mem_mask);
	u16 palette_r(offs_t offset);

	void hsync_on();
	void hsync_off(u64 glue_tick);
	void hde_on(u64 glue_tick);
	void hde_off(u64 glue_tick);

	void update_mode();
	void shifter_handle_load();
	void shifter_sync(u64 now = 0);
	void de_set(bool level);
	std::tuple<u64, u16, u16> compute_glue_tick(u64 now);
	void glue_sync();
	void glue_determine_next_event();

	u64 time_now();
	void next_event(u64 when);
	void next_de_event(u64 glue_tick, int level);

	std::string context(u64 now = 0);
};

DECLARE_DEVICE_TYPE(ST_VIDEO, st_video_device)


#endif
