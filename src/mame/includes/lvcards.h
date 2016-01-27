// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder
class lvcards_state : public driver_device
{
public:
	lvcards_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	UINT8 m_payout;
	UINT8 m_pulse;
	UINT8 m_result;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(control_port_2_w);
	DECLARE_WRITE8_MEMBER(control_port_2a_w);
	DECLARE_READ8_MEMBER(payout_r);
	DECLARE_WRITE8_MEMBER(lvcards_videoram_w);
	DECLARE_WRITE8_MEMBER(lvcards_colorram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(lvcards);
	DECLARE_MACHINE_START(lvpoker);
	DECLARE_MACHINE_RESET(lvpoker);
	DECLARE_PALETTE_INIT(ponttehk);
	UINT32 screen_update_lvcards(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
