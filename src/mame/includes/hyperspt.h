// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#ifndef MAME_INCLUDES_HYPERSPT_H
#define MAME_INCLUDES_HYPERSPT_H

#pragma once

#include "audio/trackfld.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class hyperspt_state : public driver_device
{
public:
	hyperspt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundbrd(*this, "trackfld_audio"),
		m_dac(*this, "dac"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void hyperspt(machine_config &config);
	void roadf(machine_config &config);
	void roadfu(machine_config &config);
	void hypersptb(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<trackfld_audio_device> m_soundbrd;
	required_device<dac_8bit_r2r_device> m_dac;
	optional_device<sn76496_device> m_sn;
	optional_device<vlm5030_device> m_vlm;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t *m_bg_tilemap;

	uint8_t m_irq_mask;
	uint8_t m_SN76496_latch;

	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	void konami_SN76496_latch_w(uint8_t data) { m_SN76496_latch = data; }
	void konami_SN76496_w(uint8_t data) { m_sn->write(m_SN76496_latch); }

	virtual void machine_start() override;
	virtual void video_start() override;
	void hyperspt_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(roadf);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(roadf_get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_map(address_map &map);
	void common_sound_map(address_map &map);
	void hyperspt_map(address_map &map);
	void hyperspt_sound_map(address_map &map);
	void roadf_map(address_map &map);
	void roadf_sound_map(address_map &map);
	void soundb_map(address_map &map);
	void hyprolyb_adpcm_map(address_map &map);
};

#endif // MAME_INCLUDES_HYPERSPT_H
