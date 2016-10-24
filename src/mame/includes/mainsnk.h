// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomasz Slanina

#include "machine/gen_latch.h"

class mainsnk_state : public driver_device
{
public:
	mainsnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram"),
		m_fgram(*this, "fgram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_sound_cpu_busy;
	uint32_t m_bg_tile_offset;

	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void c600_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value sound_r(ioport_field &field, void *param);

	tilemap_memory_index marvins_tx_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_mainsnk(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int scrollx, int scrolly );
};
