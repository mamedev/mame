// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Quench

#include "machine/gen_latch.h"
#include "sound/okim6295.h"

class sslam_state : public driver_device
{
public:
	sslam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bg_tileram(*this, "bg_tileram"),
		m_md_tileram(*this, "md_tileram"),
		m_tx_tileram(*this, "tx_tileram"),
		m_regs(*this, "regs"),
		m_spriteram(*this, "spriteram") { }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_bg_tileram;
	optional_shared_ptr<uint16_t> m_md_tileram;
	optional_shared_ptr<uint16_t> m_tx_tileram;
	required_shared_ptr<uint16_t> m_regs;
	required_shared_ptr<uint16_t> m_spriteram;

	emu_timer *m_music_timer;

	int m_sound;
	int m_melody;
	int m_bar;
	int m_track;
	int m_snd_bank;

	uint8_t m_oki_control;
	uint8_t m_oki_command;
	uint8_t m_oki_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_md_tilemap;

	int m_sprites_x_offset;
	void powerbls_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t playmark_snd_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void playmark_oki_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void playmark_snd_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sslam_tx_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sslam_md_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sslam_bg_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void powerbls_bg_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sslam_snd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_sslam();
	void init_powerbls();
	void get_sslam_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_sslam_md_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_sslam_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_powerbls_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_sslam();
	void video_start_powerbls();
	uint32_t screen_update_sslam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_powerbls(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void music_playback(void *ptr, int32_t param);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sslam_play(int track, int data);
};
