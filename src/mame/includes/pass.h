// license:BSD-3-Clause
// copyright-holders:David Haywood
class pass_state : public driver_device
{
public:
	pass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_fg_videoram;
	DECLARE_WRITE16_MEMBER(pass_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(pass_fg_videoram_w);
	TILE_GET_INFO_MEMBER(get_pass_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_pass_fg_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_pass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
