// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Frank Palazzolo, Alex Pasadyn, David Haywood, Phil Stroffolino, Uki
class nova2001_state : public driver_device
{
public:
	nova2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram")  { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_ninjakun_io_a002_ctrl;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	void ninjakun_cpu1_io_A002_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakun_cpu2_io_A002_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakun_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nova2001_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nova2001_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ninjakun_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ninjakun_bg_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nova2001_scroll_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nova2001_scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nova2001_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pkunwar_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value ninjakun_io_A002_ctrl_r(ioport_field &field, void *param);

	void init_raiders5();
	void init_pkunwar();
	void video_start_nova2001();
	void palette_init_nova2001(palette_device &palette);
	static rgb_t BBGGRRII_decoder(uint32_t raw);
	void machine_start_ninjakun();
	void video_start_ninjakun();
	void video_start_pkunwar();
	void video_start_raiders5();

	void nova2001_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void nova2001_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ninjakun_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void ninjakun_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pkunwar_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void raiders5_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void raiders5_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update_nova2001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ninjakun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pkunwar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_raiders5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nova2001_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pkunwar_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void lineswap_gfx_roms(const char *region, const int bit);
};
