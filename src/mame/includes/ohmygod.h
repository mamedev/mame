// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Oh My God!

*************************************************************************/

class ohmygod_state : public driver_device
{
public:
	ohmygod_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }


	/* memory pointers */
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int m_spritebank;
	UINT16 m_scrollx;
	UINT16 m_scrolly;

	/* misc */
	int m_adpcm_bank_shift;
	int m_sndbank;
	DECLARE_WRITE16_MEMBER(ohmygod_ctrl_w);
	DECLARE_WRITE16_MEMBER(ohmygod_videoram_w);
	DECLARE_WRITE16_MEMBER(ohmygod_spritebank_w);
	DECLARE_WRITE16_MEMBER(ohmygod_scrollx_w);
	DECLARE_WRITE16_MEMBER(ohmygod_scrolly_w);
	DECLARE_DRIVER_INIT(ohmygod);
	DECLARE_DRIVER_INIT(naname);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_ohmygod(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
