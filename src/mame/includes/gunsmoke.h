// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/*************************************************************************

    Gun.Smoke

*************************************************************************/

class gunsmoke_state : public driver_device
{
public:
	gunsmoke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_scrollx(*this, "scrollx"),
		m_scrolly(*this, "scrolly"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	uint8_t      m_chon;
	uint8_t      m_objon;
	uint8_t      m_bgon;
	uint8_t      m_sprite3bank;
	uint8_t gunsmoke_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gunsmoke_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gunsmoke_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gunsmoke_c804_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gunsmoke_d806_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_gunsmoke(palette_device &palette);
	uint32_t screen_update_gunsmoke(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
