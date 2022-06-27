// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_SEIBU_DCON_H
#define MAME_SEIBU_DCON_H

#pragma once

#include "seibusound.h"

#include "emupal.h"
#include "tilemap.h"

class dcon_state : public driver_device, public seibu_sound_common
{
public:
	dcon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram")
	{ }

	void dcon(machine_config &config);
	void sdgndmps(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_back_data;
	required_shared_ptr<uint16_t> m_fore_data;
	required_shared_ptr<uint16_t> m_mid_data;
	required_shared_ptr<uint16_t> m_textram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_background_layer = nullptr;
	tilemap_t *m_foreground_layer = nullptr;
	tilemap_t *m_midground_layer = nullptr;
	tilemap_t *m_text_layer = nullptr;

	int m_gfx_bank_select = 0;
	int m_last_gfx_bank = 0;
	uint16_t m_scroll_ram[6]{};
	uint16_t m_layer_en = 0U;

	u8 sdgndmps_sound_comms_r(offs_t offset);

	void layer_en_w(uint16_t data);
	void layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxbank_w(uint16_t data);
	void background_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midground_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	virtual void video_start() override;

	uint32_t screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void dcon_map(address_map &map);
	void sdgndmps_map(address_map &map);
};

#endif // MAME_SEIBU_DCON_H
