// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Capcom Baseball

*************************************************************************/

class cbasebal_state : public driver_device
{
public:
	cbasebal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;
	std::unique_ptr<uint8_t[]>    m_textram;
	std::unique_ptr<uint8_t[]>      m_scrollram;
	std::unique_ptr<uint8_t[]>    m_decoded;
	uint8_t      m_scroll_x[2];
	uint8_t      m_scroll_y[2];
	int        m_tilebank;
	int        m_spritebank;
	int        m_text_on;
	int        m_bg_on;
	int        m_obj_on;
	int        m_flipscreen;

	/* misc */
	uint8_t      m_rambank;
	void cbasebal_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bankedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bankedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cbasebal_coinctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cbasebal_textram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cbasebal_textram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cbasebal_scrollram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cbasebal_scrollram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cbasebal_gfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cbasebal_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cbasebal_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_cbasebal();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cbasebal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
