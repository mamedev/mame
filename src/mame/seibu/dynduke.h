// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_SEIBU_DYNDUKE_H
#define MAME_SEIBU_DYNDUKE_H

#pragma once

#include "seibusound.h"

#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
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
		m_text_ram(*this, "text_ram"),
		m_bg_ram(*this, "bg_ram"),
		m_fg_ram(*this, "fg_ram")
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

	required_shared_ptr<u16> m_scroll_ram;
	required_shared_ptr<u16> m_text_ram;
	required_shared_ptr<u16> m_bg_ram;
	required_shared_ptr<u16> m_fg_ram;

	tilemap_t *m_bg_layer = nullptr;
	tilemap_t *m_fg_layer = nullptr;
	tilemap_t *m_tx_layer = nullptr;
	u32 m_back_bankbase = 0;
	u32 m_fore_bankbase = 0;
	bool m_back_enable = false;
	bool m_fore_enable = false;
	bool m_sprite_enable = false;
	bool m_txt_enable = false;
	u32 m_old_back = 0;
	u32 m_old_fore = 0;

	void background_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void foreground_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void text_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void gfxbank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void control_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void video_start() override;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, u32 pri_mask);

	void vblank_irq(int state);
	void master_map(address_map &map);
	void masterj_map(address_map &map);
	void sei80bu_encrypted_full_map(address_map &map);
	void slave_map(address_map &map);
	void sound_decrypted_opcodes_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_SEIBU_DYNDUKE_H
