// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Phil Bennett
/*************************************************************************

    Atari Cyberstorm hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CYBSTORM_H
#define MAME_INCLUDES_CYBSTORM_H

#pragma once

#include "atarijsa.h"
#include "machine/bankdev.h"
#include "atarivad.h"
#include "screen.h"
#include "tilemap.h"


class cybstorm_state : public driver_device
{
public:
	cybstorm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_jsa(*this, "jsa")
		, m_vad(*this, "vad")
		, m_vadbank(*this, "vadbank")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void init_cybstorm();
	void cybstorm(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t special_port1_r();
	void latch_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	TILEMAP_MAPPER_MEMBER(playfield_scan);

	uint32_t screen_update_cybstorm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void round2(machine_config &config);
	void main_map(address_map &map);
	void vadbank_map(address_map &map);

private:
	required_device<cpu_device> m_maincpu;
	optional_device<atari_jsa_iiis_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<address_map_bank_device> m_vadbank;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t m_latch_data = 0U;
	uint8_t m_alpha_tile_bank = 0U;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_CYBSTORM_H
