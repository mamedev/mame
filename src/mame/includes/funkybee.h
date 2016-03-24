// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari


class funkybee_state : public driver_device
{
public:
	funkybee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_gfx_bank;
	DECLARE_READ8_MEMBER(funkybee_input_port_0_r);
	DECLARE_WRITE8_MEMBER(funkybee_coin_counter_w);
	DECLARE_WRITE8_MEMBER(funkybee_videoram_w);
	DECLARE_WRITE8_MEMBER(funkybee_colorram_w);
	DECLARE_WRITE8_MEMBER(funkybee_gfx_bank_w);
	DECLARE_WRITE8_MEMBER(funkybee_scroll_w);
	DECLARE_WRITE8_MEMBER(funkybee_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(funkybee_tilemap_scan);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(funkybee);
	UINT32 screen_update_funkybee(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_columns( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
