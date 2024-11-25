// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#ifndef MAME_NAMCO_WARPWARP_H
#define MAME_NAMCO_WARPWARP_H

#pragma once

#include "machine/74259.h"
#include "machine/watchdog.h"
#include "geebee.h"
#include "warpwarp_a.h"
#include "emupal.h"
#include "tilemap.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_warpwarp_sound(*this, "warpwarp_custom"),
		m_geebee_sound(*this, "geebee_custom"),
		m_geebee_videoram(*this, "geebee_videoram"),
		m_videoram(*this, "videoram"),
		m_palette(*this, "palette"),
		m_latch(*this, "latch"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_dsw1(*this, "DSW1"),
		m_volin1(*this, "VOLIN1"),
		m_volin2(*this, "VOLIN2"),
		m_in_config(*this, "CONFIG"),
		m_ports(*this, { { "SW0", "SW1", "DSW2", "PLACEHOLDER" } }) // "IN1" & "IN2" are read separately when offset==3
	{ }

	void warpwarp(machine_config &config);
	void geebee(machine_config &config);
	void navarone(machine_config &config);
	void sos(machine_config &config);
	void kaitei(machine_config &config);
	void bombbee(machine_config &config);
	void geebeeb(machine_config &config);

	void init_navarone();
	void init_geebee();
	void init_kaitein();
	void init_warpwarp();
	void init_sos();
	void init_kaitei();
	void init_bombbee();

private:
	required_device<cpu_device> m_maincpu;
	optional_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<warpwarp_sound_device> m_warpwarp_sound;
	optional_device<geebee_sound_device> m_geebee_sound;
	optional_shared_ptr<uint8_t> m_geebee_videoram;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_device<palette_device> m_palette;
	optional_device<ls259_device> m_latch;
	optional_ioport m_in0;
	optional_ioport m_in1;
	optional_ioport m_in2;
	optional_ioport m_dsw1;
	optional_ioport m_volin1;
	optional_ioport m_volin2;
	optional_ioport m_in_config;
	optional_ioport_array<4> m_ports;

	int m_geebee_bgw = 0;
	int m_ball_on = 0;
	int m_ball_h = 0;
	int m_ball_v = 0;
	int m_ball_pen = 0;
	int m_ball_sizex = 0;
	int m_ball_sizey = 0;
	int m_handle_joystick = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	// warpwarp and bombbee
	uint8_t warpwarp_sw_r(offs_t offset);
	void warpwarp_out0_w(offs_t offset, uint8_t data);
	void warpwarp_videoram_w(offs_t offset, uint8_t data);
	uint8_t warpwarp_dsw1_r(offs_t offset);
	uint8_t warpwarp_vol_r();

	//geebee and navarone
	uint8_t geebee_in_r(offs_t offset);
	void geebee_out6_w(offs_t offset, uint8_t data);
	void counter_w(int state);
	void lock_out_w(int state);
	void geebee_bgw_w(int state);
	void ball_on_w(int state);
	void geebee_videoram_w(offs_t offset, uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	DECLARE_MACHINE_RESET(kaitei);

	DECLARE_VIDEO_START(geebee);
	void geebee_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(warpwarp);
	void warpwarp_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(navarone);
	void navarone_palette(palette_device &palette) const;
	void sos_palette(palette_device &palette) const;

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(geebee_get_tile_info);
	TILE_GET_INFO_MEMBER(navarone_get_tile_info);
	TILE_GET_INFO_MEMBER(warpwarp_get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void plot(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, pen_t pen);
	void draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect,pen_t pen);

	void vblank_irq(int state);

	void bombbee_map(address_map &map) ATTR_COLD;
	void geebee_map(address_map &map) ATTR_COLD;
	void geebee_port_map(address_map &map) ATTR_COLD;
	void warpwarp_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_WARPWARP_H
