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
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_grid_data;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_grid_tilemap;   // ladybug
	UINT8      m_grid_color;
	int        m_star_speed;
	UINT8      m_stars_enable;
	UINT8      m_stars_speed;
	UINT32     m_stars_state;
	UINT16     m_stars_offset;
	UINT8      m_stars_count;

	/* misc */
	UINT8      m_sound_low;
	UINT8      m_sound_high;
	UINT8      m_weird_value[8];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_ioport m_port_dsw0;
	optional_ioport m_p1_control;
	optional_ioport m_p2_control;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	DECLARE_WRITE8_MEMBER(ladybug_videoram_w);
	DECLARE_WRITE8_MEMBER(ladybug_colorram_w);
	DECLARE_WRITE8_MEMBER(ladybug_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ladybug_p1_control_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ladybug_p2_control_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin1_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin2_inserted);
	DECLARE_DRIVER_INIT(dorodon);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_grid_tile_info);
	DECLARE_MACHINE_START(ladybug);
	DECLARE_VIDEO_START(ladybug);
	DECLARE_PALETTE_INIT(ladybug);
	UINT32 screen_update_ladybug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void palette_init_common( palette_device &palette, const UINT8 *color_prom,
								int r_bit0, int r_bit1, int g_bit0, int g_bit1, int b_bit0, int b_bit1 );

};
