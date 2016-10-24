// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Kyugo hardware games

***************************************************************************/

class kyugo_state : public driver_device
{
public:
	kyugo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bgattribram(*this, "bgattribram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_spriteram_2(*this, "spriteram_2"),
		m_shared_ram(*this, "shared_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_bgattribram;
	required_shared_ptr<uint8_t> m_spriteram_1;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_shared_ram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	uint8_t       m_scroll_x_lo;
	uint8_t       m_scroll_x_hi;
	uint8_t       m_scroll_y;
	int         m_bgpalbank;
	int         m_fgcolor;
	const uint8_t *m_color_codes;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t       m_nmi_mask;
	void kyugo_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_sub_cpu_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_bgattribram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kyugo_spriteram_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kyugo_scroll_x_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kyugo_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_srdmissn();
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_kyugo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
