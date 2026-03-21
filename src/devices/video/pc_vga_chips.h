// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_CHIPS_H
#define MAME_VIDEO_PC_VGA_CHIPS_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"


class f65535_vga_device :  public svga_device
{
public:
	// stub-ish, clearly unemulated_features but mdartstr client don't deserve that
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	f65535_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void ext_map(address_map &map) ATTR_COLD;
private:
	virtual space_config_vector memory_space_config() const override;

	virtual void io_3bx_3dx_map(address_map &map) override ATTR_COLD;

	address_space_config m_ext_space_config;

	u8 m_ext_index;
	u8 m_chip_version;

	u8 m_cpu_interface_1;
	u8 m_memory_control_1;
	u8 m_memory_control_2;
	u8 m_palette_control;
	u8 m_cpu_paging;
	u8 m_start_address_top;
	u8 m_software_flags_0;
	u8 m_horizontal_overflow;
	u8 m_alt_h_panel_size;

	u8 m_video_interface;
	u8 m_software_flags_1;

	u8 m_clock_control[4];
	u8 m_software_flags_2;

	u8 m_display_type;
	u8 m_h_compensation, m_v_compensation;
	u8 m_v_line_replication;
	u8 m_output_drive;

	void flush_true_color_mode();
};

DECLARE_DEVICE_TYPE(F65535_VGA, f65535_vga_device)

#endif // MAME_VIDEO_PC_VGA_CHIPS_H
