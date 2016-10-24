// license:BSD-3-Clause
// copyright-holders:David Haywood
class bigstrkb_state : public driver_device
{
public:
	bigstrkb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_vidreg1(*this, "vidreg1"),
		m_vidreg2(*this, "vidreg2") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_videoram3;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vidreg1;
	required_shared_ptr<uint16_t> m_vidreg2;

	tilemap_t *m_tilemap;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap3;

	void videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	tilemap_memory_index bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile2_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile3_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
