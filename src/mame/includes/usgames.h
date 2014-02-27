class usgames_state : public driver_device
{
public:
	usgames_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_charram;
	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(usgames_rombank_w);
	DECLARE_WRITE8_MEMBER(lamps1_w);
	DECLARE_WRITE8_MEMBER(lamps2_w);
	DECLARE_WRITE8_MEMBER(usgames_videoram_w);
	DECLARE_WRITE8_MEMBER(usgames_charram_w);
	TILE_GET_INFO_MEMBER(get_usgames_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(usgames);
	UINT32 screen_update_usgames(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
