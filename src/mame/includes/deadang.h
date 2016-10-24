// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "audio/seibu.h"

class deadang_state : public driver_device
{
public:
	deadang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_seibu_sound(*this, "seibu_sound"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_video_data(*this, "video_data") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<seibu_adpcm_device> m_adpcm1;
	required_device<seibu_adpcm_device> m_adpcm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_scroll_ram;
	required_shared_ptr<uint16_t> m_video_data;

	tilemap_t *m_pf3_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_layer;
	tilemap_t *m_text_layer;
	int m_tilebank;
	int m_oldtilebank;

	void foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ghunter_trackball_low_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ghunter_trackball_high_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void init_deadang();
	void init_ghunter();
	virtual void video_start() override;

	tilemap_memory_index bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_pf3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_scanline(timer_device &timer, void *ptr, int32_t param);
	void sub_scanline(timer_device &timer, void *ptr, int32_t param);
};
