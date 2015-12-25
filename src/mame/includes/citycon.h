// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    City Connection

*************************************************************************/

class citycon_state : public driver_device
{
public:
	citycon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_linecolor(*this, "linecolor"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_linecolor;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_image;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(citycon_in_r);
	DECLARE_READ8_MEMBER(citycon_irq_ack_r);
	DECLARE_WRITE8_MEMBER(citycon_videoram_w);
	DECLARE_WRITE8_MEMBER(citycon_linecolor_w);
	DECLARE_WRITE8_MEMBER(citycon_background_w);
	DECLARE_DRIVER_INIT(citycon);
	TILEMAP_MAPPER_MEMBER(citycon_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_citycon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	inline void changecolor_RRRRGGGGBBBBxxxx( int color, int indx );
};
