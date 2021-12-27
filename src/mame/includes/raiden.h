// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Seibu Raiden hardware

*******************************************************************************/
#ifndef MAME_INCLUDES_RAIDEN_H
#define MAME_INCLUDES_RAIDEN_H

#pragma once

#include "audio/seibu.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "tilemap.h"

class raiden_state : public driver_device, public seibu_sound_common
{
public:
	raiden_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_shared_ram(*this, "shared_ram"),
		m_textram(*this, "textram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram")
	{ }

	void raidene(machine_config &config);
	void raiden(machine_config &config);
	void raidenkb(machine_config &config);
	void raidenu(machine_config &config);

	void init_raiden();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;

	bool m_bg_layer_enabled;
	bool m_fg_layer_enabled;
	bool m_tx_layer_enabled;
	bool m_sp_layer_enabled;
	bool m_flipscreen;

	virtual void video_start() override;

	u32 screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16 *scrollregs);

	void textram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void common_video_start();

	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<u16> m_shared_ram;
	required_shared_ptr<u16> m_textram;
	optional_shared_ptr<u16> m_scroll_ram;
	required_shared_ptr<u16> m_bgram;
	required_shared_ptr<u16> m_fgram;

	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;

	void bgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void raiden_control_w(u8 data);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	u32 screen_update_raiden(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap);
	void common_decrypt();

	void main_map(address_map &map);
	void sub_map(address_map &map);
	void raiden_sound_map(address_map &map);
	void raiden_sound_decrypted_opcodes_map(address_map &map);
	void raidenu_main_map(address_map &map);
	void raidenu_sub_map(address_map &map);
	void sei80bu_encrypted_full_map(address_map &map);
};


class raidenb_state : public raiden_state
{
public:
	using raiden_state::raiden_state;

	void raidenb(machine_config &config);

protected:
	virtual void video_start() override;

private:
	u16 m_raidenb_scroll_ram[6];

	u32 screen_update_raidenb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void raidenb_control_w(u8 data);
	void raidenb_layer_enable_w(u16 data);
	void raidenb_layer_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void raidenb_main_map(address_map &map);
};

#endif // MAME_INCLUDES_RAIDEN_H
