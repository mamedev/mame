// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Couriersud
// thanks-to: Marc Lafontaine
class popeye_state : public driver_device
{
public:
	popeye_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_background_pos(*this, "background_pos"),
		m_palettebank(*this, "palettebank"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_color_prom(*this, "proms"),
		m_color_prom_spr(*this, "sprpal"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	uint8_t m_prot0;
	uint8_t m_prot1;
	uint8_t m_prot_shift;
	uint8_t m_dswbit;
	required_shared_ptr<uint8_t> m_background_pos;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_color_prom;
	required_region_ptr<uint8_t> m_color_prom_spr;

	std::unique_ptr<uint8_t[]> m_bitmapram;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap2;
	uint8_t m_invertmask;
	uint8_t m_bitmap_type;
	tilemap_t *m_fg_tilemap;
	uint8_t m_lastflip;
	int   m_field;

	uint8_t protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void popeye_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void popeye_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void popeye_bitmap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void skyskipr_bitmap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void popeye_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value dsw1_read(ioport_field &field, void *param);
	void init_skyskipr();
	void init_popeye();
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_popeye(palette_device &palette);
	void video_start_popeye();
	void palette_init_popeyebl(palette_device &palette);
	void palette_init_skyskipr(palette_device &palette);
	uint32_t screen_update_popeye(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void popeye_interrupt(device_t &device);
	ioport_value pop_field_r(ioport_field &field, void *param);
	void convert_color_prom(const uint8_t *color_prom);
	void set_background_palette(int bank);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_field(bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
