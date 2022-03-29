// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
#ifndef MAME_INCLUDES_SDERBY_H
#define MAME_INCLUDES_SDERBY_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class sderby_state : public driver_device
{
public:
	sderby_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void spacewin(machine_config &config);
	void sderbya(machine_config &config);
	void pmroulet(machine_config &config);
	void shinygld(machine_config &config);
	void sderby(machine_config &config);
	void luckboom(machine_config &config);

private:
	uint16_t sderby_input_r(offs_t offset);
	uint16_t sderbya_input_r(offs_t offset);
	uint16_t roulette_input_r(offs_t offset);
	uint16_t rprot_r();
	void rprot_w(uint16_t data);
	void sderby_out_w(uint16_t data);
	void scmatto_out_w(uint16_t data);
	void roulette_out_w(uint16_t data);
	void sderby_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sderby_md_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sderby_fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sderby_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_sderby_tile_info);
	TILE_GET_INFO_MEMBER(get_sderby_md_tile_info);
	TILE_GET_INFO_MEMBER(get_sderby_fg_tile_info);
	uint32_t screen_update_sderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pmroulet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int codeshift);
	void luckboom_map(address_map &map);
	void roulette_map(address_map &map);
	void sderby_map(address_map &map);
	void sderbya_map(address_map &map);
	void shinygld_map(address_map &map);
	void spacewin_map(address_map &map);

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_md_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_tilemap = nullptr;
	tilemap_t *m_md_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	uint16_t m_scroll[6]{};
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<7> m_lamps;
};

#endif // MAME_INCLUDES_SDERBY_H
