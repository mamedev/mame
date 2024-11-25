// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari GX2 hardware

*************************************************************************/
#ifndef MAME_ATARI_ATARIGX2_H
#define MAME_ATARI_ATARIGX2_H

#pragma once

#include "atarijsa.h"
#include "machine/adc0808.h"
#include "atarigen.h"
#include "atarixga.h"
#include "machine/timer.h"
#include "atarirle.h"
#include "tilemap.h"


class atarigx2_state : public atarigen_state
{
public:
	atarigx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_jsa(*this, "jsa")
		, m_xga(*this, "xga")
		, m_mo_command(*this, "mo_command")
		, m_playfield_tilemap(*this, "playfield")
		, m_alpha_tilemap(*this, "alpha")
		, m_rle(*this, "rle")
		, m_adc(*this, "adc")
	{ }

	void init_spclords();
	void init_rrreveng();
	void init_motofren();
	void atarigx2_0x200(machine_config &config);
	void atarigx2_0x400(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	void video_int_ack_w(uint32_t data = 0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	uint32_t special_port2_r();
	uint32_t special_port3_r();
	uint8_t a2d_data_r(offs_t offset);
	void latch_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void mo_command_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void atarigx2_protection_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t atarigx2_protection_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t rrreveng_prot_r();
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(atarigx2_playfield_scan);
	uint32_t screen_update_atarigx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atarigx2_mo_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void atarigx2(machine_config &config);
	void main_map(address_map &map) ATTR_COLD;

private:
	uint16_t          m_playfield_base = 0U;

	required_device<atari_jsa_iiis_device> m_jsa;
	optional_device<atari_xga_device> m_xga;

	required_shared_ptr<uint32_t> m_mo_command;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	required_device<adc0808_device> m_adc;

	uint16_t          m_current_control = 0U;
	uint8_t           m_playfield_tile_bank = 0U;
	uint8_t           m_playfield_color_bank = 0U;
	uint16_t          m_playfield_xscroll = 0U;
	uint16_t          m_playfield_yscroll = 0U;

	// LEGACY PROTECTION
	uint16_t          m_last_write = 0U;
	uint16_t          m_last_write_offset = 0U;
	uint32_t          m_protection_ram[0x1000]{};
};

#endif // MAME_ATARI_ATARIGX2_H
