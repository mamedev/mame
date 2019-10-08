// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
#ifndef MAME_INCLUDES_BLOODBRO_H
#define MAME_INCLUDES_BLOODBRO_H

#pragma once

#include "audio/seibu.h"
#include "sound/3812intf.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class bloodbro_state : public driver_device, public seibu_sound_common
{
public:
	bloodbro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_ymsnd(*this, "ymsnd"),
		m_spriteram(*this, "spriteram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_txvideoram(*this, "txvideoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_audiocpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<ym3812_device> m_ymsnd;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_txvideoram;

	uint16_t m_scrollram[6];
	uint16_t m_layer_en;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	bool m_weststry_opl_irq;
	bool m_weststry_soundnmi_mask;

	DECLARE_WRITE16_MEMBER(bgvideoram_w);
	DECLARE_WRITE16_MEMBER(fgvideoram_w);
	DECLARE_WRITE16_MEMBER(txvideoram_w);
	DECLARE_WRITE16_MEMBER(layer_en_w);
	DECLARE_WRITE16_MEMBER(layer_scroll_w);
	DECLARE_WRITE16_MEMBER(weststry_layer_scroll_w);
	void weststry_soundlatch_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(weststry_opl_irq_w);
	DECLARE_WRITE8_MEMBER(weststry_opl_w);
	DECLARE_WRITE8_MEMBER(weststry_soundnmi_ack_w);
	void weststry_soundnmi_update();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void video_start() override;

	uint32_t screen_update_bloodbro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_weststry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_skysmash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bloodbro_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void weststry_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void init_weststry();
	void bloodbro(machine_config &config);
	void skysmash(machine_config &config);
	void weststry(machine_config &config);
	void bloodbro_map(address_map &map);
	void common_map(address_map &map);
	void skysmash_map(address_map &map);
	void weststry_map(address_map &map);
	void weststry_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_BLOODBRO_H
