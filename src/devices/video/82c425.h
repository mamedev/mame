// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Chips 82C245 CGA LCD/CRT Controller

**********************************************************************/

#ifndef MAME_VIDEO_82C425_H
#define MAME_VIDEO_82C425_H

#pragma once

#include "emupal.h"


class f82c425_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	f82c425_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_lcd_palette(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }

	auto crt_lcd_callback() { return m_crt_lcd_cb.bind(); }

	void io_map(address_map &map) ATTR_COLD;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	uint8_t address_r() { return m_register_address; }
	void address_w(uint8_t data) { m_register_address = data; }
	uint8_t register_r();
	void register_w(uint8_t data);
	uint8_t extreg_r(offs_t offset);
	void extreg_w(offs_t offset, uint8_t data);

	void dispfont_map(address_map &map) ATTR_COLD;

	void lcd_draw_line_text(bitmap_rgb32 &bitmap, uint8_t ra, uint16_t y, uint64_t frame);
	void lcd_draw_line_gfx1(bitmap_rgb32 &bitmap, uint8_t ra, uint16_t y, uint64_t frame);
	void lcd_draw_line_gfx2(bitmap_rgb32 &bitmap, uint8_t ra, uint16_t y, uint64_t frame);

	bool cursor_visible(uint16_t ma, uint8_t ra);
	void update_blink_cursor_state(uint64_t frame);

	// address space configurations
	const address_space_config m_space_config;

	required_device<palette_device> m_palette;

	devcb_write_line m_crt_lcd_cb;

	// registers
	uint8_t  m_register_address;
	uint8_t  m_horiz_total;          // 0x00
	uint8_t  m_horiz_disp;           // 0x01
	uint8_t  m_horiz_sync_pos;       // 0x02
	uint8_t  m_vert_total;           // 0x04
	uint8_t  m_vert_total_adj;       // 0x05
	uint8_t  m_vert_disp;            // 0x06
	uint8_t  m_vert_sync_pos;        // 0x07
	uint8_t  m_max_scan_row;         // 0x09
	uint8_t  m_cursor_start_scan;    // 0x0a
	uint8_t  m_cursor_end_scan;      // 0x0b
	uint16_t m_disp_start_addr;      // 0x0c/0x0d
	uint16_t m_cursor_addr;          // 0x0e/0x0f
	uint16_t m_light_pen_addr;       // 0x10/0x11

	// extension registers
	uint8_t  m_ac_control;           // 0xd9
	uint8_t  m_threshold;            // 0xda
	uint8_t  m_shift_param;          // 0xdb
	uint8_t  m_hsync_width;          // 0xdc
	uint8_t  m_vsync_width;          // 0xdd
	uint8_t  m_timing_control;       // 0xde
	uint8_t  m_func_control;         // 0xdf
	uint8_t  m_mode_control;
	uint8_t  m_color_select;
	uint8_t  m_input_status;

	// other internal state
	bool m_blink_state;
	bool m_cursor_state;
	bool m_dispen_state;
	bool m_light_pen_latched;
};


DECLARE_DEVICE_TYPE(F82C425, f82c425_device)


#endif // MAME_VIDEO_82C425_H
