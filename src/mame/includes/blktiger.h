// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    Black Tiger

***************************************************************************/

#include "video/bufsprite.h"

class blktiger_state : public driver_device
{
public:
	blktiger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_txvideoram(*this, "txvideoram"),
		m_mcu(*this, "mcu"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_txvideoram;

	/* video-related */
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap8x4;
	tilemap_t *m_bg_tilemap4x8;
	uint32_t  m_scroll_bank;
	uint8_t   m_scroll_x[2];
	uint8_t   m_scroll_y[2];
	std::unique_ptr<uint8_t[]>   m_scroll_ram;
	uint8_t   m_screen_layout;
	uint8_t   m_chon;
	uint8_t   m_objon;
	uint8_t   m_bgon;

	/* mcu-related */
	uint8_t   m_z80_latch;
	uint8_t   m_i8751_latch;

	/* devices */
	optional_device<cpu_device> m_mcu;
	required_device<cpu_device> m_audiocpu;
	uint8_t blktiger_from_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blktiger_to_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t blktiger_from_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blktiger_to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_coinlockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_txvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t blktiger_bgvideoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blktiger_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_bgvideoram_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_video_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blktiger_screen_layout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	tilemap_memory_index bg8x4_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index bg4x8_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_blktiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void init_blktigerb3();
};
