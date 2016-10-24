// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#define PIXMAP_COLOR_BASE  (16 + 32)
#define BITMAPRAM_SIZE      0x6000


class dogfgt_state : public driver_device
{
public:
	dogfgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sharedram(*this, "sharedram"),
		m_subcpu(*this, "sub") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_sharedram;

	/* video-related */
	bitmap_ind16 m_pixbitmap;
	tilemap_t   *m_bg_tilemap;
	std::unique_ptr<uint8_t[]>     m_bitmapram;
	int       m_bm_plane;
	int       m_pixcolor;
	int       m_scroll[4];
	int       m_lastflip;
	int       m_lastpixcolor;

	/* sound-related */
	int       m_soundlatch;
	int       m_last_snd_ctrl;

	/* devices */
	required_device<cpu_device> m_subcpu;
	uint8_t sharedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sharedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void subirqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sub_irqack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_soundcontrol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_plane_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dogfgt_bitmapram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void internal_bitmapram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_bitmapram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dogfgt_1800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_dogfgt(palette_device &palette);
	uint32_t screen_update_dogfgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
