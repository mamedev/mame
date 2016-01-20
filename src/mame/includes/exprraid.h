// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Express Raider

*************************************************************************/


class exprraid_state : public driver_device
{
public:
	exprraid_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_main_ram(*this, "main_ram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;

	/* memory pointers */
	required_shared_ptr<UINT8> m_main_ram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* protection */
	UINT8           m_prot_value;

	/* video-related */
	tilemap_t       *m_bg_tilemap;
	tilemap_t       *m_fg_tilemap;
	int             m_bg_index[4];

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_WRITE8_MEMBER(exprraid_int_clear_w);
	DECLARE_READ8_MEMBER(exprraid_prot_status_r);
	DECLARE_READ8_MEMBER(exprraid_prot_data_r);
	DECLARE_WRITE8_MEMBER(exprraid_prot_data_w);
	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(exprraid_videoram_w);
	DECLARE_WRITE8_MEMBER(exprraid_colorram_w);
	DECLARE_WRITE8_MEMBER(exprraid_flipscreen_w);
	DECLARE_WRITE8_MEMBER(exprraid_bgselect_w);
	DECLARE_WRITE8_MEMBER(exprraid_scrollx_w);
	DECLARE_WRITE8_MEMBER(exprraid_scrolly_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_deco16);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);

	DECLARE_READ8_MEMBER(sound_cpu_command_r);

	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_DRIVER_INIT(exprraid);
	DECLARE_DRIVER_INIT(wexpressb);
	DECLARE_DRIVER_INIT(wexpressb2);
	DECLARE_DRIVER_INIT(wexpressb3);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	UINT32 screen_update_exprraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void exprraid_gfx_expand();
};
