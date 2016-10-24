// license:BSD-3-Clause
// copyright-holders:Dan Boris, Mirko Buffoni
/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

class cloak_state : public driver_device
{
public:
	cloak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	int m_nvram_enabled;
	uint8_t m_bitmap_videoram_selected;
	uint8_t m_bitmap_videoram_address_x;
	uint8_t m_bitmap_videoram_address_y;
	std::unique_ptr<uint8_t[]> m_bitmap_videoram1;
	std::unique_ptr<uint8_t[]> m_bitmap_videoram2;
	uint8_t *m_current_bitmap_videoram_accessed;
	uint8_t *m_current_bitmap_videoram_displayed;
	std::unique_ptr<uint16_t[]>  m_palette_ram;
	tilemap_t *m_bg_tilemap;
	void cloak_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_custom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_irq_reset_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_irq_reset_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_nvram_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_clearbmp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t graph_processor_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void graph_processor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cloak_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void set_current_bitmap_videoram_pointer();
	void adjust_xy(int offset);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	uint32_t screen_update_cloak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pen(int i);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
