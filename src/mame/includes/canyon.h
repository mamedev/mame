// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Canyon Bomber hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CANYON_H
#define MAME_INCLUDES_CANYON_H

#pragma once

#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "tilemap.h"

/* Discrete Sound Input Nodes */
#define CANYON_MOTOR1_DATA      NODE_01
#define CANYON_MOTOR2_DATA      NODE_02
#define CANYON_EXPLODE_DATA     NODE_03
#define CANYON_WHISTLE1_EN      NODE_04
#define CANYON_WHISTLE2_EN      NODE_05
#define CANYON_ATTRACT1_EN      NODE_06
#define CANYON_ATTRACT2_EN      NODE_07



class canyon_state : public driver_device
{
public:
	canyon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_outlatch(*this, "outlatch"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void canyon(machine_config &config);

protected:
	uint8_t canyon_switches_r(offs_t offset);
	uint8_t canyon_options_r(offs_t offset);
	void output_latch_w(offs_t offset, uint8_t data);
	void canyon_videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void canyon_palette(palette_device &palette) const;
	uint32_t screen_update_canyon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void canyon_motor_w(offs_t offset, uint8_t data);
	void canyon_explode_w(uint8_t data);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_bombs( bitmap_ind16 &bitmap, const rectangle &cliprect );

	virtual void video_start() override;
	void main_map(address_map &map);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	required_device<f9334_device> m_outlatch;
	required_device<discrete_sound_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*----------- defined in audio/canyon.c -----------*/
DISCRETE_SOUND_EXTERN( canyon_discrete );

#endif // MAME_INCLUDES_CANYON_H
