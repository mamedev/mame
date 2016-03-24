// license:BSD-3-Clause
// copyright-holders:David Haywood
class sbugger_state : public driver_device
{
public:
	sbugger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram_attr(*this, "videoram_attr"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_videoram_attr;
	required_shared_ptr<UINT8> m_videoram;

	tilemap_t *m_tilemap;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(videoram_attr_w);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void video_start() override;
	DECLARE_PALETTE_INIT(sbugger);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
