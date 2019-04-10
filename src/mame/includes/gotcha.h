// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Gotcha

*************************************************************************/
#ifndef MAME_INCLUDES_GOTCHA_H
#define MAME_INCLUDES_GOTCHA_H

#pragma once

#include "sound/okim6295.h"
#include "video/decospr.h"

class gotcha_state : public driver_device
{
public:
	gotcha_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamp_r(*this, "lamp_p%u_r", 1U),
		m_lamp_g(*this, "lamp_p%u_g", 1U),
		m_lamp_b(*this, "lamp_p%u_b", 1U),
		m_lamp_s(*this, "lamp_p%u_s", 1U)
	{
	}

	void gotcha(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE16_MEMBER(gotcha_lamps_w);
	DECLARE_WRITE16_MEMBER(gotcha_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_select_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_w);
	DECLARE_WRITE16_MEMBER(gotcha_scroll_w);
	DECLARE_WRITE8_MEMBER(gotcha_oki_bank_w);
	TILEMAP_MAPPER_MEMBER(gotcha_tilemap_scan);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	uint32_t screen_update_gotcha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info( tile_data &tileinfo, int tile_index ,uint16_t *vram, int color_offs);
	void gotcha_map(address_map &map);
	void sound_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_device<decospr_device> m_sprgen;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	int         m_banksel;
	int         m_gfxbank[4];
	uint16_t      m_scroll[4];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;

	output_finder<3> m_lamp_r;
	output_finder<3> m_lamp_g;
	output_finder<3> m_lamp_b;
	output_finder<3> m_lamp_s;
};

#endif // MAME_INCLUDES_GOTCHA_H
