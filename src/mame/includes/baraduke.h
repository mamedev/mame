// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "sound/namco.h"

class baraduke_state : public driver_device
{
public:
	baraduke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_maincpu(*this, "maincpu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_inputport_selected;
	int m_counter;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_textram;
	required_device<cpu_device> m_maincpu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap[2];
	int m_xscroll[2];
	int m_yscroll[2];
	int m_copy_sprites;
	void inputport_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t inputport_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void baraduke_lamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void baraduke_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t soundkludge_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t readFF(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t baraduke_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void baraduke_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t baraduke_textram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void baraduke_textram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void baraduke_scroll0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void baraduke_scroll1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t baraduke_spriteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void baraduke_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_baraduke();
	tilemap_memory_index tx_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void tx_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_baraduke(palette_device &palette);
	uint32_t screen_update_baraduke(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_baraduke(screen_device &screen, bool state);
	void scroll_w(address_space &space, int layer, int offset, int data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority);
	void set_scroll(int layer);
};
