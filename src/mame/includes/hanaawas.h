// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Hana Awase

*************************************************************************/

class hanaawas_state : public driver_device
{
public:
	hanaawas_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_mux;
	DECLARE_READ8_MEMBER(hanaawas_input_port_0_r);
	DECLARE_WRITE8_MEMBER(hanaawas_inputs_mux_w);
	DECLARE_WRITE8_MEMBER(hanaawas_videoram_w);
	DECLARE_WRITE8_MEMBER(hanaawas_colorram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(hanaawas);
	UINT32 screen_update_hanaawas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(hanaawas_portB_w);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
