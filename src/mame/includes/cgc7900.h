// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_CGC7900_H
#define MAME_INCLUDES_CGC7900_H

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
	required_device<i8251_device> m_i8251_0;
	required_device<i8251_device> m_i8251_1;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void cgc7900_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER( keyboard_r );
	DECLARE_WRITE16_MEMBER( keyboard_w );
	DECLARE_WRITE16_MEMBER( interrupt_mask_w );
	DECLARE_READ16_MEMBER( disk_data_r );
	DECLARE_WRITE16_MEMBER( disk_data_w );
	DECLARE_READ16_MEMBER( disk_status_r );
	DECLARE_WRITE16_MEMBER( disk_command_w );
	DECLARE_READ16_MEMBER( z_mode_r );
	DECLARE_WRITE16_MEMBER( z_mode_w );
	DECLARE_WRITE16_MEMBER( color_status_w );
	DECLARE_READ16_MEMBER( sync_r );
	DECLARE_READ16_MEMBER( unmapped_r );

	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(irq) { irq_encoder(N, state); }

	void update_clut();
	void draw_bitmap(screen_device *screen, bitmap_rgb32 &bitmap);
	void draw_overlay(screen_device *screen, bitmap_rgb32 &bitmap);

	/* interrupt state */
	uint16_t m_int_mask, m_int_active;

	/* video state */
	rgb_t m_clut[256];
	int m_blink;

	TIMER_DEVICE_CALLBACK_MEMBER(blink_tick);

	void kbd_put(u8 data);

	void cgc7900(machine_config &config);
	void cgc7900_video(machine_config &config);
	void cgc7900_mem(address_map &map);
	void keyboard_mem(address_map &map);
	void cpu_space_map(address_map &map);
private:
	u16 kbd_mods;
	u8 kbd_data;
	bool kbd_ready;

	void irq_encoder(int pin, int state);
};

#endif
