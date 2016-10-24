// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"

class taotaido_state : public driver_device
{
public:
	taotaido_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spr(*this, "vsystem_spr"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_scrollram(*this, "scrollram"),
		m_bgram(*this, "bgram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_spriteram2;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_bgram;

	int m_pending_command;
	uint16_t m_sprite_character_bank_select[8];
	uint16_t m_video_bank_select[8];
	tilemap_t *m_bg_tilemap;
	std::unique_ptr<uint16_t[]> m_spriteram_old;
	std::unique_ptr<uint16_t[]> m_spriteram_older;
	std::unique_ptr<uint16_t[]> m_spriteram2_old;
	std::unique_ptr<uint16_t[]> m_spriteram2_older;

	uint16_t pending_command_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pending_command_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sh_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprite_character_bank_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tileregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_rows(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	uint32_t tile_callback( uint32_t code );
};
