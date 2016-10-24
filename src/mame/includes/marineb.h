// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
class marineb_state : public driver_device
{
public:
	marineb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	uint8_t     m_palette_bank;
	uint8_t     m_column_scroll;
	uint8_t     m_flipscreen_x;
	uint8_t     m_flipscreen_y;
	uint8_t     m_marineb_active_low_flipscreen;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t     m_irq_mask;
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_column_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_palette_bank_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_palette_bank_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_flipscreen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void marineb_flipscreen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_marineb(palette_device &palette);
	void machine_reset_springer();
	uint32_t screen_update_marineb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_changes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_springer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hoccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hopprobo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void marineb_vblank_irq(device_t &device);
	void wanted_vblank_irq(device_t &device);
	void set_tilemap_scrolly( int cols );
};
