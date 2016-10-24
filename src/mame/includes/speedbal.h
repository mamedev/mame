// license:BSD-3-Clause
// copyright-holders:Joseba Epalza
class speedbal_state : public driver_device
{
public:
	speedbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_background_videoram(*this, "bg_videoram"),
		m_foreground_videoram(*this, "fg_videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_background_videoram;
	required_shared_ptr<uint8_t> m_foreground_videoram;

	bool m_leds_start;
	uint32_t m_leds_shiftreg;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	void init_speedbal();
	void init_musicbal();
	virtual void machine_start() override;
	virtual void video_start() override;

	void coincounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void foreground_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void background_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maincpu_50_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leds_output_block(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leds_start_block(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leds_shift_bit(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
