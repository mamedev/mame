// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_DRGNMST_H
#define MAME_INCLUDES_DRGNMST_H

#pragma once

#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "emupal.h"
#include "tilemap.h"

class drgnmst_base_state : public driver_device
{
public:
	drgnmst_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_alt_scrolling(false),
		m_vidregs(*this, "vidregs"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_rowscrollram(*this, "rowscrollram"),
		m_vidregs2(*this, "vidregs2"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void drgnmst(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void drgnmst_main_map(address_map &map);
	required_device<cpu_device> m_maincpu;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_md_tilemap;

	bool m_alt_scrolling;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_vidregs;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_md_videoram;
	required_shared_ptr<uint16_t> m_rowscrollram;
	required_shared_ptr<uint16_t> m_vidregs2;
	required_shared_ptr<uint16_t> m_spriteram;

	/* devices */
	DECLARE_WRITE16_MEMBER(coin_w);
	DECLARE_WRITE16_MEMBER(fg_videoram_w);
	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(md_videoram_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_md_tile_info);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan_cols);
	TILEMAP_MAPPER_MEMBER(md_tilemap_scan_cols);
	TILEMAP_MAPPER_MEMBER(bg_tilemap_scan_cols);
	static rgb_t drgnmst_IIIIRRRRGGGGBBBB(uint32_t raw);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

};

class drgnmst_pic_state : public drgnmst_base_state
{
public:
	drgnmst_pic_state(const machine_config& mconfig, device_type type, const char* tag) :
		drgnmst_base_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_oki1bank(*this, "oki1bank"),
		m_oki(*this, "oki%u", 1U)
	{ }

	void init_drgnmst();

	void drgnmst_with_pic(machine_config& config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	optional_device<pic16c55_device> m_audiocpu;

	uint8_t drgnmst_asciitohex(uint8_t data);

	required_memory_bank m_oki1bank;
	optional_device_array<okim6295_device, 2> m_oki;

	/* misc */
	uint8_t       m_snd_command;
	uint16_t      m_snd_flag;
	uint8_t       m_oki_control;
	uint8_t       m_oki_command;
	uint8_t       m_pic16c5x_port0;
	uint8_t       m_oki_bank[2];

	DECLARE_WRITE8_MEMBER(snd_command_w);
	DECLARE_WRITE8_MEMBER(snd_flag_w);
	DECLARE_READ8_MEMBER(pic16c5x_port0_r);
	DECLARE_READ8_MEMBER(snd_command_r);
	DECLARE_READ8_MEMBER(snd_flag_r);
	DECLARE_WRITE8_MEMBER(pcm_banksel_w);
	DECLARE_WRITE8_MEMBER(oki_w);
	DECLARE_WRITE8_MEMBER(snd_control_w);

	void drgnmst_main_map_with_pic(address_map& map);

	void drgnmst_oki1_map(address_map &map);
};

class drgnmst_ym_state : public drgnmst_base_state
{
public:
	drgnmst_ym_state(const machine_config& mconfig, device_type type, const char* tag) :
		drgnmst_base_state(mconfig, type, tag),
		m_oki(*this, "oki")
	{ }

	void drgnmst_ym(machine_config& config);

protected:
	virtual void video_start() override;

private:
	required_device<okim6295_device> m_oki;

	void drgnmst_main_map_with_ym(address_map& map);
};


#endif // MAME_INCLUDES_DRGNMST_H
