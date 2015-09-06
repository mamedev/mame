// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Himeshikibu

*************************************************************************/

class himesiki_state : public driver_device
{
public:
	himesiki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram(*this, "spriteram"),
		m_subcpu(*this, "sub"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bg_ram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int          m_scrollx[2];
	int        m_flipscreen;

	/* devices */
	required_device<cpu_device> m_subcpu;
	DECLARE_WRITE8_MEMBER(himesiki_rombank_w);
	DECLARE_WRITE8_MEMBER(himesiki_sound_w);
	DECLARE_WRITE8_MEMBER(himesiki_bg_ram_w);
	DECLARE_WRITE8_MEMBER(himesiki_scrollx_w);
	DECLARE_WRITE8_MEMBER(himesiki_flip_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_himesiki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void himesiki_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
