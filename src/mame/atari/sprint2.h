// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Sprint hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SPRINT2_H
#define MAME_INCLUDES_SPRINT2_H

#pragma once

#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

// Discrete Sound Input Nodes
#define SPRINT2_SKIDSND1_EN        NODE_01
#define SPRINT2_SKIDSND2_EN        NODE_02
#define SPRINT2_MOTORSND1_DATA     NODE_03
#define SPRINT2_MOTORSND2_DATA     NODE_04
#define SPRINT2_CRASHSND_DATA      NODE_05
#define SPRINT2_ATTRACT_EN         NODE_06
#define SPRINT2_NOISE_RESET        NODE_07

#define DOMINOS_FREQ_DATA          SPRINT2_MOTORSND1_DATA
#define DOMINOS_AMP_DATA           SPRINT2_CRASHSND_DATA
#define DOMINOS_TUMBLE_EN          SPRINT2_SKIDSND1_EN
#define DOMINOS_ATTRACT_EN         SPRINT2_ATTRACT_EN


class sprint2_state : public driver_device
{
public:
	sprint2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_outlatch(*this, "outlatch"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_in(*this, { "INA", "INB" }),
		m_dials(*this, "DIAL_P%u", 1U),
		m_gears(*this, "GEAR_P%u", 1U),
		m_dsw(*this, "DSW"),
		m_gear_sel(*this, "P%ugear", 1U)
	{ }

	void sprint1(machine_config &config);
	void sprint2(machine_config &config);
	void dominos4(machine_config &config);
	void dominos(machine_config &config);

	void init_sprint1();
	void init_sprint2();
	void init_dominos();
	void init_dominos4();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_video_ram;
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<f9334_device> m_outlatch;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<2> m_in;
	optional_ioport_array<2> m_dials, m_gears;
	required_ioport m_dsw;
	output_finder<2> m_gear_sel;

	uint8_t m_steering[2]{};
	uint8_t m_gear[2]{};
	uint8_t m_game = 0;
	uint8_t m_dial[2]{};
	tilemap_t* m_bg_tilemap = nullptr;
	bitmap_ind16 m_helper;
	uint8_t m_collision[2]{};

	uint8_t wram_r(offs_t offset);
	uint8_t dip_r(offs_t offset);
	uint8_t input_A_r(offs_t offset);
	uint8_t input_B_r(offs_t offset);
	uint8_t sync_r();
	template <uint8_t Which> uint8_t steering_r();
	template <uint8_t Which> void steering_reset_w(uint8_t data);
	void wram_w(offs_t offset, uint8_t data);
	void output_latch_w(offs_t offset, uint8_t data);
	void video_ram_w(offs_t offset, uint8_t data);
	void noise_reset_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	INTERRUPT_GEN_MEMBER(irq);
	uint8_t collision_check(rectangle& rect);
	inline int get_sprite_code(int n);
	inline int get_sprite_x(int n);
	inline int get_sprite_y(int n);
	uint8_t service_mode();

	void main_map(address_map &map);
};

//----------- defined in audio/sprint2.cpp -----------
DISCRETE_SOUND_EXTERN( sprint2_discrete );
DISCRETE_SOUND_EXTERN( sprint1_discrete );
DISCRETE_SOUND_EXTERN( dominos_discrete );

#endif // MAME_INCLUDES_SPRINT2_H
