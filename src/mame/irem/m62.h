// license:BSD-3-Clause
// copyright-holders:smf, David Haywood
#ifndef MAME_IREM_M62_H
#define MAME_IREM_M62_H

#pragma once

#include "irem.h"
#include "emupal.h"
#include "tilemap.h"

class m62_state : public driver_device
{
public:
	m62_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audio(*this, "irem_audio")
		, m_spriteram(*this, "spriteram")
		, m_m62_tileram(*this, "m62_tileram")
		, m_m62_textram(*this, "m62_textram")
		, m_scrollram(*this, "scrollram")
		, m_sprite_height_prom(*this, "spr_height_prom")
		, m_sprite_color_proms(*this, "spr_color_proms")
		, m_chr_color_proms(*this, "chr_color_proms")
		, m_fg_color_proms(*this, "fg_color_proms")
		, m_fg_decode(*this, "fg_decode")
		, m_spr_decode(*this, "spr_decode")
		, m_chr_decode(*this, "chr_decode")
		, m_fg_palette(*this, "fg_palette")
		, m_spr_palette(*this, "spr_palette")
		, m_chr_palette(*this, "chr_palette")
	{ }

	void ldrun2(machine_config &config);
	void lotlot(machine_config &config);
	void ldrun3(machine_config &config);
	void battroad(machine_config &config);
	void horizon(machine_config &config);
	void ldrun4(machine_config &config);
	void spelunk2(machine_config &config);
	void youjyudn(machine_config &config);
	void kungfum(machine_config &config);
	void spelunkr(machine_config &config);
	void ldrun(machine_config &config);
	void kidniki(machine_config &config);

	void init_youjyudn();
	void init_spelunkr();
	void init_ldrun2();
	void init_ldrun4();
	void init_spelunk2();
	void init_kidniki();
	void init_battroad();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<irem_audio_device> m_audio;

	tilemap_t*             m_bg_tilemap = nullptr;
	uint8_t                m_kidniki_background_bank = 0;

	void m62_flipscreen_w(uint8_t data);
	void m62_hscroll_low_w(uint8_t data);
	void m62_hscroll_high_w(uint8_t data);
	void kidniki_background_bank_w(uint8_t data);

	void m62_start(tilemap_get_info_delegate tile_get_info, int rows, int cols, int x1, int y1, int x2, int y2);

	void kungfum_io_map(address_map &map) ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;

	optional_shared_ptr<uint8_t> m_m62_tileram;
	optional_shared_ptr<uint8_t> m_m62_textram;
	optional_shared_ptr<uint8_t> m_scrollram;

	/* video-related */
	tilemap_t*             m_fg_tilemap = nullptr;
	int                  m_flipscreen = 0;
	required_region_ptr<uint8_t> m_sprite_height_prom;
	required_region_ptr<uint8_t> m_sprite_color_proms;
	required_region_ptr<uint8_t> m_chr_color_proms;
	optional_region_ptr<uint8_t> m_fg_color_proms;
	int32_t                m_m62_background_hscroll = 0;
	int32_t                m_m62_background_vscroll = 0;
	int32_t                m_kidniki_text_vscroll = 0;
	int                  m_ldrun3_topbottom_mask = 0;
	int32_t                m_spelunkr_palbank = 0;

	/* misc */
	int                 m_ldrun2_bankswap = 0;  //ldrun2
	int                 m_bankcontrol[2];   //ldrun2
	uint8_t ldrun2_bankswitch_r();
	void ldrun2_bankswitch_w(offs_t offset, uint8_t data);
	uint8_t ldrun3_prot_5_r();
	uint8_t ldrun3_prot_7_r();
	void ldrun4_bankswitch_w(uint8_t data);
	void kidniki_bankswitch_w(uint8_t data);
	void spelunkr_bankswitch_w(uint8_t data);
	void spelunk2_bankswitch_w(uint8_t data);
	void youjyudn_bankswitch_w(uint8_t data);
	void m62_vscroll_low_w(uint8_t data);
	void m62_vscroll_high_w(uint8_t data);
	void m62_tileram_w(offs_t offset, uint8_t data);
	void m62_textram_w(offs_t offset, uint8_t data);
	void kungfum_tileram_w(offs_t offset, uint8_t data);
	void ldrun3_topbottom_mask_w(uint8_t data);
	void kidniki_text_vscroll_low_w(uint8_t data);
	void kidniki_text_vscroll_high_w(uint8_t data);
	void spelunkr_palbank_w(uint8_t data);
	void spelunk2_gfxport_w(uint8_t data);
	void horizon_scrollram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_kungfum_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_ldrun_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_ldrun2_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_battroad_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_battroad_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_ldrun4_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_lotlot_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_lotlot_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_kidniki_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_kidniki_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_spelunkr_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_spelunkr_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_spelunk2_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_youjyudn_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_youjyudn_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_horizon_bg_tile_info);
	DECLARE_MACHINE_START(battroad);
	void machine_init_save();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void m62_spr(palette_device &palette) const;
	void m62_chr(palette_device &palette) const;
	void m62_lotlot_fg(palette_device &palette) const;
	void m62_battroad_fg(palette_device &palette) const;
	DECLARE_VIDEO_START(kungfum);
	DECLARE_VIDEO_START(battroad);
	DECLARE_VIDEO_START(ldrun2);
	DECLARE_VIDEO_START(ldrun4);
	DECLARE_VIDEO_START(lotlot);
	DECLARE_VIDEO_START(kidniki);
	DECLARE_VIDEO_START(spelunkr);
	DECLARE_VIDEO_START(spelunk2);
	void spelunk2_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(youjyudn);
	DECLARE_VIDEO_START(horizon);
	uint32_t screen_update_ldrun(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kungfum(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_battroad(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ldrun3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ldrun4(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lotlot(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kidniki(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spelunkr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spelunk2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_youjyudn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_horizon(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void m62_amplify_contrast(bool include_fg);
	void register_savestate();
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int colormask, int prioritymask, int priority);
	void m62_textlayer(tilemap_get_info_delegate tile_get_info, int rows, int cols, int x1, int y1, int x2, int y2);
	optional_device<gfxdecode_device> m_fg_decode;
	required_device<gfxdecode_device> m_spr_decode;
	required_device<gfxdecode_device> m_chr_decode;
	optional_device<palette_device> m_fg_palette;
	required_device<palette_device> m_spr_palette;
	required_device<palette_device> m_chr_palette;

	void battroad_io_map(address_map &map) ATTR_COLD;
	void battroad_map(address_map &map) ATTR_COLD;
	void horizon_map(address_map &map) ATTR_COLD;
	void kidniki_io_map(address_map &map) ATTR_COLD;
	void kidniki_map(address_map &map) ATTR_COLD;
	void kungfum_map(address_map &map) ATTR_COLD;
	void ldrun2_io_map(address_map &map) ATTR_COLD;
	void ldrun2_map(address_map &map) ATTR_COLD;
	void ldrun3_io_map(address_map &map) ATTR_COLD;
	void ldrun3_map(address_map &map) ATTR_COLD;
	void ldrun4_io_map(address_map &map) ATTR_COLD;
	void ldrun4_map(address_map &map) ATTR_COLD;
	void ldrun_map(address_map &map) ATTR_COLD;
	void lotlot_map(address_map &map) ATTR_COLD;
	void spelunk2_map(address_map &map) ATTR_COLD;
	void spelunkr_map(address_map &map) ATTR_COLD;
	void youjyudn_io_map(address_map &map) ATTR_COLD;
	void youjyudn_map(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN( m62_common );
INPUT_PORTS_EXTERN( kungfum );

#endif // MAME_IREM_M62_H
