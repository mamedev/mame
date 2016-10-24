// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina

#include "sound/okim6295.h"
#include "audio/seibu.h"
#include "machine/gen_latch.h"
#include "machine/seibucop/seibucop.h"
#include "video/seibu_crtc.h"

class legionna_state : public driver_device
{
public:
	legionna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram"),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_seibu_sound(*this, "seibu_sound"),
			m_oki(*this, "oki"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_raiden2cop(*this, "raiden2cop")
	{
		memset(scrollvals, 0, sizeof(uint16_t)*6);
	}

	required_shared_ptr<uint16_t> m_spriteram;
	std::unique_ptr<uint16_t[]> m_back_data;
	std::unique_ptr<uint16_t[]> m_fore_data;
	std::unique_ptr<uint16_t[]> m_mid_data;
	std::unique_ptr<uint16_t[]> m_textram;
	std::unique_ptr<uint16_t[]> m_scrollram16;
	uint16_t m_layer_disable;
	std::unique_ptr<uint16_t[]> m_layer_config;
	int m_sprite_xoffs;
	int m_sprite_yoffs;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	int m_has_extended_banking;
	int m_has_extended_priority;
	uint16_t m_sprite_pri_mask[4];
	uint16_t m_back_gfx_bank;
	uint16_t m_fore_gfx_bank;
	uint16_t m_mid_gfx_bank;
	uint16_t scrollvals[6];
	void tilemap_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tile_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tile_scroll_base_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tile_vreg_1a_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videowrite_cb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wordswapram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void legionna_background_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void legionna_midground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void legionna_foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void legionna_text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_comms_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t denjinmk_sound_comms_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_comms_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void denjinmk_setgfxbank(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void heatbrl_setgfxbank(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void grainbow_layer_config_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_legiongfx();
	void init_cupsoc_debug();
	void init_cupsoc();
	void init_cupsocs();
	void init_olysoc92();
	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mid_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mid_tile_info_denji(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mid_tile_info_cupsoc(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info_denji(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_legionna();
	void video_start_heatbrl();
	void video_start_godzilla();
	void video_start_denjinmk();
	void video_start_grainbow();
	void video_start_cupsoc();
	uint32_t screen_update_legionna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_heatbrl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_godzilla(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_grainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void descramble_legionnaire_gfx(uint8_t* src);
	void common_video_start();
	void common_video_allocate_ptr();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<raiden2cop_device> m_raiden2cop;
};
