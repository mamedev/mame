// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
class m52_state : public driver_device
{
public:
	m52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	uint8_t                m_bg1xpos;
	uint8_t                m_bg1ypos;
	uint8_t                m_bg2xpos;
	uint8_t                m_bg2ypos;
	uint8_t                m_bgcontrol;
	void m52_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m52_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m52_bg1ypos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_bg1xpos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_bg2xpos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_bg2ypos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_bgcontrol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m52_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void alpha1v_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_m52(palette_device &palette);
	uint32_t screen_update_m52(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
