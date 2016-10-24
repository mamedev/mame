// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
#include "audio/seibu.h"
#include "sound/msm5205.h"

class cabal_state : public driver_device
{
public:
	cabal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	optional_device<seibu_adpcm_device> m_adpcm1;
	optional_device<seibu_adpcm_device> m_adpcm2;
	optional_device<msm5205_device> m_msm1;
	optional_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_colorram;
	required_shared_ptr<uint16_t> m_videoram;

	tilemap_t *m_background_layer;
	tilemap_t *m_text_layer;
	int m_sound_command1;
	int m_sound_command2;
	int m_last[4];

	// common
	void flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void background_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void text_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// cabal specific
	void track_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t track_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_irq_trigger_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// cabalbl specific
	void cabalbl_sndcmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cabalbl_sound_irq_trigger_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t cabalbl_snd2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cabalbl_snd1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cabalbl_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cabalbl_1_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cabalbl_2_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_cabal();
	void init_cabalbl2();
	void machine_start_cabal();
	void machine_start_cabalbl();
	void machine_reset_cabalbl();
	virtual void video_start() override;

	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
