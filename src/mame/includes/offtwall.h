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
		m_bankrom_base(*this, "bankrom_base")
	{ }

	void offtwall(machine_config &config);

	void init_offtwall();
	void init_offtwalc();

private:
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_READ16_MEMBER(bankswitch_r);
	DECLARE_READ16_MEMBER(bankrom_r);
	DECLARE_READ16_MEMBER(spritecache_count_r);
	DECLARE_READ16_MEMBER(unknown_verify_r);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_offtwall(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_shared_ptr<uint16_t> m_mainram;

	uint16_t *m_bankswitch_base;
	required_shared_ptr<uint16_t> m_bankrom_base;
	uint32_t m_bank_offset;

	uint16_t *m_spritecache_count;
	uint16_t *m_unknown_verify_base;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_OFFTWALL_H
