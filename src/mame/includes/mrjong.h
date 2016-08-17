// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/*************************************************************************

    Mr. Jong

*************************************************************************/

class mrjong_state : public driver_device
{
public:
	mrjong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(io_0x03_r);
	DECLARE_WRITE8_MEMBER(mrjong_videoram_w);
	DECLARE_WRITE8_MEMBER(mrjong_colorram_w);
	DECLARE_WRITE8_MEMBER(mrjong_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mrjong);
	UINT32 screen_update_mrjong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
