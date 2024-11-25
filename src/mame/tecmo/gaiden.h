// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

/***************************************************************************

    Ninja Gaiden

***************************************************************************/
#ifndef MAME_TECMO_GAIDEN_H
#define MAME_TECMO_GAIDEN_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/74157.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"
#include "tecmo_spr.h"
#include "tecmo_mix.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class gaiden_state : public driver_device
{
public:
	gaiden_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_videoram(*this, "videoram%u", 1),
		m_spriteram(*this, "spriteram")
	{ }

	void drgnbowl(machine_config &config);
	void shadoww(machine_config &config);

	void init_drgnbowl();
	void init_drgnbowla();
	void init_shadoww();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void irq_ack_w(uint16_t data);
	void drgnbowl_irq_ack_w(uint8_t data);
	void flip_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void txscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void txscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void txoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sproffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tx_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info_raiga);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	DECLARE_VIDEO_START(gaiden);
	DECLARE_VIDEO_START(drgnbowl);
	uint32_t screen_update_gaiden(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_drgnbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void drgnbowl_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void descramble_drgnbowl(int descramble_cpu);

	void drgnbowl_map(address_map &map) ATTR_COLD;
	void drgnbowl_sound_map(address_map &map) ATTR_COLD;
	void drgnbowl_sound_port_map(address_map &map) ATTR_COLD;
	void gaiden_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<tecmo_spr_device> m_sprgen;
	optional_device<tecmo_mix_device> m_mixer;

	// memory pointers
	required_shared_ptr_array<uint16_t, 3> m_videoram;
	required_device<buffered_spriteram16_device> m_spriteram;

	// video-related
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

	// misc
	int         m_sprite_sizey = 0;
};

class wildfang_state : public gaiden_state
{
public:
	wildfang_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaiden_state(mconfig, type, tag)
	{ }

	void wildfang(machine_config &config);

	void init_wildfang();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void wildfang_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t protection_r();

	void wildfang_map(address_map &map) ATTR_COLD;

	// protection related
	uint16_t    m_prot = 0;
	uint8_t     m_jumpcode = 0;
	const int   *m_jumppoints = nullptr;
};

class raiga_state : public wildfang_state
{
public:
	raiga_state(const machine_config &mconfig, device_type type, const char *tag) :
		wildfang_state(mconfig, type, tag)
	{ }

	void raiga(machine_config &config);

	void init_raiga();

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void raiga_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void screen_vblank_raiga(int state);
	uint32_t screen_update_raiga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void raiga_map(address_map &map) ATTR_COLD;

	// protection related
	bool m_protmode = false; // protection related
};

class mastninj_state : public gaiden_state
{
public:
	mastninj_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaiden_state(mconfig, type, tag),
		m_msm(*this, "msm%u", 1),
		m_adpcm_select(*this, "adpcm_select%u", 1),
		m_adpcm_bank(*this, "adpcm_bank")
	{ }

	void mastninj(machine_config &config);

	void init_mastninj();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices
	required_device_array<msm5205_device, 2> m_msm;
	required_device_array<ls157_device, 2> m_adpcm_select;

	// memory pointers
	required_memory_bank m_adpcm_bank;

	// misc
	bool m_adpcm_toggle = false;

	// mastninja ADPCM control
	void vck_flipflop_w(int state);
	void adpcm_bankswitch_w(uint8_t data);

	void descramble_mastninj_gfx(uint8_t* src);

	void mastninj_map(address_map &map) ATTR_COLD;
	void mastninj_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TECMO_GAIDEN_H
