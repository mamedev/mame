// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    City Connection

*************************************************************************/

class citycon_state : public driver_device
{
public:
	citycon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_linecolor(*this, "linecolor"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_linecolor;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_image;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t citycon_in_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t citycon_irq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void citycon_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void citycon_linecolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void citycon_background_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_citycon();
	tilemap_memory_index citycon_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_citycon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	inline void changecolor_RRRRGGGGBBBBxxxx( int color, int indx );
};
