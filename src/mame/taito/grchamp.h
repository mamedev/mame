// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino
/*************************************************************************

    Taito Grand Champ hardware

*************************************************************************/
#ifndef MAME_TAITO_GRCHAMP_H
#define MAME_TAITO_GRCHAMP_H

#pragma once

#include "machine/input_merger.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class grchamp_state : public driver_device
{
public:
	grchamp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundnmi(*this, "soundnmi"),
		m_radarram(*this, "radarram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_leftram(*this, "leftram"),
		m_rightram(*this, "rightram"),
		m_centerram(*this, "centerram"),
		m_digits(*this, "digit%u", 0U),
		m_led0(*this, "led0")
	{ }

	void grchamp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void cpu0_outputs_w(offs_t offset, uint8_t data);
	void led_board_w(offs_t offset, uint8_t data);
	void cpu1_outputs_w(offs_t offset, uint8_t data);
	uint8_t pc3259_0_r();
	uint8_t pc3259_1_r();
	uint8_t pc3259_2_r();
	uint8_t pc3259_3_r();
	uint8_t sub_to_main_comm_r();
	void main_to_sub_comm_w(offs_t offset, uint8_t data);
	uint8_t main_to_sub_comm_r(offs_t offset);
	uint8_t get_pc3259_bits(int offs);
	void left_w(offs_t offset, uint8_t data);
	void center_w(offs_t offset, uint8_t data);
	void right_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(soundlatch_w_cb);
	TIMER_CALLBACK_MEMBER(soundlatch_clear7_w_cb);
	uint8_t soundlatch_r();
	void soundlatch_clear7_w(uint8_t data);
	uint8_t soundlatch_flags_r();
	void portA_0_w(uint8_t data);
	void portB_0_w(uint8_t data);
	void portA_2_w(uint8_t data);
	void portB_2_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_left_tile_info);
	TILE_GET_INFO_MEMBER(get_right_tile_info);
	TILE_GET_INFO_MEMBER(get_center_tile_info);
	TILEMAP_MAPPER_MEMBER(get_memory_offset);

	void grchamp_palette(palette_device &palette) const;
	INTERRUPT_GEN_MEMBER(cpu0_interrupt);
	INTERRUPT_GEN_MEMBER(cpu1_interrupt);
	TIMER_CALLBACK_MEMBER(main_to_sub_comm_sync_w);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_objects(int y, uint8_t *objdata);
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void sub_portmap(address_map &map) ATTR_COLD;

	uint8_t       m_cpu0_out[16]{};
	uint8_t       m_cpu1_out[16]{};

	uint8_t       m_comm_latch = 0U;
	uint8_t       m_comm_latch2[4]{};

	uint16_t      m_ledlatch = 0U;
	uint8_t       m_ledaddr = 0U;
	uint16_t      m_ledram[8]{};

	uint8_t       m_soundlatch_data = 0U;
	bool          m_soundlatch_flag = false;

	uint16_t      m_collide = 0U;
	uint8_t       m_collmode = 0U;

	bitmap_ind16 m_work_bitmap{};
	tilemap_t *m_text_tilemap = nullptr;
	tilemap_t *m_left_tilemap = nullptr;
	tilemap_t *m_center_tilemap = nullptr;
	tilemap_t *m_right_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<input_merger_device> m_soundnmi;

	required_shared_ptr<uint8_t> m_radarram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_leftram;
	required_shared_ptr<uint8_t> m_rightram;
	required_shared_ptr<uint8_t> m_centerram;
	output_finder<8> m_digits;
	output_finder<> m_led0;
};

/* Discrete Sound Input Nodes */
#define GRCHAMP_ENGINE_CS_EN                NODE_01
#define GRCHAMP_SIFT_DATA                   NODE_02
#define GRCHAMP_ATTACK_UP_DATA              NODE_03
#define GRCHAMP_IDLING_EN                   NODE_04
#define GRCHAMP_FOG_EN                      NODE_05
#define GRCHAMP_PLAYER_SPEED_DATA           NODE_06
#define GRCHAMP_ATTACK_SPEED_DATA           NODE_07
#define GRCHAMP_A_DATA                      NODE_08
#define GRCHAMP_B_DATA                      NODE_09

/*----------- defined in audio/grchamp.c -----------*/

DISCRETE_SOUND_EXTERN( grchamp_discrete );

#endif // MAME_TAITO_GRCHAMP_H
