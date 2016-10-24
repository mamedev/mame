// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

class battlane_state : public driver_device
{
public:
	battlane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	bitmap_ind8 m_screen_bitmap;
	int         m_video_ctrl;
	int         m_cpu_control;  /* CPU interrupt control register */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void battlane_cpu_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_tileram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_bitmap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlane_video_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index battlane_tilemap_scan_rows_2x2(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_battlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void battlane_cpu1_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_fg_bitmap( bitmap_ind16 &bitmap );
};
