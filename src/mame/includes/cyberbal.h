// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CYBERBAL_H
#define MAME_INCLUDES_CYBERBAL_H

#pragma once

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"
#include "sound/ym2151.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class cyberbal_base_state : public atarigen_state
{
protected:
	cyberbal_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_playfield(*this, "playfield"),
		m_alpha(*this, "alpha"),
		m_mob(*this, "mob")
	{ }

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	virtual void video_start() override;

	void scanline_update_one(screen_device &screen, int scanline, int i, tilemap_t &curplayfield, tilemap_device &curalpha);
	uint32_t update_one_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, atari_motion_objects_device &curmob, tilemap_t &curplayfield, tilemap_t &curalpha);

	static const atari_motion_objects_config s_mob_config;

	required_device<tilemap_device> m_playfield;
	required_device<tilemap_device> m_alpha;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t    m_current_slip[2];

private:
	uint8_t     m_playfield_palette_bank[2];
	uint16_t    m_playfield_xscroll[2];
	uint16_t    m_playfield_yscroll[2];
};


class cyberbal2p_state : public cyberbal_base_state
{
public:
	cyberbal2p_state(const machine_config &mconfig, device_type type, const char *tag) :
		cyberbal_base_state(mconfig, type, tag),
		m_jsa(*this, "jsa")
	{ }

	void cyberbal2p(machine_config &config);

protected:
	DECLARE_READ16_MEMBER(sound_state_r);

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// memory maps
	void cyberbal2p_map(address_map &map);

private:
	required_device<atari_jsa_ii_device> m_jsa;
};


class cyberbal_state : public cyberbal_base_state
{
public:
	cyberbal_state(const machine_config &mconfig, device_type type, const char *tag) :
		cyberbal_base_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_extracpu(*this, "extra"),
		m_daccpu(*this, "dac"),
		m_rdac(*this, "rdac"),
		m_ldac(*this, "ldac"),
		m_soundcomm(*this, "soundcomm"),
		m_ymsnd(*this, "ymsnd"),
		m_playfield2(*this, "playfield2"),
		m_alpha2(*this, "alpha2"),
		m_mob2(*this, "mob2"),
		m_rpalette(*this, "rpalette"),
		m_lscreen(*this, "lscreen"),
		m_rscreen(*this, "rscreen")
	{ }

	void init_cyberbalt();
	void cyberbal(machine_config &config);
	void cyberbal_base(machine_config &config);
	void cyberbalt(machine_config &config);

protected:
	DECLARE_WRITE16_MEMBER(p2_reset_w);
	TILE_GET_INFO_MEMBER(get_alpha2_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// sound-related
	DECLARE_READ8_MEMBER(special_port3_r);
	DECLARE_READ8_MEMBER(sound_6502_stat_r);
	DECLARE_WRITE8_MEMBER(sound_bank_select_w);
	DECLARE_READ8_MEMBER(sound_68k_6502_r);
	DECLARE_WRITE8_MEMBER(sound_68k_6502_w);
	DECLARE_WRITE16_MEMBER(io_68k_irq_ack_w);
	DECLARE_READ16_MEMBER(sound_68k_r);
	DECLARE_WRITE16_MEMBER(sound_68k_w);
	DECLARE_WRITE16_MEMBER(sound_68k_dac_w);
	INTERRUPT_GEN_MEMBER(sound_68k_irq_gen);
	void cyberbal_sound_reset();
	void update_sound_68k_interrupts();

	// memory maps
	void main_map(address_map &map);
	void extra_map(address_map &map);
	void sound_map(address_map &map);
	void sound_68k_map(address_map &map);

private:
	required_device<m6502_device> m_audiocpu;
	required_device<cpu_device> m_extracpu;
	required_device<cpu_device> m_daccpu;
	required_device<dac_word_interface> m_rdac;
	required_device<dac_word_interface> m_ldac;
	required_device<atari_sound_comm_device> m_soundcomm;
	required_device<ym2151_device> m_ymsnd;
	required_device<tilemap_device> m_playfield2;
	required_device<tilemap_device> m_alpha2;
	required_device<atari_motion_objects_device> m_mob2;
	required_device<palette_device> m_rpalette;
	required_device<screen_device> m_lscreen;
	required_device<screen_device> m_rscreen;

	uint8_t m_fast_68k_int;
	uint8_t m_io_68k_int;
	uint8_t m_sound_data_from_68k;
	uint8_t m_sound_data_from_6502;
	uint8_t m_sound_data_from_68k_ready;
	uint8_t m_sound_data_from_6502_ready;
};

#endif // MAME_INCLUDES_CYBERBAL_H
