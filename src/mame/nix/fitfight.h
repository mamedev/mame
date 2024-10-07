// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NIX_FITFIGHT_H
#define MAME_NIX_FITFIGHT_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class fitfight_state : public driver_device
{
public:
	fitfight_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fof_100000(*this, "fof_100000"),
		m_fof_600000(*this, "fof_600000"),
		m_fof_700000(*this, "fof_700000"),
		m_fof_800000(*this, "fof_800000"),
		m_fof_900000(*this, "fof_900000"),
		m_fof_a00000(*this, "fof_a00000"),
		m_fof_bak_tileram(*this, "fof_bak_tileram"),
		m_fof_mid_tileram(*this, "fof_mid_tileram"),
		m_fof_txt_tileram(*this, "fof_txt_tileram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void bbprot(machine_config &config);
	void fitfight(machine_config &config);

	void init_hotmindff();
	void init_fitfight();
	void init_histryma();
	void init_bbprot();

private:
	uint16_t fitfight_700000_r();
	uint16_t histryma_700000_r();
	uint16_t bbprot_700000_r();
	void fitfight_700000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t snd_porta_r();
	uint8_t snd_portb_r();
	uint8_t snd_portc_r();
	void snd_porta_w(uint8_t data);
	void snd_portb_w(uint8_t data);
	void snd_portc_w(uint8_t data);
	void fof_bak_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fof_mid_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fof_txt_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hotmindff_unk_r();
	TILE_GET_INFO_MEMBER(get_fof_bak_tile_info);
	TILE_GET_INFO_MEMBER(get_fof_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fof_txt_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_fitfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer );
	void bbprot_main_map(address_map &map) ATTR_COLD;
	void fitfight_main_map(address_map &map) ATTR_COLD;
	void snd_mem(address_map &map) ATTR_COLD;

	// memory pointers
	required_shared_ptr<uint16_t> m_fof_100000;
	required_shared_ptr<uint16_t> m_fof_600000;
	required_shared_ptr<uint16_t> m_fof_700000;
	required_shared_ptr<uint16_t> m_fof_800000;
	required_shared_ptr<uint16_t> m_fof_900000;
	required_shared_ptr<uint16_t> m_fof_a00000;
	required_shared_ptr<uint16_t> m_fof_bak_tileram;
	required_shared_ptr<uint16_t> m_fof_mid_tileram;
	required_shared_ptr<uint16_t> m_fof_txt_tileram;
	required_shared_ptr<uint16_t> m_spriteram;

	// video-related
	tilemap_t  *m_fof_bak_tilemap = nullptr;
	tilemap_t  *m_fof_mid_tilemap = nullptr;
	tilemap_t  *m_fof_txt_tilemap = nullptr;

	// misc
	int      m_bbprot_kludge = 0;
	uint16_t   m_fof_700000_data = 0U;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#endif // MAME_NIX_FITFIGHT_H
