// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*

Witch / Pinball Champ '95 / Keirin Ou

*/

#ifndef MAME_INCLUDES_WITCH_H
#define MAME_INCLUDES_WITCH_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "sound/es8712.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK        XTAL(12'000'000)
#define CPU_CLOCK         MAIN_CLOCK / 4
#define YM2203_CLOCK      MAIN_CLOCK / 4
#define AY8910_CLOCK      MAIN_CLOCK / 8
#define MSM5202_CLOCK     384_kHz_XTAL

#define HOPPER_PULSE      50          // time between hopper pulses in milliseconds (not right for attendant pay)
#define UNBANKED_SIZE 0x800


class witch_state : public driver_device
{
public:
	witch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_ppi(*this, "ppi%u", 1U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_gfx0_vram(*this, "gfx0_vram")
		, m_gfx0_cram(*this, "gfx0_cram")
		, m_gfx1_vram(*this, "gfx1_vram")
		, m_gfx1_cram(*this, "gfx1_cram")
		, m_sprite_ram(*this, "sprite_ram")
		, m_palette(*this, "palette")
		, m_hopper(*this, "hopper")
		, m_mainbank(*this, "mainbank")
	{ }

	void witch(machine_config &config);

	void init_witch();

	DECLARE_WRITE8_MEMBER(gfx0_vram_w);
	DECLARE_WRITE8_MEMBER(gfx0_cram_w);
	DECLARE_WRITE8_MEMBER(gfx1_vram_w);
	DECLARE_WRITE8_MEMBER(gfx1_cram_w);
	DECLARE_READ8_MEMBER(gfx1_vram_r);
	DECLARE_READ8_MEMBER(gfx1_cram_r);
	DECLARE_READ8_MEMBER(read_a000);
	DECLARE_WRITE8_MEMBER(write_a002);
	DECLARE_WRITE8_MEMBER(write_a006);
	DECLARE_WRITE8_MEMBER(main_write_a008);
	DECLARE_WRITE8_MEMBER(sub_write_a008);
	DECLARE_READ8_MEMBER(prot_read_700x);
	DECLARE_WRITE8_MEMBER(xscroll_w);
	DECLARE_WRITE8_MEMBER(yscroll_w);

protected:
	void common_map(address_map &map);

	tilemap_t *m_gfx0_tilemap;
	tilemap_t *m_gfx1_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_gfx0_vram;
	required_shared_ptr<uint8_t> m_gfx0_cram;
	required_shared_ptr<uint8_t> m_gfx1_vram;
	required_shared_ptr<uint8_t> m_gfx1_cram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_device<palette_device> m_palette;

	required_device<ticket_dispenser_device> m_hopper;

	optional_memory_bank m_mainbank;

	int m_scrollx;
	int m_scrolly;
	uint8_t m_reg_a002;
	uint8_t m_motor_active;

	TILE_GET_INFO_MEMBER(get_gfx0_tile_info);
	TILE_GET_INFO_MEMBER(get_gfx1_tile_info);
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_reset() override;

	void witch_common_map(address_map &map);
	void witch_main_map(address_map &map);
	void witch_sub_map(address_map &map);

	void video_common_init();
	bool has_spr_rom_bank;
	uint8_t m_spr_bank;
};

class keirinou_state : public witch_state
{
public:
	keirinou_state(const machine_config &mconfig, device_type type, const char *tag)
		: witch_state(mconfig, type, tag),
		m_paletteram(*this, "paletteram")
	{ }

	void keirinou(machine_config &config);

private:
	void keirinou_common_map(address_map &map);
	void keirinou_main_map(address_map &map);
	void keirinou_sub_map(address_map &map);

	DECLARE_WRITE8_MEMBER(write_keirinou_a002);
	DECLARE_WRITE8_MEMBER(palette_w);
	TILE_GET_INFO_MEMBER(get_keirinou_gfx1_tile_info);

	virtual void video_start() override;

	uint8_t m_bg_bank;
	required_shared_ptr<uint8_t> m_paletteram;
};


#endif
