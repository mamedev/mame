// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Escape hardware

*************************************************************************/
#ifndef MAME_INCLUDES_EPROM_H
#define MAME_INCLUDES_EPROM_H

#pragma once

#include "machine/adc0808.h"
#include "machine/timer.h"
#include "atarijsa.h"
#include "atarimo.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class eprom_state : public driver_device
{
public:
	eprom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_jsa(*this, "jsa"),
		m_share1(*this, "share1"),
		m_adc(*this, "adc"),
		m_extra(*this, "extra"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram")
	{ }

	void guts(machine_config &config);
	void eprom(machine_config &config);
	void klaxp(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_base_device> m_jsa;
	required_shared_ptr<uint16_t> m_share1;
	uint8_t m_screen_intensity = 0U;
	uint8_t m_video_disable = 0U;
	optional_device<adc0808_device> m_adc;
	optional_device<cpu_device> m_extra;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint16_t> m_paletteram;
	static const atari_motion_objects_config s_mob_config;
	static const atari_motion_objects_config s_guts_mob_config;

	void video_int_ack_w(uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	uint8_t adc_r(offs_t offset);
	void eprom_latch_w(uint8_t data);
	template<bool maincpu> void sync_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(guts_get_playfield_tile_info);
	uint32_t screen_update_eprom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_guts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette();
	void extra_map(address_map &map);
	void guts_map(address_map &map);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_EPROM_H
