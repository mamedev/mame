// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari System 1 hardware

*************************************************************************/
#ifndef MAME_INCLUDES_ATARISY1_H
#define MAME_INCLUDES_ATARISY1_H

#pragma once

#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/atarigen.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "sound/tms5220.h"
#include "sound/ym2151.h"
#include "video/atarimo.h"
#include "emupal.h"

class atarisy1_state : public atarigen_state
{
public:
	atarisy1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_soundcomm(*this, "soundcomm")
		, m_bankselect(*this, "bankselect")
		, m_mob(*this, "mob")
		, m_palette(*this, "palette")
		, m_adc(*this, "adc")
		, m_ajsint(*this, "ajsint")
		, m_playfield_tilemap(*this, "playfield")
		, m_alpha_tilemap(*this, "alpha")
		, m_yscroll_reset_timer(*this, "yreset_timer")
		, m_scanline_timer(*this, "scan_timer")
		, m_int3off_timer(*this, "int3off_timer")
		, m_tms(*this, "tms")
		, m_outlatch(*this, "outlatch")
	{ }

	required_device<cpu_device> m_audiocpu;
	required_device<atari_sound_comm_device> m_soundcomm;

	required_shared_ptr<uint16_t> m_bankselect;
	required_device<atari_motion_objects_device> m_mob;
	required_device<palette_device> m_palette;

	uint8_t           m_joystick_type;
	uint8_t           m_trackball_type;

	optional_device<adc0808_device> m_adc;
	optional_device<input_merger_device> m_ajsint;
	uint8_t           m_joystick_int;

	/* playfield parameters */
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	uint16_t          m_playfield_lookup[256];
	uint8_t           m_playfield_tile_bank;
	uint16_t          m_playfield_priority_pens;
	required_device<timer_device> m_yscroll_reset_timer;

	/* INT3 tracking */
	int             m_next_timer_scanline;
	required_device<timer_device> m_scanline_timer;
	required_device<timer_device> m_int3off_timer;

	/* speech */
	required_device<tms5220_device> m_tms;

	required_device<ls259_device> m_outlatch;

	/* graphics bank tracking */
	uint8_t           m_bank_gfx[3][8];
	uint8_t           m_bank_color_shift[MAX_GFX_ELEMENTS];

	uint8_t           m_cur[2][2];
	virtual void update_interrupts() override;
	template<int Input> DECLARE_READ8_MEMBER(digital_joystick_r);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE8_MEMBER(adc_w);
	DECLARE_READ16_MEMBER(trakball_r);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_right_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_left_w);
	DECLARE_WRITE8_MEMBER(via_pa_w);
	DECLARE_READ8_MEMBER(via_pa_r);
	DECLARE_WRITE8_MEMBER(via_pb_w);
	DECLARE_READ8_MEMBER(via_pb_r);
	void init_roadblst();
	void init_peterpak();
	void init_marble();
	void init_roadrunn();
	void init_indytemp();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(atarisy1);
	DECLARE_MACHINE_RESET(atarisy1);
	DECLARE_VIDEO_START(atarisy1);
	uint32_t screen_update_atarisy1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(joystick_int);
	TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_reset_yscroll_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_int3off_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_int3_callback);
	void update_timers(int scanline);
	void decode_gfx(uint16_t *pflookup, uint16_t *molookup);
	int get_bank(uint8_t prom1, uint8_t prom2, int bpp);
	DECLARE_READ16_MEMBER( atarisy1_int3state_r );
	DECLARE_WRITE16_MEMBER( atarisy1_spriteram_w );
	DECLARE_WRITE16_MEMBER( atarisy1_bankselect_w );
	DECLARE_WRITE16_MEMBER( atarisy1_xscroll_w );
	DECLARE_WRITE16_MEMBER( atarisy1_yscroll_w );
	DECLARE_WRITE16_MEMBER( atarisy1_priority_w );

	static const atari_motion_objects_config s_mob_config;
	void add_adc(machine_config &config);
	void atarisy1(machine_config &config);
	void indytemp(machine_config &config);
	void roadb110(machine_config &config);
	void peterpak(machine_config &config);
	void roadrunn(machine_config &config);
	void roadb109(machine_config &config);
	void marble(machine_config &config);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_ATARISY1_H
