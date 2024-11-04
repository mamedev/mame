// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari System 1 hardware

*************************************************************************/
#ifndef MAME_ATARI_ATARISY1_H
#define MAME_ATARI_ATARISY1_H

#pragma once

#include "machine/6522via.h"
#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "slapstic.h"
#include "machine/timer.h"
#include "sound/tms5220.h"
#include "sound/ymopm.h"
#include "atarimo.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class atarisy1_state : public driver_device
{
public:
	atarisy1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_mainlatch(*this, "mainlatch")
		, m_slapstic(*this, "slapstic")
		, m_slapstic_bank(*this, "slapstic_bank")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_mob(*this, "mob")
		, m_palette(*this, "palette")
		, m_adc(*this, "adc")
		, m_ajsint(*this, "ajsint")
		, m_playfield_tilemap(*this, "playfield")
		, m_alpha_tilemap(*this, "alpha")
		, m_xscroll(*this, "xscroll")
		, m_yscroll(*this, "yscroll")
		, m_yscroll_reset_timer(*this, "yreset_timer")
		, m_scanline_timer(*this, "scan_timer")
		, m_int3off_timer(*this, "int3off_timer")
		, m_tms(*this, "tms")
		, m_outlatch(*this, "outlatch")
		, m_via(*this, "via")
	{ }

	void indytemp(machine_config &config);
	void roadb110(machine_config &config);
	void peterpak(machine_config &config);
	void roadrunn(machine_config &config);
	void roadb109(machine_config &config);
	void marble(machine_config &config);

	void init_roadblst();
	void init_peterpak();
	void init_marble();
	void init_roadrunn();
	void init_indytemp();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<atari_slapstic_device> m_slapstic;
	required_memory_bank m_slapstic_bank;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_motion_objects_device> m_mob;
	required_device<palette_device> m_palette;

	uint8_t           m_joystick_type = 0;
	uint8_t           m_trackball_type = 0;

	optional_device<adc0808_device> m_adc;
	optional_device<input_merger_device> m_ajsint;

	// playfield parameters
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_shared_ptr<uint16_t> m_xscroll;
	required_shared_ptr<uint16_t> m_yscroll;
	uint16_t          m_playfield_lookup[256]{};
	uint8_t           m_playfield_tile_bank = 0;
	uint16_t          m_playfield_priority_pens = 0;
	required_device<timer_device> m_yscroll_reset_timer;

	// INT3 tracking
	int             m_next_timer_scanline = 0;
	required_device<timer_device> m_scanline_timer;
	required_device<timer_device> m_int3off_timer;
	uint8_t           m_scanline_int_state = 0;

	// speech
	optional_device<tms5220_device> m_tms;

	required_device<ls259_device> m_outlatch;
	optional_device<via6522_device> m_via;

	// graphics bank tracking
	uint8_t           m_bank_gfx[3][8]{};
	uint8_t           m_bank_color_shift[MAX_GFX_ELEMENTS]{};
	uint8_t           m_bankselect = 0;

	uint8_t           m_cur[2][2]{};

	void video_int_ack_w(uint8_t data = 0);
	template<int Input> uint8_t digital_joystick_r();
	uint8_t adc_r(offs_t offset);
	void adc_w(offs_t offset, uint8_t data);
	uint16_t trakball_r(offs_t offset);
	uint8_t switch_6502_r();
	void coin_counter_right_w(int state);
	void coin_counter_left_w(int state);
	void via_pb_w(uint8_t data);
	uint8_t via_pb_r();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_atarisy1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(reset_yscroll_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(int3off_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(int3_callback);
	virtual void update_timers(int scanline);
	void decode_gfx(uint16_t *pflookup, uint16_t *molookup);
	int get_bank(uint8_t prom1, uint8_t prom2, int bpp);
	uint16_t atarisy1_int3state_r();
	void atarisy1_spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bankselect_w(uint8_t data);
	void atarisy1_xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void atarisy1_yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void atarisy1_priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	static const atari_motion_objects_config s_mob_config;
	void add_adc(machine_config &config);
	void add_speech(machine_config &config);
	void atarisy1(machine_config &config);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_ext_map(address_map &map) ATTR_COLD;

	void init_slapstic();
};

class atarisy1r_state : public atarisy1_state
{
public:
	using atarisy1_state::atarisy1_state;

	virtual void update_timers(int scanline) override;
};

#endif // MAME_ATARI_ATARISY1_H
