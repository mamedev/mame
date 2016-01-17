// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/*************************************************************************

    Goindol

*************************************************************************/

class goindol_state : public driver_device
{
public:
	goindol_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_fg_scrolly(*this, "fg_scrolly"),
		m_fg_scrollx(*this, "fg_scrollx"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	required_shared_ptr<UINT8> m_fg_scrolly;
	required_shared_ptr<UINT8> m_fg_scrollx;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_fg_videoram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	UINT16      m_char_bank;

	/* misc */
	int         m_prot_toggle;
	DECLARE_WRITE8_MEMBER(goindol_bankswitch_w);
	DECLARE_READ8_MEMBER(prot_f422_r);
	DECLARE_WRITE8_MEMBER(prot_fc44_w);
	DECLARE_WRITE8_MEMBER(prot_fd99_w);
	DECLARE_WRITE8_MEMBER(prot_fc66_w);
	DECLARE_WRITE8_MEMBER(prot_fcb0_w);
	DECLARE_WRITE8_MEMBER(goindol_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(goindol_bg_videoram_w);
	DECLARE_DRIVER_INIT(goindol);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_goindol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int gfxbank, UINT8 *sprite_ram );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
