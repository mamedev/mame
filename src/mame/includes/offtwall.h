// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari "Round" hardware

*************************************************************************/
#ifndef MAME_INCLUDES_OFFTWALL_H
#define MAME_INCLUDES_OFFTWALL_H

#pragma once

#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "video/atarivad.h"
#include "screen.h"
#include "tilemap.h"

class offtwall_state : public driver_device
{
public:
	offtwall_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_mainram(*this, "mainram"),
		m_bankrom_base(*this, "maincpu")
	{ }

	void offtwall(machine_config &config);

	void init_offtwall();
	void init_offtwalc();

private:
	void io_latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bankswitch_r(offs_t offset);
	uint16_t bankrom_r(address_space &space, offs_t offset);
	uint16_t spritecache_count_r(offs_t offset);
	uint16_t unknown_verify_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_offtwall(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_shared_ptr<uint16_t> m_mainram;

	uint16_t *m_bankswitch_base = nullptr;
	required_region_ptr<uint16_t> m_bankrom_base;
	uint32_t m_bank_offset = 0;

	uint16_t *m_spritecache_count = nullptr;
	uint16_t *m_unknown_verify_base = nullptr;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_OFFTWALL_H
