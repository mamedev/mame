// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
#ifndef MAME_INCLUDES_SDERBY_H
#define MAME_INCLUDES_SDERBY_H

#pragma once

#include "emupal.h"

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
	DECLARE_READ16_MEMBER(sderby_input_r);
	DECLARE_READ16_MEMBER(sderbya_input_r);
	DECLARE_READ16_MEMBER(roulette_input_r);
	DECLARE_READ16_MEMBER(rprot_r);
	DECLARE_WRITE16_MEMBER(rprot_w);
	DECLARE_WRITE16_MEMBER(sderby_out_w);
	DECLARE_WRITE16_MEMBER(scmatto_out_w);
	DECLARE_WRITE16_MEMBER(roulette_out_w);
	DECLARE_WRITE16_MEMBER(sderby_videoram_w);
	DECLARE_WRITE16_MEMBER(sderby_md_videoram_w);
	DECLARE_WRITE16_MEMBER(sderby_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(sderby_scroll_w);
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

	tilemap_t *m_tilemap;
	tilemap_t *m_md_tilemap;
	tilemap_t *m_fg_tilemap;

	uint16_t m_scroll[6];
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<7> m_lamps;
};

#endif // MAME_INCLUDES_SDERBY_H
