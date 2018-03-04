// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/
#ifndef MAME_INCLUDES_THUNDERJ_H
#define MAME_INCLUDES_THUNDERJ_H

#pragma once

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class thunderj_state : public atarigen_state
{
public:
	thunderj_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_jsa(*this, "jsa"),
		m_vad(*this, "vad"),
		m_extra(*this, "extra")
	{ }

	DECLARE_DRIVER_INIT(thunderj);
	void thunderj(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void update_interrupts() override;
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(latch_w);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	uint32_t screen_update_thunderj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void extra_map(address_map &map);
	void main_map(address_map &map);

private:
	required_device<atari_jsa_ii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<cpu_device> m_extra;

	uint8_t           m_alpha_tile_bank;

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_THUNDERJ_H
