// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/
#ifndef MAME_INCLUDES_TOOBIN_H
#define MAME_INCLUDES_TOOBIN_H

#pragma once

#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class toobin_state : public driver_device
{
public:
	toobin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_interrupt_scan(*this, "interrupt_scan"),
		m_xscroll(*this, "xscroll"),
		m_yscroll(*this, "yscroll")
	{ }

	void toobin(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void video_start() override;

	TIMER_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(uint16_t data);

	DECLARE_WRITE16_MEMBER(interrupt_scan_w);
	DECLARE_WRITE16_MEMBER(paletteram_w);
	DECLARE_WRITE16_MEMBER(intensity_w);
	DECLARE_WRITE16_MEMBER(xscroll_w);
	DECLARE_WRITE16_MEMBER(yscroll_w);
	DECLARE_WRITE16_MEMBER(slip_w);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_interrupt_scan;
	required_shared_ptr<uint16_t> m_xscroll;
	required_shared_ptr<uint16_t> m_yscroll;

	double          m_brightness;
	bitmap_ind16 m_pfbitmap;

	emu_timer *m_scanline_interrupt_timer;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_TOOBIN_H
