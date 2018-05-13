// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari GX2 hardware

*************************************************************************/
#ifndef MAME_INCLUDES_ATARIGX2_H
#define MAME_INCLUDES_ATARIGX2_H

#pragma once

#include "audio/atarijsa.h"
#include "machine/adc0808.h"
#include "machine/atarigen.h"
#include "machine/atarixga.h"
#include "video/atarirle.h"


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
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ32_MEMBER(special_port2_r);
	DECLARE_READ32_MEMBER(special_port3_r);
	DECLARE_READ8_MEMBER(a2d_data_r);
	DECLARE_WRITE32_MEMBER(latch_w);
	DECLARE_WRITE32_MEMBER(mo_command_w);
	DECLARE_WRITE32_MEMBER(atarigx2_protection_w);
	DECLARE_READ32_MEMBER(atarigx2_protection_r);
	DECLARE_READ32_MEMBER(rrreveng_prot_r);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(atarigx2_playfield_scan);
	uint32_t screen_update_atarigx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE16_MEMBER( atarigx2_mo_control_w );

	void atarigx2(machine_config &config);
	void main_map(address_map &map);

private:
	uint16_t          m_playfield_base;

	required_device<atari_jsa_iiis_device> m_jsa;
	optional_device<atari_xga_device> m_xga;

	required_shared_ptr<uint32_t> m_mo_command;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	required_device<adc0808_device> m_adc;

	uint16_t          m_current_control;
	uint8_t           m_playfield_tile_bank;
	uint8_t           m_playfield_color_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

	// LEGACY PROTECTION
	uint16_t          m_last_write;
	uint16_t          m_last_write_offset;
	uint32_t          m_protection_ram[0x1000];
};

#endif // MAME_INCLUDES_ATARIGX2_H
