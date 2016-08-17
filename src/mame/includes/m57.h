// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
class m57_state : public driver_device
{
public:
	m57_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	int                  m_flipscreen;
	DECLARE_WRITE8_MEMBER(m57_videoram_w);
	DECLARE_WRITE8_MEMBER(m57_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(m57);
	UINT32 screen_update_m57(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
