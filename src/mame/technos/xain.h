// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino
#ifndef MAME_TECHNOS_XAIN_H
#define MAME_TECHNOS_XAIN_H

#pragma once

#include "taito68705.h"

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class xain_state : public driver_device
{
public:
	xain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_mcu(*this, "mcu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_charram(*this, "charram")
		, m_bgram(*this, "bgram%u", 0U)
		, m_spriteram(*this, "spriteram")
		, m_char_tilemap(nullptr)
		, m_bg_tilemaps{ nullptr, nullptr }
		, m_rom_banks(*this, { "mainbank", "subbank" })
	{
	}

	void xsleena(machine_config &config);
	void xsleenab(machine_config &config);

	int vblank_r();
	ioport_value mcu_status_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void cpuA_bankswitch_w(uint8_t data);
	void cpuB_bankswitch_w(uint8_t data);
	void main_irq_w(offs_t offset, uint8_t data);
	void irqA_assert_w(uint8_t data);
	void irqB_clear_w(uint8_t data);
	uint8_t mcu_comm_reset_r();
	template <unsigned N> void bgram_w(offs_t offset, uint8_t data);
	void charram_w(offs_t offset, uint8_t data);
	template <unsigned N> void scrollx_w(offs_t offset, uint8_t data);
	template <unsigned N> void scrolly_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);

	TILEMAP_MAPPER_MEMBER(back_scan);
	template <unsigned N> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_char_tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bootleg_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void cpu_map_B(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	int scanline_to_vcount(int scanline);

	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	required_device<cpu_device>             m_maincpu;
	required_device<cpu_device>             m_audiocpu;
	required_device<cpu_device>             m_subcpu;
	optional_device<taito68705_mcu_device>  m_mcu;
	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<screen_device>          m_screen;
	required_device<palette_device>         m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<u8> m_charram;
	required_shared_ptr_array<u8, 2> m_bgram;
	required_shared_ptr<u8> m_spriteram;

	tilemap_t   *m_char_tilemap = nullptr;
	tilemap_t   *m_bg_tilemaps[2]{};

	required_memory_bank_array<2> m_rom_banks;

	int     m_vblank = 0;

	u8      m_pri = 0;
	u8      m_scrollx[2][2]{};
	u8      m_scrolly[2][2]{};
};

#endif // MAME_TECHNOS_XAIN_H
