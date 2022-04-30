// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SKULLXBO_H
#define MAME_INCLUDES_SKULLXBO_H

#pragma once

#include "machine/atarigen.h"
#include "machine/timer.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "tilemap.h"

class skullxbo_state : public atarigen_state
{
public:
	skullxbo_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_jsa(*this, "jsa"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_xscroll(*this, "xscroll"),
		m_yscroll(*this, "yscroll"),
		m_mob(*this, "mob"),
		m_playfield_latch(-1),
		m_scanline_int_state(0)
	{ }

	void skullxbo(machine_config &config);

	void init_skullxbo();

protected:
	virtual void machine_start() override;

private:
	void skullxbo_halt_until_hblank_0_w(uint16_t data);
	void skullxbo_mobwr_w(offs_t offset, uint16_t data);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	void playfield_latch_w(uint16_t data);
	void playfield_latched_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update_skullxbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_interrupt);
	void scanline_int_ack_w(uint16_t data = 0);
	void video_int_ack_w(uint16_t data = 0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	void skullxbo_scanline_update(int scanline);
	void skullxbo_xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void skullxbo_yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void skullxbo_mobmsb_w(offs_t offset, uint16_t data);

	void main_map(address_map &map);

	required_device<atari_jsa_ii_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_shared_ptr<uint16_t> m_xscroll;
	required_shared_ptr<uint16_t> m_yscroll;
	required_device<atari_motion_objects_device> m_mob;
	int m_playfield_latch;

	static const atari_motion_objects_config s_mob_config;

	emu_timer *m_scanline_int_timer = nullptr;
	bool m_scanline_int_state;
};

#endif // MAME_INCLUDES_SKULLXBO_H
