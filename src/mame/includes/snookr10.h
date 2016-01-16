// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
class snookr10_state : public driver_device
{
public:
	snookr10_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	int m_outportl;
	int m_outporth;
	int m_bit0;
	int m_bit1;
	int m_bit2;
	int m_bit3;
	int m_bit4;
	int m_bit5;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(dsw_port_1_r);
	DECLARE_READ8_MEMBER(port2000_8_r);
	DECLARE_WRITE8_MEMBER(output_port_0_w);
	DECLARE_WRITE8_MEMBER(output_port_1_w);
	DECLARE_WRITE8_MEMBER(snookr10_videoram_w);
	DECLARE_WRITE8_MEMBER(snookr10_colorram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(apple10_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(crystalc_get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(snookr10);
	DECLARE_VIDEO_START(apple10);
	DECLARE_VIDEO_START(crystalc);
	DECLARE_PALETTE_INIT(apple10);
	DECLARE_PALETTE_INIT(crystalc);
	UINT32 screen_update_snookr10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
