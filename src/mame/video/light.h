// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Tyson Smith
/*
        Silicon Graphics LG1 "Light" graphics board used as
        entry level graphics in the Indigo and IRIS Crimson.
*/

#ifndef MAME_VIDEO_LIGHT_H
#define MAME_VIDEO_LIGHT_H

#pragma once

#include "emupal.h"

class light_video_device : public device_t
{
public:
	light_video_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: light_video_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	light_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint32_t entry_r(offs_t offset, uint32_t mem_mask = ~0);
	void entry_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	static constexpr uint32_t x_res = 1024;
	static constexpr uint32_t y_res = 768;

protected:
	void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void do_rex_command();

	struct lg1_t
	{
		uint32_t m_config_sel = 0;
		uint32_t m_write_addr = 0;
		uint32_t m_control = 0;

		uint32_t m_command = 0;
		uint32_t m_x_start_i = 0;
		uint32_t m_y_start_i = 0;
		uint32_t m_xy_move = 0;
		uint32_t m_color_red_i = 0;
		uint32_t m_color_green_i = 0;
		uint32_t m_color_blue_i = 0;
		uint32_t m_color_back = 0;
		uint32_t m_z_pattern = 0;
		uint32_t m_x_end_i = 0;
		uint32_t m_y_end_i = 0;
		uint32_t m_x_curr_i = 0;
		uint32_t m_y_curr_i = 0;

		uint8_t m_palette_idx = 0;
		uint8_t m_palette_channel = 0;
		uint8_t m_palette_entry[3]{};
		uint8_t m_pix_read_mask[256]{};
	};
	lg1_t m_lg1;

	required_device<palette_device> m_palette;
	std::unique_ptr<uint8_t[]> m_framebuffer;
};

DECLARE_DEVICE_TYPE(LIGHT_VIDEO, light_video_device)


#endif // MAME_VIDEO_LIGHT_H
