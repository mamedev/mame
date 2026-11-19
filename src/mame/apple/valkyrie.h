// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_VALKYRIE_H
#define MAME_APPLE_VALKYRIE_H

#pragma once

#include "cpu/m68000/m68040.h"
#include "machine/i2chle.h"

#include "emupal.h"
#include "screen.h"

class valkyrie_device : public device_t, public i2c_hle_interface
{
public:
	valkyrie_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	~valkyrie_device();

	void map(address_map &map) ATTR_COLD;
	void valkyriear_map(address_map &map) ATTR_COLD;

	auto write_irq() { return m_irq.bind(); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	// i2c_hle_interface overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

	void recalc_ints();
	void recalc_mode();

	u32 m_pixel_clock;

	u8 m_pal_address, m_pal_idx, m_mode;

private:
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport m_monitor_config;
	memory_share_creator<u32> m_vram;
	devcb_write_line m_irq;

	u32 m_vram_offset;
	u8 m_monitor_id;
	u32 m_base, m_stride, m_video_timing;
	s32 m_int_status;
	u32 m_hres, m_vres, m_htotal, m_vtotal, m_config;
	u8 m_int_enable;
	u8 m_M, m_N, m_P;
	u8 m_video_in_control, m_color_key;
	u16 m_window_x, m_window_y, m_window_width, m_window_height, m_field_x, m_field_y;
	u8 m_video_clut[256][3];
	u8 m_pal_latch[3];

	void load_pal_latch();
	void draw_video_window(bitmap_rgb32 &bitmap, rectangle const &cliprect, u32 stride);

	// in mode 0x1f, the mode after a reset, no interrupt is latched and blanking stays asserted
	bool timing_stopped() const { return (m_video_timing & 0x7f) == 0x1f; }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 regs_r(offs_t offset);
	void regs_w(offs_t offset, u8 data);

	u8 regs64_r(offs_t offset);
	void regs64_w(offs_t offset, u8 data);

	u32 ramdac_r(offs_t offset);
	void ramdac_w(offs_t offset, u32 data);
	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);
	u32 video_buffer_r(offs_t offset);
	void video_buffer_w(offs_t offset, u32 data, u32 mem_mask);

	void vblank_w(int state);
};

DECLARE_DEVICE_TYPE(VALKYRIE, valkyrie_device)

#endif  /* MAME_APPLE_VALKYRIE_H */
