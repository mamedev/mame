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
#include "tilemap.h"

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

	void crospang(machine_config &config);
	void bestri(machine_config &config);
	void bestria(machine_config &config);
	void pitapat(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_fg_videoram;
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_spriteram;

	/* video-related */
	tilemap_t   *m_bg_layer = nullptr;
	tilemap_t   *m_fg_layer = nullptr;
	u8          m_tilebank[4]{};
	u8          m_tilebankselect = 0U;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<decospr_device> m_sprgen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	void tilebank_data_w(u16 data);
	void tilebank_select_w(u16 data);
	void bestri_bg_scrolly_w(u16 data);
	void bestri_fg_scrolly_w(u16 data);
	void bestri_fg_scrollx_w(u16 data);
	void bestri_bg_scrollx_w(u16 data);
	void fg_scrolly_w(u16 data);
	void bg_scrolly_w(u16 data);
	void fg_scrollx_w(u16 data);
	void bg_scrollx_w(u16 data);
	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void base_map(address_map &map);
	void bestri_map(address_map &map);
	void bestria_map(address_map &map);
	void crospang_map(address_map &map);
	void pitapat_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CROSPANG_H
