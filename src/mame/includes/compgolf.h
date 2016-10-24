// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli, Bryan McPhail
/*************************************************************************

    Competition Golf Final Round

*************************************************************************/

class compgolf_state : public driver_device
{
public:
	compgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t        *m_text_tilemap;
	tilemap_t        *m_bg_tilemap;
	int            m_scrollx_lo;
	int            m_scrollx_hi;
	int            m_scrolly_lo;
	int            m_scrolly_hi;

	/* misc */
	int            m_bank;
	void compgolf_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void compgolf_video_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void compgolf_back_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void compgolf_scrollx_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void compgolf_scrolly_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_compgolf();
	void get_text_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index back_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_back_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_compgolf(palette_device &palette);
	uint32_t screen_update_compgolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void compgolf_expand_bg();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
