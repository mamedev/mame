// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Kyugo hardware games

***************************************************************************/

class kyugo_state : public driver_device
{
public:
	kyugo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bgattribram(*this, "bgattribram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_spriteram_2(*this, "spriteram_2"),
		m_shared_ram(*this, "shared_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_bgattribram;
	required_shared_ptr<UINT8> m_spriteram_1;
	required_shared_ptr<UINT8> m_spriteram_2;
	required_shared_ptr<UINT8> m_shared_ram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	UINT8       m_scroll_x_lo;
	UINT8       m_scroll_x_hi;
	UINT8       m_scroll_y;
	int         m_bgpalbank;
	int         m_fgcolor;
	const UINT8 *m_color_codes;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8       m_nmi_mask;
	DECLARE_WRITE8_MEMBER(kyugo_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(kyugo_sub_cpu_control_w);
	DECLARE_WRITE8_MEMBER(kyugo_coin_counter_w);
	DECLARE_WRITE8_MEMBER(kyugo_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(kyugo_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(kyugo_bgattribram_w);
	DECLARE_READ8_MEMBER(kyugo_spriteram_2_r);
	DECLARE_WRITE8_MEMBER(kyugo_scroll_x_lo_w);
	DECLARE_WRITE8_MEMBER(kyugo_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(kyugo_scroll_y_w);
	DECLARE_WRITE8_MEMBER(kyugo_flipscreen_w);
	DECLARE_DRIVER_INIT(srdmissn);
	DECLARE_DRIVER_INIT(gyrodine);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_kyugo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
