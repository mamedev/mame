// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_FUNYBUBL_H
#define MAME_INCLUDES_FUNYBUBL_H

#pragma once

#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class funybubl_state : public driver_device
{
public:
	funybubl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tilemapram(*this, "tilemapram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_okibank(*this, "okibank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_vrambank(*this, "vrambank")
	{ }

	void funybubl(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_tilemapram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_memory_bank m_mainbank;
	required_memory_bank m_okibank;

	/* video-related */
	tilemap_t     *m_tilemap = nullptr;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<address_map_bank_device> m_vrambank;

	/* memory */
	void vidram_bank_w(uint8_t data);
	void cpurombank_w(uint8_t data);
	void oki_bank_w(uint8_t data);

	static rgb_t funybubl_R6B6G6(uint32_t raw);
	void tilemap_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void funybubl_map(address_map &map);
	void io_map(address_map &map);
	void oki_map(address_map &map);
	void sound_map(address_map &map);
	void vrambank_map(address_map &map);
};

#endif // MAME_INCLUDES_FUNYBUBL_H
