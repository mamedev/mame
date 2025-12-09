// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
#ifndef MAME_TOAPLAN_RAIZING_H
#define MAME_TOAPLAN_RAIZING_H

#pragma once

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplan_txtilemap.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class raizing_base_state : public driver_device
{
public:
	raizing_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_tx_gfxram(*this, "tx_gfxram")
		, m_audiobank(*this, "audiobank")
		, m_raizing_okibank{
			{ *this, "raizing_okibank0_%u", 0U },
			{ *this, "raizing_okibank1_%u", 0U } }
		, m_shared_ram(*this, "shared_ram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_oki(*this, "oki%u", 1U)
		, m_tx_tilemap(*this, "tx_tilemap")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch%u", 1U)
		, m_oki_rom(*this, "oki%u", 1U)
		, m_coincounter(*this, "coincounter")
	{ }

protected:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// used by bgaregga + batrider etc.
	void bgaregga_common_video_start();
	void raizing_z80_bankswitch_w(u8 data);
	void raizing_oki_bankswitch_w(offs_t offset, u8 data);
	void install_raizing_okibank(int chip);
	void raizing_oki_reset();

	void common_mem(address_map &map, offs_t rom_limit) ATTR_COLD;

	// similar as NMK112, but GAL-driven; NOT actual NMK112 is present
	template<unsigned Chip>
	void raizing_oki(address_map &map)
	{
		map(0x00000, 0x000ff).bankr(m_raizing_okibank[Chip][0]);
		map(0x00100, 0x001ff).bankr(m_raizing_okibank[Chip][1]);
		map(0x00200, 0x002ff).bankr(m_raizing_okibank[Chip][2]);
		map(0x00300, 0x003ff).bankr(m_raizing_okibank[Chip][3]);
		map(0x00400, 0x0ffff).bankr(m_raizing_okibank[Chip][4]);
		map(0x10000, 0x1ffff).bankr(m_raizing_okibank[Chip][5]);
		map(0x20000, 0x2ffff).bankr(m_raizing_okibank[Chip][6]);
		map(0x30000, 0x3ffff).bankr(m_raizing_okibank[Chip][7]);
	};

	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }

	u32 screen_update_base(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void coin_w(u8 data);
	void reset_audiocpu(int state);

	optional_shared_ptr<u16> m_tx_gfxram;
	optional_memory_bank m_audiobank; // batrider and bgaregga
	optional_memory_bank_array<8> m_raizing_okibank[2];
	optional_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU

	required_device<m68000_base_device> m_maincpu;
	optional_device<z80_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	optional_device_array<okim6295_device, 2> m_oki;
	required_device<toaplan_txtilemap_device> m_tx_tilemap;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device_array<generic_latch_8_device, 4> m_soundlatch; // tekipaki, batrider, bgaregga, batsugun
	optional_region_ptr_array<u8, 2> m_oki_rom;
	required_device<toaplan_coincounter_device> m_coincounter;
	bitmap_ind8 m_custom_priority_bitmap;
};

#endif // MAME_TOAPLAN_RAIZING_H
