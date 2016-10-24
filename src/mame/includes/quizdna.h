// license:BSD-3-Clause
// copyright-holders:Uki
class quizdna_state : public driver_device
{
public:
	quizdna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_generic_paletteram_8(*this, "paletteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	std::unique_ptr<uint8_t[]> m_bg_ram;
	std::unique_ptr<uint8_t[]> m_fg_ram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_bg_xscroll[2];
	int m_flipscreen;
	int m_video_enable;

	// common
	void bg_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_yscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_xscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void screen_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void paletteram_xBGR_RRRR_GGGG_BBBB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// game specific
	void gekiretu_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
