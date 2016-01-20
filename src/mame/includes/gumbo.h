// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Gumbo - Miss Bingo - Miss Puzzle

*************************************************************************/

class gumbo_state : public driver_device
{
public:
	gumbo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_fg_videoram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(gumbo_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(gumbo_fg_videoram_w);
	TILE_GET_INFO_MEMBER(get_gumbo_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_gumbo_fg_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_gumbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
