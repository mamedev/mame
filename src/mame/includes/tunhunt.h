// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_workram(*this, "workram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_generic_paletteram_8(*this, "paletteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	uint8_t m_control;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_tmpbitmap;

	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t button_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsw2_0r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw2_1r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw2_2r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw2_3r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw2_4r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;
	void palette_init_tunhunt(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pens();
	void draw_motion_object(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_shell(bitmap_ind16 &bitmap, const rectangle &cliprect, int picture_code,
		int hposition,int vstart,int vstop,int vstretch,int hstretch);
};
