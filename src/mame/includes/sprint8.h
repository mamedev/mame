// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#ifndef MAME_INCLUDES_SPRINT8_H
#define MAME_INCLUDES_SPRINT8_H

#pragma once

#include "machine/74259.h"
#include "machine/timer.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class sprint8_state : public driver_device
{
public:
	sprint8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram")
	{ }

	void sprint8(machine_config &config);

private:
	DECLARE_READ8_MEMBER(collision_r);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(lockout_w);
	DECLARE_WRITE_LINE_MEMBER(int_reset_w);
	DECLARE_WRITE_LINE_MEMBER(team_w);
	DECLARE_WRITE8_MEMBER(video_ram_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void sprint8_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_CALLBACK_MEMBER(collision_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(input_callback);

	void set_pens();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_collision(int n);
	void sprint8_audio(machine_config &config);
	void sprint8_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<discrete_sound_device> m_discrete;

	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_pos_h_ram;
	required_shared_ptr<uint8_t> m_pos_v_ram;
	required_shared_ptr<uint8_t> m_pos_d_ram;

	int m_steer_dir[8];
	int m_steer_flag[8];
	int m_collision_reset;
	int m_collision_index;
	uint8_t m_dial[8];
	int m_team;

	tilemap_t* m_tilemap1;
	tilemap_t* m_tilemap2;
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	emu_timer *m_collision_timer;
};

/*----------- defined in audio/sprint8.c -----------*/
DISCRETE_SOUND_EXTERN( sprint8_discrete );

#endif // MAME_INCLUDES_SPRINT8_H
