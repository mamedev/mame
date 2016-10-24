// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    D-Day

*************************************************************************/


class dday_state : public driver_device
{
public:
	dday_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_textvideoram(*this, "textvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_textvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_text_tilemap;
	tilemap_t        *m_sl_tilemap;
	bitmap_ind16 m_main_bitmap;
	int            m_control;
	int            m_sl_image;
	int            m_sl_enable;
	int            m_timer_value;

	/* devices */
	device_t *m_ay1;
	uint8_t dday_countdown_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dday_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dday_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dday_textvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dday_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dday_colorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dday_sl_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dday_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_sl_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_dday(palette_device &palette);
	uint32_t screen_update_dday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void countdown_timer_callback(void *ptr, int32_t param);
	void start_countdown_timer();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
