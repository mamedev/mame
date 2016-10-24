// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Battle Cross

***************************************************************************/

class battlex_state : public driver_device
{
public:
	battlex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	uint8_t m_in0_b4;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	uint8_t m_scroll_lsb;
	uint8_t m_scroll_msb;
	uint8_t m_starfield_enabled;
	void battlex_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlex_scroll_x_lsb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlex_scroll_x_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlex_scroll_starfield_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlex_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battlex_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value battlex_in0_b4_r(ioport_field &field, void *param);
	void init_battlex();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_battlex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void battlex_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
