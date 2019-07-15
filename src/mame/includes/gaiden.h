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
	tilemap_t   *m_text_layer;
	tilemap_t   *m_foreground;
	tilemap_t   *m_background;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	bitmap_ind16 m_tile_bitmap_tx;
	uint16_t      m_tx_scroll_x;
	uint16_t      m_tx_scroll_y;
	uint16_t      m_bg_scroll_x;
	uint16_t      m_bg_scroll_y;
	uint16_t      m_fg_scroll_x;
	uint16_t      m_fg_scroll_y;
	int8_t        m_tx_offset_y;
	int8_t        m_bg_offset_y;
	int8_t        m_fg_offset_y;
	int8_t        m_spr_offset_y;

	/* misc */
	int         m_sprite_sizey;
	int         m_prot;
	int         m_jumpcode;
	const int   *m_jumppoints; // raiga, wildfang
	bool        m_adpcm_toggle;

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
	DECLARE_WRITE8_MEMBER(adpcm_bankswitch_w);

	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(drgnbowl_irq_ack_w);
	DECLARE_WRITE16_MEMBER(gaiden_sound_command_w);
	DECLARE_WRITE16_MEMBER(wildfang_protection_w);
	DECLARE_READ16_MEMBER(wildfang_protection_r);
	DECLARE_WRITE16_MEMBER(raiga_protection_w);
	DECLARE_READ16_MEMBER(raiga_protection_r);
	DECLARE_WRITE16_MEMBER(gaiden_flip_w);
	DECLARE_WRITE16_MEMBER(gaiden_txscrollx_w);
	DECLARE_WRITE16_MEMBER(gaiden_txscrolly_w);
	DECLARE_WRITE16_MEMBER(gaiden_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(gaiden_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(gaiden_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(gaiden_bgscrolly_w);
	DECLARE_WRITE16_MEMBER(gaiden_txoffsety_w);
	DECLARE_WRITE16_MEMBER(gaiden_fgoffsety_w);
	DECLARE_WRITE16_MEMBER(gaiden_bgoffsety_w);
	DECLARE_WRITE16_MEMBER(gaiden_sproffsety_w);
	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(fg_videoram_w);
	DECLARE_WRITE16_MEMBER(tx_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info_raiga);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	DECLARE_MACHINE_START(mastninj);
	DECLARE_MACHINE_RESET(raiga);
	DECLARE_VIDEO_START(gaiden);
	DECLARE_VIDEO_START(drgnbowl);
	DECLARE_VIDEO_START(raiga);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
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
