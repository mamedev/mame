// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/
#ifndef MAME_INCLUDES_BLSTROID_H
#define MAME_INCLUDES_BLSTROID_H

#pragma once

#include "machine/atarigen.h"
#include "machine/timer.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "tilemap.h"

class blstroid_state : public atarigen_state
{
public:
	enum
	{
		TIMER_IRQ_OFF = TID_ATARIGEN_LAST,
		TIMER_IRQ_ON
	};

	blstroid_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_playfield_tilemap(*this, "playfield"),
		m_jsa(*this, "jsa"),
		m_mob(*this, "mob"),
		m_priorityram(*this, "priorityram")
	{ }

	void init_blstroid();
	void blstroid(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	void scanline_int_ack_w(uint16_t data = 0);
	void video_int_ack_w(uint16_t data = 0);
	void halt_until_hblank_0_w(uint16_t data = 0);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_blstroid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_jsa_i_device> m_jsa;
	required_device<atari_motion_objects_device> m_mob;
	required_shared_ptr<uint16_t> m_priorityram;

	emu_timer *m_irq_off_timer = nullptr;
	emu_timer *m_irq_on_timer = nullptr;

	static const atari_motion_objects_config s_mob_config;

	bool m_scanline_int_state = false;
};

#endif // MAME_INCLUDES_BLSTROID_H
