// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

/***************************************************************************

    Ninja Gaiden

***************************************************************************/
#ifndef MAME_INCLUDES_GAIDEN_H
#define MAME_INCLUDES_GAIDEN_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/74157.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"
#include "video/tecmo_spr.h"
#include "video/tecmo_mix.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class gaiden_state : public driver_device
{
public:
	gaiden_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 1),
		m_spriteram(*this, "spriteram"),
		m_adpcm_bank(*this, "adpcm_bank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_msm(*this, "msm%u", 1),
		m_adpcm_select(*this, "adpcm_select%u", 1)
	{ }

	void raiga(machine_config &config);
	void drgnbowl(machine_config &config);
	void mastninj(machine_config &config);
	void shadoww(machine_config &config);
	void wildfang(machine_config &config);

	void init_raiga();
	void init_drgnbowl();
	void init_drgnbowla();
	void init_mastninj();
	void init_shadoww();
	void init_wildfang();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	/* memory pointers */
	required_shared_ptr_array<uint16_t, 3> m_videoram;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_memory_bank m_adpcm_bank;

	/* video-related */
	tilemap_t   *m_text_layer = nullptr;
	tilemap_t   *m_foreground = nullptr;
	tilemap_t   *m_background = nullptr;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	bitmap_ind16 m_tile_bitmap_tx;
	uint16_t      m_tx_scroll_x = 0;
	uint16_t      m_tx_scroll_y = 0;
	uint16_t      m_bg_scroll_x = 0;
	uint16_t      m_bg_scroll_y = 0;
	uint16_t      m_fg_scroll_x = 0;
	uint16_t      m_fg_scroll_y = 0;
	int8_t        m_tx_offset_y = 0;
	int8_t        m_bg_offset_y = 0;
	int8_t        m_fg_offset_y = 0;
	int8_t        m_spr_offset_y = 0;

	/* misc */
	int         m_sprite_sizey = 0;
	int         m_prot = 0;
	int         m_jumpcode = 0;
	const int   *m_jumppoints = nullptr; // raiga, wildfang
	bool        m_adpcm_toggle = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<tecmo_spr_device> m_sprgen;
	optional_device<tecmo_mix_device> m_mixer;
	optional_device_array<msm5205_device, 2> m_msm;
	optional_device_array<ls157_device, 2> m_adpcm_select;

	// mastninja ADPCM control
	DECLARE_WRITE_LINE_MEMBER(vck_flipflop_w);
	void adpcm_bankswitch_w(uint8_t data);

	void irq_ack_w(uint16_t data);
	void drgnbowl_irq_ack_w(uint8_t data);
	void gaiden_sound_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wildfang_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t wildfang_protection_r();
	void raiga_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t raiga_protection_r();
	void gaiden_flip_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_txscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_txscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_fgscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_fgscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_bgscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_bgscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_txoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_fgoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_bgoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaiden_sproffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tx_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info_raiga);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	DECLARE_MACHINE_START(mastninj);
	DECLARE_MACHINE_RESET(raiga);
	DECLARE_VIDEO_START(gaiden);
	DECLARE_VIDEO_START(drgnbowl);
	DECLARE_VIDEO_START(raiga);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_raiga);
	uint32_t screen_update_gaiden(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_raiga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_drgnbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void drgnbowl_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void descramble_drgnbowl(int descramble_cpu);
	void descramble_mastninj_gfx(uint8_t* src);

	void drgnbowl_map(address_map &map);
	void drgnbowl_sound_map(address_map &map);
	void drgnbowl_sound_port_map(address_map &map);
	void gaiden_map(address_map &map);
	void wildfang_map(address_map &map);
	void raiga_map(address_map &map);
	void mastninj_map(address_map &map);
	void mastninj_sound_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_GAIDEN_H
