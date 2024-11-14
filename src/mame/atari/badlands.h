// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/
#ifndef MAME_ATARI_BADLANDS_H
#define MAME_ATARI_BADLANDS_H

#pragma once

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/eeprompar.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "machine/timer.h"
#include "sound/ymopm.h"
#include "atarimo.h"

#include "speaker.h"
#include "tilemap.h"

/*----------- defined in machine/badlands.cpp -----------*/

//extern const gfx_layout badlands_molayout;

INPUT_PORTS_EXTERN(badlands);


class badlands_state : public driver_device
{
public:
	badlands_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_playfield_tilemap(*this, "playfield")
		, m_screen(*this, "screen")
		, m_soundlatch(*this, "soundlatch")
		, m_mainlatch(*this, "mainlatch")
		, m_ymsnd(*this, "ymsnd")
		, m_mob(*this, "mob")
	{ }

	void badlands(machine_config &config);
	void init_badlands();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<screen_device> m_screen;

	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	void video_int_ack_w(uint16_t data);
	void badlands_pf_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

private:
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_mainlatch;
	optional_device<ym2151_device> m_ymsnd;
	optional_device<atari_motion_objects_device> m_mob;

	uint16_t sound_busy_r();
	void sound_reset_w(uint16_t data);
	uint16_t pedal_0_r();
	uint16_t pedal_1_r();

	uint8_t audio_io_r();
	void audio_io_w(uint8_t data);
	uint8_t audio_irqack_r();
	void audio_irqack_w(uint8_t data);

	uint32_t screen_update_badlands(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_int);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_scanline);

	static const atari_motion_objects_config s_mob_config;
	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

private:
	uint8_t           m_pedal_value[2]{};
	uint8_t           m_playfield_tile_bank = 0U;
};

class badlandsbl_state : public badlands_state
{
public:
	badlandsbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: badlands_state(mconfig, type, tag),
		  m_b_sharedram(*this, "b_sharedram"),
		  m_spriteram(*this, "spriteram")
	{}

	uint8_t bootleg_shared_r(offs_t offset);
	void bootleg_shared_w(offs_t offset, uint8_t data);
	void bootleg_main_irq_w(uint8_t data);
	uint16_t badlandsb_unk_r();
	uint8_t sound_response_r();
	TIMER_DEVICE_CALLBACK_MEMBER(bootleg_sound_scanline);

	void badlandsb(machine_config &config);
	void bootleg_map(address_map &map) ATTR_COLD;
	void bootleg_audio_map(address_map &map) ATTR_COLD;
	uint32_t screen_update_badlandsbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_b_sharedram;
	required_shared_ptr<uint16_t> m_spriteram;

	uint8_t m_sound_response = 0U;
};


#endif // MAME_ATARI_BADLANDS_H


