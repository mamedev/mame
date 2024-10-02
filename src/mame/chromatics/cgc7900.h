// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_CHROMATICS_CGC7900_H
#define MAME_CHROMATICS_CGC7900_H

#pragma once


#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "machine/com8116.h"
#include "machine/mm58167.h"
#include "machine/keyboard.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"

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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, M68000_TAG)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_char_rom(*this, "gfx1")
		, m_chrom_ram(*this, "chrom_ram")
		, m_plane_ram(*this, "plane_ram")
		, m_clut_ram(*this, "clut_ram")
		, m_overlay_ram(*this, "overlay_ram")
		, m_roll_bitmap(*this, "roll_bitmap")
		, m_pan_x(*this, "pan_x")
		, m_pan_y(*this, "pan_y")
		, m_zoom(*this, "zoom")
		, m_blink_select(*this, "blink_select")
		, m_plane_select(*this, "plane_select")
		, m_plane_switch(*this, "plane_switch")
		, m_color_status_fg(*this, "color_status_fg")
		, m_color_status_bg(*this, "color_status_bg")
		, m_roll_overlay(*this, "roll_overlay")
		, m_i8251_0(*this, INS8251_0_TAG)
		, m_i8251_1(*this, INS8251_1_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_region m_char_rom;
	required_shared_ptr<u16> m_chrom_ram;
	required_shared_ptr<u16> m_plane_ram;
	required_shared_ptr<u16> m_clut_ram;
	required_shared_ptr<u16> m_overlay_ram;
	required_shared_ptr<u16> m_roll_bitmap;
	required_shared_ptr<u16> m_pan_x;
	required_shared_ptr<u16> m_pan_y;
	required_shared_ptr<u16> m_zoom;
	required_shared_ptr<u16> m_blink_select;
	required_shared_ptr<u16> m_plane_select;
	required_shared_ptr<u16> m_plane_switch;
	required_shared_ptr<u16> m_color_status_fg;
	required_shared_ptr<u16> m_color_status_bg;
	required_shared_ptr<u16> m_roll_overlay;
	required_device<i8251_device> m_i8251_0;
	required_device<i8251_device> m_i8251_1;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cgc7900_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 keyboard_r();
	void keyboard_w(u16 data);
	void interrupt_mask_w(u16 data);
	u16 disk_data_r();
	void disk_data_w(u16 data);
	u16 disk_status_r();
	void disk_command_w(u16 data);
	u16 z_mode_r();
	void z_mode_w(u16 data);
	void color_status_w(u16 data);
	u16 sync_r();
	u16 unmapped_r();

	template <unsigned N> void irq(int state) { irq_encoder(N, state); }

	void update_clut();
	void draw_bitmap(screen_device *screen, bitmap_rgb32 &bitmap);
	void draw_overlay(screen_device *screen, bitmap_rgb32 &bitmap);

	/* interrupt state */
	u16 m_int_mask = 0U, m_int_active = 0U;

	/* video state */
	rgb_t m_clut[256]{};
	int m_blink = 0;

	TIMER_DEVICE_CALLBACK_MEMBER(blink_tick);

	void kbd_put(u8 data);

	void cgc7900(machine_config &config);
	void cgc7900_video(machine_config &config);
	void cgc7900_mem(address_map &map) ATTR_COLD;
	void keyboard_mem(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
private:
	u16 kbd_mods = 0U;
	u8 kbd_data = 0U;
	bool kbd_ready = false;

	void irq_encoder(int pin, int state);
};

#endif // MAME_CHROMATICS_CGC7900_H
