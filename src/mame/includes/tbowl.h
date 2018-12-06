// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_TBOWL_H
#define MAME_INCLUDES_TBOWL_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/tecmo_spr.h"
#include "emupal.h"

class tbowl_state : public driver_device
{
public:
	tbowl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_txvideoram(*this, "txvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void tbowl(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	uint16_t m_xscroll;
	uint16_t m_yscroll;
	uint16_t m_bg2xscroll;
	uint16_t m_bg2yscroll;
	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_data[2];

	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(boardb_bankswitch_w);
	DECLARE_WRITE8_MEMBER(boardc_bankswitch_w);
	DECLARE_WRITE8_MEMBER(trigger_nmi);
	DECLARE_WRITE8_MEMBER(adpcm_start_w);
	DECLARE_WRITE8_MEMBER(adpcm_end_w);
	DECLARE_WRITE8_MEMBER(adpcm_vol_w);
	DECLARE_WRITE8_MEMBER(txvideoram_w);
	DECLARE_WRITE8_MEMBER(bg2videoram_w);
	DECLARE_WRITE8_MEMBER(bgxscroll_lo);
	DECLARE_WRITE8_MEMBER(bgxscroll_hi);
	DECLARE_WRITE8_MEMBER(bgyscroll_lo);
	DECLARE_WRITE8_MEMBER(bgyscroll_hi);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(bg2xscroll_lo);
	DECLARE_WRITE8_MEMBER(bg2xscroll_hi);
	DECLARE_WRITE8_MEMBER(bg2yscroll_lo);
	DECLARE_WRITE8_MEMBER(bg2yscroll_hi);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void adpcm_int(msm5205_device *device, int chip);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int_2);

	void _6206A_map(address_map &map);
	void _6206B_map(address_map &map);
	void _6206C_map(address_map &map);
};

#endif // MAME_INCLUDES_TBOWL_H
