// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CYBERBAL_H
#define MAME_INCLUDES_CYBERBAL_H

#pragma once

#include "slapstic.h"
#include "atarijsa.h"
#include "atarisac.h"
#include "atarimo.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class cyberbal_base_state : public driver_device
{
protected:
	cyberbal_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
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

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<tilemap_device> m_playfield;
	required_device<tilemap_device> m_alpha;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t    m_current_slip[2]{};

private:
	uint8_t     m_playfield_palette_bank[2]{};
	uint16_t    m_playfield_xscroll[2]{};
	uint16_t    m_playfield_yscroll[2]{};
};


class cyberbal2p_state : public cyberbal_base_state
{
public:
	cyberbal2p_state(const machine_config &mconfig, device_type type, const char *tag) :
		cyberbal_base_state(mconfig, type, tag),
		m_screen(*this, "screen"),
		m_jsa(*this, "jsa")
	{ }

	void cyberbal2p(machine_config &config);

protected:
	uint16_t sound_state_r();

	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// memory maps
	void cyberbal2p_map(address_map &map);

private:
	DECLARE_WRITE_LINE_MEMBER(video_int_write_line);
	void video_int_ack_w(uint16_t data = 0);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);

	required_device<screen_device> m_screen;
	required_device<atari_jsa_ii_device> m_jsa;
};


class cyberbal_state : public cyberbal_base_state
{
public:
	cyberbal_state(const machine_config &mconfig, device_type type, const char *tag) :
		cyberbal_base_state(mconfig, type, tag),
		m_slapstic(*this, "slapstic"),
		m_slapstic_bank(*this, "slapstic_bank"),
		m_extracpu(*this, "extra"),
		m_sac(*this, "sac"),
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
	void p2_reset_w(uint16_t data);
	TILE_GET_INFO_MEMBER(get_alpha2_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// memory maps
	void main_map(address_map &map);
	void tournament_map(address_map &map);
	void extra_map(address_map &map);

private:
	DECLARE_WRITE_LINE_MEMBER(video_int_write_line);
	void video_int_ack_w(uint16_t data = 0);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);

	optional_device<atari_slapstic_device> m_slapstic;
	optional_memory_bank m_slapstic_bank;
	required_device<cpu_device> m_extracpu;
	required_device<atari_sac_device> m_sac;
	required_device<tilemap_device> m_playfield2;
	required_device<tilemap_device> m_alpha2;
	required_device<atari_motion_objects_device> m_mob2;
	required_device<palette_device> m_rpalette;
	required_device<screen_device> m_lscreen;
	required_device<screen_device> m_rscreen;

};

#endif // MAME_INCLUDES_CYBERBAL_H
