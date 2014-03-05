/*************************************************************************

    Bogey Manor

*************************************************************************/

class bogeyman_state : public driver_device
{
public:
	bogeyman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_spriteram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;

	/* misc */
	int        m_psg_latch;
	int        m_last_write;
	int        m_colbank;
	DECLARE_WRITE8_MEMBER(bogeyman_8910_latch_w);
	DECLARE_WRITE8_MEMBER(bogeyman_8910_control_w);
	DECLARE_WRITE8_MEMBER(bogeyman_videoram_w);
	DECLARE_WRITE8_MEMBER(bogeyman_colorram_w);
	DECLARE_WRITE8_MEMBER(bogeyman_videoram2_w);
	DECLARE_WRITE8_MEMBER(bogeyman_colorram2_w);
	DECLARE_WRITE8_MEMBER(bogeyman_paletteram_w);
	DECLARE_WRITE8_MEMBER(bogeyman_colbank_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(bogeyman);
	UINT32 screen_update_bogeyman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
