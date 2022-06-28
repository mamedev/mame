// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Drag Race hardware

*************************************************************************/
#ifndef MAME_INCLUDES_DRAGRACE_H
#define MAME_INCLUDES_DRAGRACE_H

#pragma once

#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

// Discrete Sound Input Nodes
#define DRAGRACE_SCREECH1_EN    NODE_01
#define DRAGRACE_SCREECH2_EN    NODE_02
#define DRAGRACE_LOTONE_EN      NODE_03
#define DRAGRACE_HITONE_EN      NODE_04
#define DRAGRACE_EXPLODE1_EN    NODE_05
#define DRAGRACE_EXPLODE2_EN    NODE_06
#define DRAGRACE_MOTOR1_DATA    NODE_07
#define DRAGRACE_MOTOR2_DATA    NODE_08
#define DRAGRACE_MOTOR1_EN      NODE_80
#define DRAGRACE_MOTOR2_EN      NODE_81
#define DRAGRACE_KLEXPL1_EN     NODE_82
#define DRAGRACE_KLEXPL2_EN     NODE_83
#define DRAGRACE_ATTRACT_EN     NODE_09


class dragrace_state : public driver_device
{
public:
	dragrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_discrete(*this, "discrete"),
		m_playfield_ram(*this, "playfield_ram"),
		m_position_ram(*this, "position_ram"),
		m_p(*this, "P%u", 1U),
		m_dial(*this, "DIAL%u", 1U),
		m_in(*this, "IN%u", 0U),
		m_gear_sel(*this, "P%ugear", 1U),
		m_tacho_sel(*this, "tachometer%u", 1U)
	{
	}

	void dragrace(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	void speed1_w(uint8_t data);
	void speed2_w(uint8_t data);
	uint8_t input_r(offs_t offset);
	uint8_t steering_r();
	uint8_t scanline_r();
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(frame_callback);

	void main_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<discrete_sound_device> m_discrete;

	// memory pointers
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_position_ram;

	// inputs
	required_ioport_array<2> m_p, m_dial;
	required_ioport_array<3> m_in;

	// outputs
	output_finder<2> m_gear_sel, m_tacho_sel;

	// video-related
	tilemap_t  *m_bg_tilemap = nullptr;

	// misc
	uint8_t       m_gear[2]{};
};

//----------- defined in audio/dragrace.cpp -----------
DISCRETE_SOUND_EXTERN( dragrace_discrete );

#endif // MAME_INCLUDES_DRAGRACE_H
