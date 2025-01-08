// license:GPL-2.0+
// copyright-holders:Norbert Kehrer
/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/
#ifndef MAME_DATAEAST_MADALIEN_H
#define MAME_DATAEAST_MADALIEN_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "tilemap.h"


#define MADALIEN_MAIN_CLOCK     XTAL(10'595'000)


class madalien_state : public driver_device
{
public:
	madalien_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_shift_hi(*this, "shift_hi"),
		m_shift_lo(*this, "shift_lo"),
		m_video_flags(*this, "video_flags"),
		m_headlight_pos(*this, "headlight_pos"),
		m_edge1_pos(*this, "edge1_pos"),
		m_edge2_pos(*this, "edge2_pos"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2")
	{ }

	void madalien(machine_config &config);
	void madalien_video(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_video_control;
	required_shared_ptr<uint8_t> m_shift_hi;
	required_shared_ptr<uint8_t> m_shift_lo;
	required_shared_ptr<uint8_t> m_video_flags;
	required_shared_ptr<uint8_t> m_headlight_pos;
	required_shared_ptr<uint8_t> m_edge1_pos;
	required_shared_ptr<uint8_t> m_edge2_pos;
	required_shared_ptr<uint8_t> m_scroll;

	tilemap_t *m_tilemap_fg = nullptr;
	tilemap_t *m_tilemap_edge1[4]{};
	tilemap_t *m_tilemap_edge2[4]{};
	std::unique_ptr<bitmap_ind16> m_headlight_bitmap;
	uint8_t shift_r();
	uint8_t shift_rev_r();
	void madalien_output_w(uint8_t data);
	void madalien_videoram_w(offs_t offset, uint8_t data);
	void madalien_charram_w(offs_t offset, uint8_t data);
	void madalien_portA_w(uint8_t data);
	void madalien_portB_w(uint8_t data);
	TILEMAP_MAPPER_MEMBER(scan_mode0);
	TILEMAP_MAPPER_MEMBER(scan_mode1);
	TILEMAP_MAPPER_MEMBER(scan_mode2);
	TILEMAP_MAPPER_MEMBER(scan_mode3);
	TILE_GET_INFO_MEMBER(get_tile_info_BG_1);
	TILE_GET_INFO_MEMBER(get_tile_info_BG_2);
	TILE_GET_INFO_MEMBER(get_tile_info_FG);
	void madalien_palette(palette_device &palette) const;
	uint32_t screen_update_madalien(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int scan_helper(int col, int row, int section);
	void draw_edges(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, int scroll_mode);
	void draw_headlight(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_foreground(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	inline uint8_t shift_common(uint8_t hi, uint8_t lo);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

/*----------- defined in audio/madalien.c -----------*/

DISCRETE_SOUND_EXTERN( madalien_discrete );

/* Discrete Sound Input Nodes */
#define MADALIEN_8910_PORTA         NODE_01
#define MADALIEN_8910_PORTB         NODE_02

#endif // MAME_DATAEAST_MADALIEN_H
