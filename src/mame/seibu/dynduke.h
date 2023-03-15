// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_SEIBU_DYNDUKE_H
#define MAME_SEIBU_DYNDUKE_H

#pragma once

#include "seibusound.h"

#include "video/bufsprite.h"

#include "emupal.h"
#include "tilemap.h"


class dynduke_state : public driver_device
{
public:
	dynduke_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_scroll_ram(*this, "scroll_ram"),
		m_videoram(*this, "videoram"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data")
	{ }

	void dynduke(machine_config &config);
	void dbldyn(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<uint16_t> m_scroll_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_back_data;
	required_shared_ptr<uint16_t> m_fore_data;

	tilemap_t *m_bg_layer = nullptr;
	tilemap_t *m_fg_layer = nullptr;
	tilemap_t *m_tx_layer = nullptr;
	int m_back_bankbase = 0;
	int m_fore_bankbase = 0;
	int m_back_enable = 0;
	int m_fore_enable = 0;
	int m_sprite_enable = 0;
	int m_txt_enable = 0;
	int m_old_back = 0;
	int m_old_fore = 0;

	void background_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void master_map(address_map &map);
	void masterj_map(address_map &map);
	void sei80bu_encrypted_full_map(address_map &map);
	void slave_map(address_map &map);
	void sound_decrypted_opcodes_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_SEIBU_DYNDUKE_H
