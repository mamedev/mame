// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
class dcon_state : public driver_device
{
public:
	dcon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_back_data;
	required_shared_ptr<uint16_t> m_fore_data;
	required_shared_ptr<uint16_t> m_mid_data;
	required_shared_ptr<uint16_t> m_textram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;

	int m_gfx_bank_select;
	int m_last_gfx_bank;
	uint16_t m_scroll_ram[6];
	uint16_t m_layer_en;

	void layer_en_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void layer_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void background_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mid_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_sdgndmps();
	virtual void video_start() override;

	uint32_t screen_update_dcon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sdgndmps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
};
