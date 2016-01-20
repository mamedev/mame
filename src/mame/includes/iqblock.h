// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Ernesto Corvi
class iqblock_state : public driver_device
{
public:
	iqblock_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_rambase;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;

	int m_videoenable;
	int m_video_type;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	DECLARE_WRITE8_MEMBER(iqblock_prot_w);
	DECLARE_WRITE8_MEMBER(grndtour_prot_w);
	DECLARE_WRITE8_MEMBER(irqack_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(fgscroll_w);
	DECLARE_WRITE8_MEMBER(port_C_w);

	TIMER_DEVICE_CALLBACK_MEMBER(irq);

	DECLARE_DRIVER_INIT(grndtour);
	DECLARE_DRIVER_INIT(iqblock);
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
