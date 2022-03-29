// license:BSD-3-Clause
// copyright-holders:David Haywood,Nicola Salmoria,Paul Priest
#ifndef MAME_INCLUDES_PIRATES_H
#define MAME_INCLUDES_PIRATES_H

#pragma once

#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class pirates_state : public driver_device
{
public:
	pirates_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_tx_tileram(*this, "tx_tileram"),
		m_fg_tileram(*this, "fg_tileram"),
		m_bg_tileram(*this, "bg_tileram")
	{ }

	void pirates(machine_config &config);

	void init_pirates();
	void init_genix();

	DECLARE_READ_LINE_MEMBER(prot_r);

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_tx_tileram;
	required_shared_ptr<uint16_t> m_fg_tileram;
	required_shared_ptr<uint16_t> m_bg_tileram;

	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	void out_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tx_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t genix_prot_r(offs_t offset);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void decrypt_68k();
	void decrypt_p();
	void decrypt_s();
	void decrypt_oki();
	void pirates_map(address_map &map);
};

#endif // MAME_INCLUDES_PIRATES_H
