// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_UNICO_DRGNMST_H
#define MAME_UNICO_DRGNMST_H

#pragma once

#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "tilemap.h"
#include "video/bufsprite.h"

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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void drgnmst_main_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;

	/* video-related */
	tilemap_t     *m_bg_tilemap = nullptr;
	tilemap_t     *m_fg_tilemap = nullptr;
	tilemap_t     *m_md_tilemap = nullptr;

	bool m_alt_scrolling = false;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_vidregs;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_md_videoram;
	required_shared_ptr<uint16_t> m_rowscrollram;
	required_shared_ptr<uint16_t> m_vidregs2;
	required_device<buffered_spriteram16_device> m_spriteram;

	/* devices */
	void coin_w(uint16_t data);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void md_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	optional_device<pic16c55_device> m_audiocpu;

	uint8_t drgnmst_asciitohex(uint8_t data);

	required_memory_bank m_oki1bank;
	optional_device_array<okim6295_device, 2> m_oki;

	/* misc */
	uint8_t       m_snd_command = 0U;
	uint16_t      m_snd_flag = 0U;
	uint8_t       m_oki_control = 0U;
	uint8_t       m_oki_command = 0U;
	uint8_t       m_pic16c5x_port0 = 0U;
	uint8_t       m_oki_bank[2]{};

	void snd_command_w(uint8_t data);
	void snd_flag_w(uint8_t data);
	uint8_t pic16c5x_port0_r();
	uint8_t snd_command_r();
	uint8_t snd_flag_r();
	void pcm_banksel_w(uint8_t data);
	void oki_w(uint8_t data);
	void snd_control_w(uint8_t data);

	void drgnmst_main_map_with_pic(address_map &map) ATTR_COLD;

	void drgnmst_oki1_map(address_map &map) ATTR_COLD;
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
	virtual void video_start() override ATTR_COLD;

private:
	required_device<okim6295_device> m_oki;

	void drgnmst_main_map_with_ym(address_map &map) ATTR_COLD;
};


#endif // MAME_UNICO_DRGNMST_H
