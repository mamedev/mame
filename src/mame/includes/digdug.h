// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class digdug_state : public galaga_state
{
public:
	digdug_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state(mconfig, type, tag),
		m_digdug_objram(*this, "digdug_objram"),
		m_digdug_posram(*this, "digdug_posram"),
		m_digdug_flpram(*this, "digdug_flpram")     { }

	required_shared_ptr<uint8_t> m_digdug_objram;
	required_shared_ptr<uint8_t> m_digdug_posram;
	required_shared_ptr<uint8_t> m_digdug_flpram;

	uint8_t m_bg_select;
	uint8_t m_tx_color_mode;
	uint8_t m_bg_disable;
	uint8_t m_bg_color_bank;
	ioport_value shifted_port_r(ioport_field &field, void *param);
	tilemap_memory_index tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tx_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_digdug();
	void palette_init_digdug(palette_device &palette);
	uint32_t screen_update_digdug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void digdug_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void digdug_PORT_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
