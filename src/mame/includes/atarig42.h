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
#include "machine/timer.h"
#include "tilemap.h"

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
	virtual void video_start() override;
	void video_int_ack_w(uint16_t data = 0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	void a2d_select_w(offs_t offset, uint8_t data);
	uint8_t a2d_data_r(offs_t offset);
	void io_latch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mo_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(atarig42_playfield_scan);
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
	uint16_t roadriot_sloop_data_r(offs_t offset);
	void roadriot_sloop_data_w(offs_t offset, uint16_t data);
	void roadriot_sloop_tweak(int offset);
};

class atarig42_0x400_state : public atarig42_state
{
public:
	using atarig42_state::atarig42_state;
	void init_guardian();
	void atarig42_0x400(machine_config &config);

protected:
	uint16_t guardians_sloop_data_r(offs_t offset);
	void guardians_sloop_data_w(offs_t offset, uint16_t data);
	void guardians_sloop_tweak(int offset);
};

#endif // MAME_INCLUDES_ATARIG42_H
