// license:BSD-3-Clause
// copyright-holders:Luca Elia
class clshroad_state : public driver_device
{
public:
	clshroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vregs(*this, "vregs") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vram_0;
	required_shared_ptr<uint8_t> m_vram_1;
	required_shared_ptr<uint8_t> m_vregs;

	tilemap_t *m_tilemap_0a;
	tilemap_t *m_tilemap_0b;
	tilemap_t *m_tilemap_1;

	uint8_t input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info_0a(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_0b(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_rows_extra(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tile_info_fb1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);


	void init_firebatl();
	virtual void machine_reset() override;
	void video_start_firebatl();
	void palette_init_firebatl(palette_device &palette);
	void video_start_clshroad();
	void palette_init_clshroad(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
