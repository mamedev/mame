// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Escape hardware

*************************************************************************/
#ifndef MAME_INCLUDES_EPROM_H
#define MAME_INCLUDES_EPROM_H

#pragma once

#include "machine/adc0808.h"
#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "emupal.h"

class eprom_state : public atarigen_state
{
public:
	eprom_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
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
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ16_MEMBER(special_port1_r);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE16_MEMBER(eprom_latch_w);
	template<bool maincpu> DECLARE_WRITE16_MEMBER(sync_w);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(guts_get_playfield_tile_info);
	DECLARE_VIDEO_START(eprom);
	DECLARE_VIDEO_START(guts);
	uint32_t screen_update_eprom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_guts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette();
	void extra_map(address_map &map);
	void guts_map(address_map &map);
	void main_map(address_map &map);

private:
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_base_device> m_jsa;
	required_shared_ptr<uint16_t> m_share1;
	int             m_screen_intensity;
	int             m_video_disable;
	optional_device<adc0808_device> m_adc;
	optional_device<cpu_device> m_extra;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint16_t> m_paletteram;
	static const atari_motion_objects_config s_mob_config;
	static const atari_motion_objects_config s_guts_mob_config;
};

#endif // MAME_INCLUDES_EPROM_H
