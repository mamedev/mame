// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

  Sega KO Punch

*************************************************************************/

class kopunch_state : public driver_device
{
public:
	kopunch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vram_fg(*this, "vram_fg"),
		m_vram_bg(*this, "vram_bg")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_vram_fg;
	required_shared_ptr<uint8_t> m_vram_bg;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_gfxbank;
	uint8_t m_scrollx;

	uint8_t sensors1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sensors2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_bg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void left_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void right_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void vblank_interrupt(device_t &device);

	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void palette_init_kopunch(palette_device &palette);
	uint32_t screen_update_kopunch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void video_start() override;
};
