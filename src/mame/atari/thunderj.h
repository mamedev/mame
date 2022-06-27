// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/
#ifndef MAME_INCLUDES_THUNDERJ_H
#define MAME_INCLUDES_THUNDERJ_H

#pragma once

#include "atarijsa.h"
#include "atarimo.h"
#include "atarivad.h"
#include "screen.h"
#include "tilemap.h"

class thunderj_state : public driver_device
{
public:
	thunderj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_maincpu(*this, "maincpu"),
		m_extra(*this, "extra")
	{ }

	void thunderj(machine_config &config);

	void init_thunderj();

private:
	virtual void machine_start() override;
	DECLARE_WRITE_LINE_MEMBER(scanline_int_write_line);
	uint16_t special_port2_r();
	void latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	uint32_t screen_update_thunderj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void extra_map(address_map &map);
	void main_map(address_map &map);

	required_device<screen_device> m_screen;
	required_device<atari_jsa_ii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_extra;

	uint8_t           m_alpha_tile_bank = 0;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_THUNDERJ_H
