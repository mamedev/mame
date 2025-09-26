// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
#ifndef MAME_TATSUMI_TATSUMI_H
#define MAME_TATSUMI_TATSUMI_H

#pragma once

#include "tzbx15_sprites.h"

#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "tilemap.h"

class tatsumi_state : public driver_device
{
public:
	tatsumi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ym2151(*this, "ymsnd")
		, m_oki(*this, "oki")
		, m_sprites(*this, "sprites")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_sharedram(*this, "sharedram")
		, m_sprite_control_ram(*this, "obj_ctrl_ram")
		, m_spriteram(*this, "spriteram")
		, m_mainregion(*this, "master_rom")
		, m_subregion(*this, "slave_rom")
	{ }

	void init_tatsumi();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<ym2151_device> m_ym2151;
	required_device<okim6295_device> m_oki;
	required_device<tzbx15_device> m_sprites;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint16_t> m_videoram;
	optional_shared_ptr<uint16_t> m_sharedram;
	required_shared_ptr<uint16_t> m_sprite_control_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_memory_region m_mainregion;
	required_memory_region m_subregion;

	static constexpr int CLOCK_1 = 16'000'000;
	static constexpr int CLOCK_2 = 50'000'000;

	INTERRUPT_GEN_MEMBER(v30_interrupt);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	void hd6445_crt_w(offs_t offset, uint8_t data);

	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tatsumi_v30_68000_r(offs_t offset);
	void tatsumi_v30_68000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tatsumi_sprite_control_r(offs_t offset);
	void tatsumi_sprite_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tatsumi_reset();
	void apply_shadow_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &shadow_bitmap, uint8_t xor_output);

	tilemap_t *m_tx_layer;

	uint16_t m_control_word;
	uint8_t m_last_control;
	uint8_t m_hd6445_reg[64];
	uint8_t m_hd6445_address;
};

#endif // MAME_TATSUMI_TATSUMI_H
