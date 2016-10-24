// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg,Tomasz Slanina
class travrusa_state : public driver_device
{
public:
	travrusa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_scrollx[2];
	void travrusa_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void travrusa_scroll_x_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void travrusa_scroll_x_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void travrusa_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shtridrb_port11_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_shtridra();
	void init_motorace();
	void init_shtridrb();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_travrusa(palette_device &palette);
	void palette_init_shtrider(palette_device &palette);
	uint32_t screen_update_travrusa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_scroll(  );
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
