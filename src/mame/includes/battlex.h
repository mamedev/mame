// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Battle Cross

***************************************************************************/

class battlex_state : public driver_device
{
public:
	battlex_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_in0_b4;

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	UINT8 m_scroll_lsb;
	UINT8 m_scroll_msb;
	UINT8 m_starfield_enabled;
	DECLARE_WRITE8_MEMBER(battlex_palette_w);
	DECLARE_WRITE8_MEMBER(battlex_scroll_x_lsb_w);
	DECLARE_WRITE8_MEMBER(battlex_scroll_x_msb_w);
	DECLARE_WRITE8_MEMBER(battlex_scroll_starfield_w);
	DECLARE_WRITE8_MEMBER(battlex_videoram_w);
	DECLARE_WRITE8_MEMBER(battlex_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(battlex_in0_b4_r);
	DECLARE_DRIVER_INIT(battlex);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_battlex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(battlex_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
