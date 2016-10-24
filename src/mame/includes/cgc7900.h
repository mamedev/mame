// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __CGC7900__
#define __CGC7900__


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "sound/ay8910.h"

#define M68000_TAG      "uh8"
#define INS8251_0_TAG   "uc10"
#define INS8251_1_TAG   "uc8"
#define MM58167_TAG     "uc6"
#define AY8910_TAG      "uc4"
#define K1135A_TAG      "uc11"
#define I8035_TAG       "i8035"
#define AM2910_TAG      "11d"
#define SCREEN_TAG      "screen"

class cgc7900_state : public driver_device
{
public:
	cgc7900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M68000_TAG),
			m_palette(*this, "palette"),
			m_char_rom(*this, "gfx1"),
			m_chrom_ram(*this, "chrom_ram"),
			m_plane_ram(*this, "plane_ram"),
			m_clut_ram(*this, "clut_ram"),
			m_overlay_ram(*this, "overlay_ram"),
			m_roll_bitmap(*this, "roll_bitmap"),
			m_pan_x(*this, "pan_x"),
			m_pan_y(*this, "pan_y"),
			m_zoom(*this, "zoom"),
			m_blink_select(*this, "blink_select"),
			m_plane_select(*this, "plane_select"),
			m_plane_switch(*this, "plane_switch"),
			m_color_status_fg(*this, "color_status_fg"),
			m_color_status_bg(*this, "color_status_bg"),
			m_roll_overlay(*this, "roll_overlay")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_memory_region m_char_rom;
	required_shared_ptr<uint16_t> m_chrom_ram;
	required_shared_ptr<uint16_t> m_plane_ram;
	required_shared_ptr<uint16_t> m_clut_ram;
	required_shared_ptr<uint16_t> m_overlay_ram;
	required_shared_ptr<uint16_t> m_roll_bitmap;
	required_shared_ptr<uint16_t> m_pan_x;
	required_shared_ptr<uint16_t> m_pan_y;
	required_shared_ptr<uint16_t> m_zoom;
	required_shared_ptr<uint16_t> m_blink_select;
	required_shared_ptr<uint16_t> m_plane_select;
	required_shared_ptr<uint16_t> m_plane_switch;
	required_shared_ptr<uint16_t> m_color_status_fg;
	required_shared_ptr<uint16_t> m_color_status_bg;
	required_shared_ptr<uint16_t> m_roll_overlay;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void palette_init_cgc7900(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint16_t keyboard_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void keyboard_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void interrupt_mask_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t disk_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void disk_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t disk_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void disk_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t z_mode_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void z_mode_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void color_status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sync_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void update_clut();
	void draw_bitmap(screen_device *screen, bitmap_rgb32 &bitmap);
	void draw_overlay(screen_device *screen, bitmap_rgb32 &bitmap);

	/* interrupt state */
	uint16_t m_int_mask;

	/* video state */
	rgb_t m_clut[256];
	int m_blink;

	void blink_tick(timer_device &timer, void *ptr, int32_t param);
};

/*----------- defined in video/cgc7900.c -----------*/

MACHINE_CONFIG_EXTERN( cgc7900_video );

#endif
