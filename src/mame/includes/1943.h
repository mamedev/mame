// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    1943

***************************************************************************/

class _1943_state : public driver_device
{
public:
	_1943_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_scrollx(*this, "scrollx"),
		m_scrolly(*this, "scrolly"),
		m_bgscrollx(*this, "bgscrollx"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* devices / memory pointers */
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_scrollx;
	required_shared_ptr<uint8_t> m_scrolly;
	required_shared_ptr<uint8_t> m_bgscrollx;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	int     m_char_on;
	int     m_obj_on;
	int     m_bg1_on;
	int     m_bg2_on;

	/* protection */
	uint8_t   m_prot_value;
	void c1943_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t c1943_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t _1943b_c007_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void c1943_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void c1943_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void c1943_c804_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void c1943_d806_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_1943b();
	void init_1943();
	void c1943_get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void c1943_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void c1943_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_1943(palette_device &palette);
	uint32_t screen_update_1943(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
};
