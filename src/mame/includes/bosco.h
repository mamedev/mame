// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class bosco_state : public galaga_state
{
public:
	bosco_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state(mconfig, type, tag),
			m_bosco_radarattr(*this, "bosco_radarattr"),
			m_bosco_starcontrol(*this, "starcontrol"),
			m_bosco_starblink(*this, "bosco_starblink") { }

	required_shared_ptr<uint8_t> m_bosco_radarattr;

	required_shared_ptr<uint8_t> m_bosco_starcontrol;
	required_shared_ptr<uint8_t> m_bosco_starblink;

	uint8_t *m_bosco_radarx;
	uint8_t *m_bosco_radary;

	uint8_t *m_spriteram;
	uint8_t *m_spriteram2;
	uint32_t m_spriteram_size;
	void bosco_flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	tilemap_memory_index fg_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_bosco();
	void palette_init_bosco(palette_device &palette);
	uint32_t screen_update_bosco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_bosco(screen_device &screen, bool state);

	inline void get_tile_info_bosco(tile_data &tileinfo,int tile_index,int ram_offs);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void bosco_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bosco_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bosco_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bosco_starclr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
