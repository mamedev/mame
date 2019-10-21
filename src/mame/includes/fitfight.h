// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_FITFIGHT_H
#define MAME_INCLUDES_FITFIGHT_H

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
	DECLARE_READ16_MEMBER(fitfight_700000_r);
	DECLARE_READ16_MEMBER(histryma_700000_r);
	DECLARE_READ16_MEMBER(bbprot_700000_r);
	DECLARE_WRITE16_MEMBER(fitfight_700000_w);
	DECLARE_READ8_MEMBER(snd_porta_r);
	DECLARE_READ8_MEMBER(snd_portb_r);
	DECLARE_READ8_MEMBER(snd_portc_r);
	DECLARE_WRITE8_MEMBER(snd_porta_w);
	DECLARE_WRITE8_MEMBER(snd_portb_w);
	DECLARE_WRITE8_MEMBER(snd_portc_w);
	DECLARE_WRITE16_MEMBER(fof_bak_tileram_w);
	DECLARE_WRITE16_MEMBER(fof_mid_tileram_w);
	DECLARE_WRITE16_MEMBER(fof_txt_tileram_w);

	DECLARE_READ16_MEMBER( hotmindff_unk_r );
	TILE_GET_INFO_MEMBER(get_fof_bak_tile_info);
	TILE_GET_INFO_MEMBER(get_fof_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fof_txt_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_fitfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer );
	void bbprot_main_map(address_map &map);
	void fitfight_main_map(address_map &map);
	void snd_mem(address_map &map);

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
	tilemap_t  *m_fof_bak_tilemap;
	tilemap_t  *m_fof_mid_tilemap;
	tilemap_t  *m_fof_txt_tilemap;

	// misc
	int      m_bbprot_kludge;
	uint16_t   m_fof_700000_data;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_FITFIGHT_H
