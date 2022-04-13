// license:BSD-3-Clause
// copyright-holders:Sebastien Monassa
/*************************************************************************

    Atari Video Pinball hardware

*************************************************************************/
#ifndef MAME_INCLUDES_VIDEOPIN_H
#define MAME_INCLUDES_VIDEOPIN_H

#pragma once

#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/* Discrete Sound Input Nodes */
#define VIDEOPIN_OCTAVE_DATA    NODE_01
#define VIDEOPIN_NOTE_DATA      NODE_02
#define VIDEOPIN_BELL_EN        NODE_03
#define VIDEOPIN_BONG_EN        NODE_04
#define VIDEOPIN_ATTRACT_EN     NODE_05
#define VIDEOPIN_VOL_DATA       NODE_06


class videopin_state : public driver_device
{
public:
	videopin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram"),
		m_leds(*this, "LED%02u", 1U)
	{ }

	void videopin(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	enum
	{
		TIMER_INTERRUPT
	};

	uint8_t misc_r();
	void led_w(uint8_t data);
	void ball_w(uint8_t data);
	void video_ram_w(offs_t offset, uint8_t data);
	void out1_w(uint8_t data);
	void out2_w(uint8_t data);
	void note_dvsr_w(uint8_t data);

	TILEMAP_MAPPER_MEMBER(get_memory_offset);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void main_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void update_plunger();
	double calc_plunger_pos();

	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_video_ram;
	output_finder<32> m_leds;

	attotime m_time_pushed;
	attotime m_time_released;
	uint8_t m_prev = 0;
	uint8_t m_mask = 0;
	int m_ball_x = 0;
	int m_ball_y = 0;
	tilemap_t* m_bg_tilemap = nullptr;
	emu_timer *m_interrupt_timer = nullptr;
};

/*----------- defined in audio/videopin.c -----------*/
DISCRETE_SOUND_EXTERN( videopin_discrete );

#endif // MAME_INCLUDES_VIDEOPIN_H
