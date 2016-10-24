// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "video/excellent_spr.h"

class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mid_videoram(*this, "mid_videoram"),
		m_bak_videoram(*this, "bak_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch")
		{ }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_mid_videoram;
	required_shared_ptr<uint16_t> m_bak_videoram;
	required_shared_ptr<uint16_t> m_txt_videoram;
	required_shared_ptr<uint16_t> m_scroll;

	/* video-related */
	tilemap_t  *m_txt_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_bak_tilemap;

	/* misc */
	int m_aquarium_snd_ack;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<excellent_spr_device> m_sprgen;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	uint16_t aquarium_coins_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void aquarium_snd_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aquarium_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aquarium_z80_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t aquarium_oki_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aquarium_oki_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aquarium_txt_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aquarium_mid_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aquarium_bak_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_aquarium();
	void get_aquarium_txt_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_aquarium_mid_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_aquarium_bak_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_aquarium(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t aquarium_snd_bitswap( uint8_t scrambled_data );
	void mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority_mask, int priority_value);
	bitmap_ind16 m_temp_sprite_bitmap;
};
