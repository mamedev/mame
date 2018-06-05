// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G42 hardware

*************************************************************************/
#ifndef MAME_INCLUDES_ATARIG42_H
#define MAME_INCLUDES_ATARIG42_H

#pragma once

#include "audio/atarijsa.h"
#include "machine/atarigen.h"
#include "video/atarirle.h"
#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "machine/asic65.h"

class atarig42_state : public atarigen_state
{
public:
	atarig42_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_jsa(*this, "jsa"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_rle(*this, "rle"),
		m_asic65(*this, "asic65"),
		m_adc(*this, "adc"),
		m_mo_command(*this, "mo_command")
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE8_MEMBER(a2d_select_w);
	DECLARE_READ8_MEMBER(a2d_data_r);
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_WRITE16_MEMBER(mo_command_w);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(atarig42_playfield_scan);
	DECLARE_VIDEO_START(atarig42);
	uint32_t screen_update_atarig42(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atarig42(machine_config &config);
	void main_map(address_map &map);

	required_device<atari_jsa_iii_device> m_jsa;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;
	required_device<asic65_device> m_asic65;
	optional_device<adc0808_device> m_adc;

	uint16_t          m_playfield_base;

	uint16_t          m_current_control;
	uint8_t           m_playfield_tile_bank;
	uint8_t           m_playfield_color_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

	required_shared_ptr<uint16_t> m_mo_command;

	int             m_sloop_bank;
	int             m_sloop_next_bank;
	int             m_sloop_offset;
	int             m_sloop_state;
	uint16_t *        m_sloop_base;

	uint32_t          m_last_accesses[8];
};

class atarig42_0x200_state : public atarig42_state
{
public:
	using atarig42_state::atarig42_state;
	void init_roadriot();
	void atarig42_0x200(machine_config &config);

protected:
	DECLARE_READ16_MEMBER(roadriot_sloop_data_r);
	DECLARE_WRITE16_MEMBER(roadriot_sloop_data_w);
	void roadriot_sloop_tweak(int offset);
};

class atarig42_0x400_state : public atarig42_state
{
public:
	using atarig42_state::atarig42_state;
	void init_guardian();
	void atarig42_0x400(machine_config &config);

protected:
	DECLARE_READ16_MEMBER(guardians_sloop_data_r);
	DECLARE_WRITE16_MEMBER(guardians_sloop_data_w);
	void guardians_sloop_tweak(int offset);
};

#endif // MAME_INCLUDES_ATARIG42_H
