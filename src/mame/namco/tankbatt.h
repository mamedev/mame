// license:BSD-3-Clause
// copyright-holders:Brad Oliver
#ifndef MAME_INCLUDES_TANKBATT_H
#define MAME_INCLUDES_TANKBATT_H

#pragma once

#include "machine/timer.h"
#include "machine/netlist.h"

#include "netlist/nl_setup.h"
#include "audio/nl_tankbatt.h"

#include "emupal.h"
#include "tilemap.h"

class tankbatt_state : public driver_device
{
public:
	tankbatt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bulletsram(*this, "bulletsram"),
		m_videoram(*this, "videoram"),
		m_player_input(*this, "P%u", 1U),
		m_dips(*this, "DSW"),
		m_sound_s1(*this, "sound_nl:s1"),
		m_sound_s2(*this, "sound_nl:s2"),
		m_sound_off(*this, "sound_nl:off"),
		m_sound_engine_hi(*this, "sound_nl:engine_hi"),
		m_sound_shoot(*this, "sound_nl:shoot"),
		m_sound_hit(*this, "sound_nl:hit")
	{ }

	void tankbatt(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bulletsram;
	required_shared_ptr<uint8_t> m_videoram;

	required_ioport_array<2> m_player_input;
	required_ioport m_dips;

	required_device<netlist_mame_logic_input_device> m_sound_s1;
	required_device<netlist_mame_logic_input_device> m_sound_s2;
	required_device<netlist_mame_logic_input_device> m_sound_off;
	required_device<netlist_mame_logic_input_device> m_sound_engine_hi;
	required_device<netlist_mame_logic_input_device> m_sound_shoot;
	required_device<netlist_mame_logic_input_device> m_sound_hit;

	int m_sound_enable = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t in0_r(offs_t offset);
	uint8_t in1_r(offs_t offset);
	uint8_t dsw_r(offs_t offset);
	void intack_w(uint8_t data);
	void coincounter_w(int state);
	void coinlockout_w(int state);
	void videoram_w(offs_t offset, uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	void tankbatt_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_TANKBATT_H
