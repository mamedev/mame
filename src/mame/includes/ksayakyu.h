// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Kusayakyuu

*************************************************************************/

class ksayakyu_state : public driver_device
{
public:
	ksayakyu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_tilemap;
	tilemap_t    *m_textmap;
	int        m_video_ctrl;
	int        m_flipscreen;

	/* misc */
	int        m_sound_status;
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(tomaincpu_w);
	DECLARE_WRITE8_MEMBER(ksayakyu_videoram_w);
	DECLARE_WRITE8_MEMBER(ksayakyu_videoctrl_w);
	DECLARE_WRITE8_MEMBER(dummy1_w);
	DECLARE_WRITE8_MEMBER(dummy2_w);
	DECLARE_WRITE8_MEMBER(dummy3_w);
	TILE_GET_INFO_MEMBER(get_ksayakyu_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ksayakyu);
	UINT32 screen_update_ksayakyu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
