// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg
/****************************************************************************

    Irem M58 hardware

****************************************************************************/

class m58_state : public driver_device
{
public:
	m58_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_x_low(*this, "scroll_x_low"),
		m_scroll_x_high(*this, "scroll_x_high"),
		m_scroll_y_low(*this, "scroll_y_low"),
		m_score_panel_disabled(*this, "score_disable")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_x_low;
	required_shared_ptr<uint8_t> m_scroll_x_high;
	required_shared_ptr<uint8_t> m_scroll_y_low;
	required_shared_ptr<uint8_t> m_score_panel_disabled;

	/* video-related */
	tilemap_t* m_bg_tilemap;
	bitmap_ind16 m_scroll_panel_bitmap;

	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_panel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void video_start() override;
	void palette_init_m58(palette_device &palette);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_rows(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_panel( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
