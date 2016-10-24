// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
class thepit_state : public driver_device
{
public:
	thepit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_attributesram(*this, "attributesram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_attributesram;
	required_shared_ptr<uint8_t> m_spriteram;

	uint8_t m_graphics_bank;
	uint8_t m_flip_x;
	uint8_t m_flip_y;
	tilemap_t *m_solid_tilemap;
	tilemap_t *m_tilemap;
	std::unique_ptr<uint8_t[]> m_dummy_tile;
	uint8_t m_nmi_mask;

	int m_question_address;
	int m_question_rom;
	int m_remap_address[16];

	void sound_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t input_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t intrepid_colorram_mirror_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void intrepid_graphics_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t rtriv_question_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void solid_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_rtriv();
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_thepit(palette_device &palette);
	void palette_init_suprmous(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_desertdan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority_to_draw);

	void vblank_irq(device_t &device);
};
