// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Mermaid

*************************************************************************/
#ifndef MAME_SANRITSU_MERMAID_H
#define MAME_SANRITSU_MERMAID_H

#pragma once

#include "machine/74259.h"
#include "machine/ripple_counter.h"
#include "sound/msm5205.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class mermaid_state : public driver_device
{
public:
	mermaid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram2(*this, "videoram2"),
		m_videoram(*this, "videoram"),
		m_bg_scrollram(*this, "bg_scrollram"),
		m_fg_scrollram(*this, "fg_scrollram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_adpcm(*this, "adpcm"),
		m_adpcm_counter(*this, "adpcm_counter"),
		m_ay8910(*this, "ay%u", 1),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_latch(*this, "latch%u", 1U)
	{
		m_bg_split = 0;
	}

	void rougien(machine_config &config);
	void mermaid(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_scrollram;
	required_shared_ptr<uint8_t> m_fg_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	bitmap_ind16 m_helper;
	bitmap_ind16 m_helper2;
	bitmap_ind16 m_helper_mask;
	int m_coll_bit0 = 0;
	int m_coll_bit1 = 0;
	int m_coll_bit2 = 0;
	int m_coll_bit3 = 0;
	int m_coll_bit6 = 0;
	int m_bg_split;
	int m_bg_mask = 0;
	int m_bg_bank = 0;
	int m_rougien_gfxbank1 = 0;
	int m_rougien_gfxbank2 = 0;

	/* sound-related */
	uint8_t    m_adpcm_idle = 0;
	int      m_adpcm_data = 0;
	uint8_t    m_adpcm_trigger = 0;
	uint8_t    m_adpcm_rom_sel = 0;
	bool       m_ay8910_enable[2];

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<msm5205_device> m_adpcm;
	optional_device<ripple_counter_device> m_adpcm_counter;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<ls259_device, 2> m_latch;

	uint8_t    m_nmi_mask = 0;
	void mermaid_ay8910_write_port_w(uint8_t data);
	void mermaid_ay8910_control_port_w(uint8_t data);
	void ay1_enable_w(int state);
	void ay2_enable_w(int state);
	void nmi_mask_w(int state);
	void rougien_sample_rom_lo_w(int state);
	void rougien_sample_rom_hi_w(int state);
	void rougien_sample_playback_w(int state);
	void adpcm_data_w(uint8_t data);
	void mermaid_videoram2_w(offs_t offset, uint8_t data);
	void mermaid_videoram_w(offs_t offset, uint8_t data);
	void mermaid_colorram_w(offs_t offset, uint8_t data);
	void mermaid_bg_scroll_w(offs_t offset, uint8_t data);
	void mermaid_fg_scroll_w(offs_t offset, uint8_t data);
	void bg_mask_w(int state);
	void bg_bank_w(int state);
	void rougien_gfxbankswitch1_w(int state);
	void rougien_gfxbankswitch2_w(int state);
	uint8_t mermaid_collision_r();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void common_palette(palette_device &palette) const;
	void mermaid_palette(palette_device &palette) const;
	void rougien_palette(palette_device &palette) const;
	uint32_t screen_update_mermaid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_mermaid(int state);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	uint8_t collision_check( rectangle& rect );
	void collision_update();
	void rougien_adpcm_int(int state);
	void mermaid_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SANRITSU_MERMAID_H
