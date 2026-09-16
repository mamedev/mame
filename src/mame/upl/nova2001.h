// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Frank Palazzolo, Alex Pasadyn, David Haywood, Phil Stroffolino, Uki
#ifndef MAME_UPL_NOVA2001_H
#define MAME_UPL_NOVA2001_H

#pragma once

#include "cpu/z80/z80.h"
#include "emupal.h"
#include "tilemap.h"

class nova2001_state : public driver_device
{
public:
	nova2001_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "palette", 768, ENDIANNESS_LITTLE)
	{ }

	void raiders5(machine_config &config);
	void ninjakun(machine_config &config);
	void nova2001(machine_config &config);
	void pkunwar(machine_config &config);

	void init_raiders5();
	void init_pkunwar();

	ioport_value ninjakun_io_A002_ctrl_r();

private:
	required_device<z80_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_shared_ptr<u8> m_fg_videoram;
	required_shared_ptr<u8> m_bg_videoram;
	optional_shared_ptr<u8> m_spriteram;
	memory_share_creator<u8> m_paletteram;

	u8 m_ninjakun_io_a002_ctrl = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void ninjakun_cpu1_io_A002_w(u8 data);
	void ninjakun_cpu2_io_A002_w(u8 data);
	void paletteram_w(offs_t offset, u8 data);
	u8 paletteram_r(offs_t offset);
	void fg_videoram_w(offs_t offset, u8 data);
	void nova2001_bg_videoram_w(offs_t offset, u8 data);
	void ninjakun_bg_videoram_w(offs_t offset, u8 data);
	u8 ninjakun_bg_videoram_r(offs_t offset);
	void scroll_x_w(u8 data);
	void scroll_y_w(u8 data);
	void nova2001_flipscreen_w(u8 data);
	void pkunwar_flipscreen_w(u8 data);

	DECLARE_VIDEO_START(nova2001);
	void nova2001_palette(palette_device &palette) const;
	static rgb_t BBGGRRII(u32 raw);
	DECLARE_MACHINE_START(ninjakun);
	DECLARE_VIDEO_START(ninjakun);
	DECLARE_VIDEO_START(pkunwar);
	DECLARE_VIDEO_START(raiders5);

	TILE_GET_INFO_MEMBER(nova2001_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(nova2001_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakun_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakun_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(pkunwar_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(raiders5_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(raiders5_get_fg_tile_info);

	u32 screen_update_nova2001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_ninjakun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_pkunwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_raiders5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nova2001_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pkunwar_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void lineswap_gfx_roms(const char *region, const int bit);
	void ninjakun_cpu1_map(address_map &map) ATTR_COLD;
	void ninjakun_cpu2_map(address_map &map) ATTR_COLD;
	void ninjakun_shared_map(address_map &map) ATTR_COLD;
	void nova2001_map(address_map &map) ATTR_COLD;
	void pkunwar_io(address_map &map) ATTR_COLD;
	void pkunwar_map(address_map &map) ATTR_COLD;
	void raiders5_cpu1_map(address_map &map) ATTR_COLD;
	void raiders5_cpu2_map(address_map &map) ATTR_COLD;
	void raiders5_io(address_map &map) ATTR_COLD;
};

#endif // MAME_UPL_NOVA2001_H
