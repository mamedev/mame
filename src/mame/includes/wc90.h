// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_WC90_H
#define MAME_INCLUDES_WC90_H

#pragma once

#include "machine/gen_latch.h"
#include "video/tecmo_spr.h"
#include "emupal.h"
#include "tilemap.h"

class wc90_state : public driver_device
{
public:
	wc90_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll0xlo(*this, "scroll0xlo"),
		m_scroll0xhi(*this, "scroll0xhi"),
		m_scroll1xlo(*this, "scroll1xlo"),
		m_scroll1xhi(*this, "scroll1xhi"),
		m_scroll2xlo(*this, "scroll2xlo"),
		m_scroll2xhi(*this, "scroll2xhi"),
		m_scroll0ylo(*this, "scroll0ylo"),
		m_scroll0yhi(*this, "scroll0yhi"),
		m_scroll1ylo(*this, "scroll1ylo"),
		m_scroll1yhi(*this, "scroll1yhi"),
		m_scroll2ylo(*this, "scroll2ylo"),
		m_scroll2yhi(*this, "scroll2yhi"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_subbank(*this, "subbank")
	{ }

	void wc90t(machine_config &config);
	void wc90(machine_config &config);
	void pac90(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_scroll0xlo;
	required_shared_ptr<uint8_t> m_scroll0xhi;
	required_shared_ptr<uint8_t> m_scroll1xlo;
	required_shared_ptr<uint8_t> m_scroll1xhi;
	required_shared_ptr<uint8_t> m_scroll2xlo;
	required_shared_ptr<uint8_t> m_scroll2xhi;
	required_shared_ptr<uint8_t> m_scroll0ylo;
	required_shared_ptr<uint8_t> m_scroll0yhi;
	required_shared_ptr<uint8_t> m_scroll1ylo;
	required_shared_ptr<uint8_t> m_scroll1yhi;
	required_shared_ptr<uint8_t> m_scroll2ylo;
	required_shared_ptr<uint8_t> m_scroll2yhi;
	required_shared_ptr<uint8_t> m_spriteram;

	required_memory_bank m_mainbank;
	required_memory_bank m_subbank;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(bankswitch1_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(txvideoram_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(track_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(track_get_fg_tile_info);

	DECLARE_VIDEO_START(wc90t);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_map(address_map &map);
	void wc90_map_1(address_map &map);
	void wc90_map_2(address_map &map);
};

#endif // MAME_INCLUDES_WC90_H
