// license:BSD-3-Clause
// copyright-holders:Uki
class xxmissio_state : public driver_device
{
public:
	xxmissio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram")  { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_status;
	uint8_t m_xscroll;
	uint8_t m_yscroll;
	uint8_t m_flipscreen;

	void bank_sel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void status_m_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void status_s_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bgram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scroll_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value status_r(ioport_field &field, void *param);

	void interrupt_m(device_t &device);
	void interrupt_s(device_t &device);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;

	static rgb_t BBGGRRII_decoder(uint32_t raw);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);
};
