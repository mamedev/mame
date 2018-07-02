// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Shuuz hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SHUUZ_H
#define MAME_INCLUDES_SHUUZ_H

#pragma once

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "video/atarivad.h"

class shuuz_state : public atarigen_state
{
public:
	shuuz_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_vad(*this, "vad")
	{ }

	void shuuz(machine_config &config);

private:
	virtual void update_interrupts() override;

	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_READ16_MEMBER(leta_r);
	DECLARE_READ16_MEMBER(special_port0_r);

	virtual void machine_start() override;

	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	required_device<atari_vad_device> m_vad;

	int m_cur[2];

	static const atari_motion_objects_config s_mob_config;
};

#endif // MAME_INCLUDES_SHUUZ_H
