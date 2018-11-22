// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*************************************************************************

    Cross Pang

*************************************************************************/
#ifndef MAME_INCLUDES_CROSPANG_H
#define MAME_INCLUDES_CROSPANG_H

#pragma once

#include "machine/gen_latch.h"
#include "video/decospr.h"

class crospang_state : public driver_device
{
public:
	crospang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_fg_videoram(*this, "fg_videoram")
		, m_bg_videoram(*this, "bg_videoram")
		, m_spriteram(*this, "spriteram")
		, m_maincpu(*this, "maincpu")
		, m_sprgen(*this, "spritegen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t   *m_bg_layer;
	tilemap_t   *m_fg_layer;
	uint8_t m_bestri_tilebank[4];
	uint8_t m_bestri_tilebankselect;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<decospr_device> m_sprgen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE16_MEMBER(bestri_tilebank_data_w);
	DECLARE_WRITE16_MEMBER(bestri_tilebank_select_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_videoram_w);
	void init_crospang();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_crospang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tumblepb_gfx1_rearrange();
	void crospang(machine_config &config);
	void bestri(machine_config &config);
	void bestria(machine_config &config);
	void pitapat(machine_config &config);
	void bestri_map(address_map &map);
	void bestria_map(address_map &map);
	void pitapat_map(address_map &map);
	void crospang_base_map(address_map &map);
	void crospang_map(address_map &map);
	void crospang_sound_io_map(address_map &map);
	void crospang_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CROSPANG_H
