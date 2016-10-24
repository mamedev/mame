// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski

#include "audio/seibu.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"

class toki_state : public driver_device
{
public:
	toki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_background1_videoram(*this, "bg1_vram"),
		m_background2_videoram(*this, "bg2_vram"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // tokib

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_background1_videoram;
	required_shared_ptr<uint16_t> m_background2_videoram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_scrollram;

	int m_msm5205next;
	int m_toggle;

	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_text_layer;

	void tokib_soundcommand_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toki_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void foreground_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void background1_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void background2_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tokib_adpcm_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tokib_adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tokib_adpcm_int(int state);

	void init_tokib();
	void init_jujuba();
	void init_toki();

	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update_toki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tokib(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void toki_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void tokib_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
