// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "sound/namco.h"

class skykid_state : public driver_device
{
public:
	skykid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	uint8_t m_inputport_selected;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_textram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	uint8_t m_priority;
	uint16_t m_scroll_x;
	uint16_t m_scroll_y;
	uint8_t m_main_irq_mask;
	uint8_t m_mcu_irq_mask;
	void inputport_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t inputport_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void skykid_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_subreset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_irq_1_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_irq_2_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t readFF(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t skykid_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void skykid_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t skykid_textram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void skykid_textram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_scroll_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skykid_flipscreen_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_skykid();
	tilemap_memory_index tx_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void tx_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_skykid(palette_device &palette);
	uint32_t screen_update_skykid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_vblank_irq(device_t &device);
	void mcu_vblank_irq(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
