// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    Bomb Jack

*************************************************************************/

class bombjack_state : public driver_device
{
public:
	bombjack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap;
	uint8_t       m_background_image;

	/* sound-related */
	uint8_t       m_latch;

	uint8_t       m_nmi_mask;
	void bombjack_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bombjack_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bombjack_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bombjack_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bombjack_background_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bombjack_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_bombjack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void soundlatch_callback(void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
