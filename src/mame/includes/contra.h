// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Phil Stroffolino
/*************************************************************************

    Contra / Gryzor

*************************************************************************/
#ifndef MAME_INCLUDES_CONTRA_H
#define MAME_INCLUDES_CONTRA_H

#pragma once

#include "video/k007121.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class contra_state : public driver_device
{
public:
	contra_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_cram(*this, "fg_cram"),
		m_fg_vram(*this, "fg_vram"),
		m_tx_cram(*this, "tx_cram"),
		m_tx_vram(*this, "tx_vram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_bg_cram(*this, "bg_cram"),
		m_bg_vram(*this, "bg_vram"),
		m_audiocpu(*this, "audiocpu"),
		m_k007121_1(*this, "k007121_1"),
		m_k007121_2(*this, "k007121_2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	std::unique_ptr<uint8_t[]>       m_buffered_spriteram;
	std::unique_ptr<uint8_t[]>       m_buffered_spriteram_2;
	required_shared_ptr<uint8_t> m_fg_cram;
	required_shared_ptr<uint8_t> m_fg_vram;
	required_shared_ptr<uint8_t> m_tx_cram;
	required_shared_ptr<uint8_t> m_tx_vram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_bg_cram;
	required_shared_ptr<uint8_t> m_bg_vram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	rectangle m_bg_clip;
	rectangle m_fg_clip;
	rectangle m_tx_clip;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<k007121_device> m_k007121_1;
	required_device<k007121_device> m_k007121_2;
	DECLARE_WRITE8_MEMBER(contra_bankswitch_w);
	DECLARE_WRITE8_MEMBER(contra_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sirq_clear_w);
	DECLARE_WRITE8_MEMBER(contra_coin_counter_w);
	DECLARE_WRITE8_MEMBER(contra_fg_vram_w);
	DECLARE_WRITE8_MEMBER(contra_fg_cram_w);
	DECLARE_WRITE8_MEMBER(contra_bg_vram_w);
	DECLARE_WRITE8_MEMBER(contra_bg_cram_w);
	DECLARE_WRITE8_MEMBER(contra_text_vram_w);
	DECLARE_WRITE8_MEMBER(contra_text_cram_w);
	DECLARE_WRITE8_MEMBER(contra_K007121_ctrl_0_w);
	DECLARE_WRITE8_MEMBER(contra_K007121_ctrl_1_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void contra_palette(palette_device &palette) const;
	uint32_t screen_update_contra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(contra_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int bank );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	void contra(machine_config &config);
	void contra_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CONTRA_H
