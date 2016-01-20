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
	cgc7900_state(const machine_config &mconfig, device_type type, std::string tag)
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
	required_shared_ptr<UINT16> m_chrom_ram;
	required_shared_ptr<UINT16> m_plane_ram;
	required_shared_ptr<UINT16> m_clut_ram;
	required_shared_ptr<UINT16> m_overlay_ram;
	required_shared_ptr<UINT16> m_roll_bitmap;
	required_shared_ptr<UINT16> m_pan_x;
	required_shared_ptr<UINT16> m_pan_y;
	required_shared_ptr<UINT16> m_zoom;
	required_shared_ptr<UINT16> m_blink_select;
	required_shared_ptr<UINT16> m_plane_select;
	required_shared_ptr<UINT16> m_plane_switch;
	required_shared_ptr<UINT16> m_color_status_fg;
	required_shared_ptr<UINT16> m_color_status_bg;
	required_shared_ptr<UINT16> m_roll_overlay;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_PALETTE_INIT(cgc7900);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

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

	void update_clut();
	void draw_bitmap(screen_device *screen, bitmap_rgb32 &bitmap);
	void draw_overlay(screen_device *screen, bitmap_rgb32 &bitmap);

	/* interrupt state */
	UINT16 m_int_mask;

	/* video state */
	rgb_t m_clut[256];
	int m_blink;

	TIMER_DEVICE_CALLBACK_MEMBER(blink_tick);
};

/*----------- defined in video/cgc7900.c -----------*/

MACHINE_CONFIG_EXTERN( cgc7900_video );

#endif
