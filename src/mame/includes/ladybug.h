// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Universal 8106-A2 + 8106-B PCB set

*************************************************************************/

class ladybug_state : public driver_device
{
public:
	ladybug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_grid_data(*this, "grid_data"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_port_dsw0(*this, "DSW0"),
		m_p1_control(*this, "CONTP1"),
		m_p2_control(*this, "CONTP2"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_grid_data;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_grid_tilemap;   // ladybug
	uint8_t      m_grid_color;
	int        m_star_speed;
	uint8_t      m_stars_enable;
	uint8_t      m_stars_speed;
	uint32_t     m_stars_state;
	uint16_t     m_stars_offset;
	uint8_t      m_stars_count;

	/* misc */
	uint8_t      m_sound_low;
	uint8_t      m_sound_high;
	uint8_t      m_weird_value[8];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_ioport m_port_dsw0;
	optional_ioport m_p1_control;
	optional_ioport m_p2_control;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	void ladybug_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ladybug_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ladybug_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value ladybug_p1_control_r(ioport_field &field, void *param);
	ioport_value ladybug_p2_control_r(ioport_field &field, void *param);
	void coin1_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void coin2_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_dorodon();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_grid_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_ladybug();
	void video_start_ladybug();
	void palette_init_ladybug(palette_device &palette);
	uint32_t screen_update_ladybug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void palette_init_common( palette_device &palette, const uint8_t *color_prom,
								int r_bit0, int r_bit1, int g_bit0, int g_bit1, int b_bit0, int b_bit1 );

};
