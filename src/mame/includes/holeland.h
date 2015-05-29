// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Hole Land

*************************************************************************/

class holeland_state : public driver_device
{
public:
	holeland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_palette_offset;
	int        m_po[2];
	DECLARE_WRITE8_MEMBER(holeland_videoram_w);
	DECLARE_WRITE8_MEMBER(holeland_colorram_w);
	DECLARE_WRITE8_MEMBER(holeland_pal_offs_w);
	DECLARE_WRITE8_MEMBER(holeland_scroll_w);
	DECLARE_WRITE8_MEMBER(holeland_flipscreen_w);
	TILE_GET_INFO_MEMBER(holeland_get_tile_info);
	TILE_GET_INFO_MEMBER(crzrally_get_tile_info);
	DECLARE_VIDEO_START(holeland);
	DECLARE_VIDEO_START(crzrally);
	UINT32 screen_update_holeland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_crzrally(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void holeland_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void crzrally_draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
