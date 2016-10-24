// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "audio/m72.h"

class m90_state : public driver_device
{
public:
	m90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_data(*this, "video_data"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	required_shared_ptr<uint16_t> m_video_data;
	optional_shared_ptr<uint16_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint16_t m_video_control_data[8];
	tilemap_t *m_pf1_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_wide_layer;
	tilemap_t *m_pf2_wide_layer;
	uint8_t m_last_pf1;
	uint8_t m_last_pf2;
	void m90_coincounter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void quizf1_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dynablsb_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m90_video_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m90_video_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_bomblord();
	void init_quizf1();
	void get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf1w_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf2w_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bomblord_get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bomblord_get_pf1w_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bomblord_get_pf2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bomblord_get_pf2w_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void dynablsb_get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void dynablsb_get_pf1w_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void dynablsb_get_pf2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void dynablsb_get_pf2w_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void video_start_bomblord();
	void video_start_dynablsb();
	uint32_t screen_update_m90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bomblord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dynablsb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fake_nmi(device_t &device);
	void bomblord_fake_nmi(device_t &device);
	void m90_interrupt(device_t &device);
	void dynablsb_interrupt(device_t &device);
	void bomblord_interrupt(device_t &device);
	inline void get_tile_info(tile_data &tileinfo,int tile_index,int layer,int page_mask);
	inline void bomblord_get_tile_info(tile_data &tileinfo,int tile_index,int layer);
	inline void dynablsb_get_tile_info(tile_data &tileinfo,int tile_index,int layer);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void bomblord_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void dynablsb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void markdirty(tilemap_t *tmap,int page,offs_t offset);
};
