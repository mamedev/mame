// license:BSD-3-Clause
// copyright-holders:Uki
class strnskil_state : public driver_device
{
public:
	strnskil_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_xscroll(*this, "xscroll"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_xscroll;
	required_shared_ptr<UINT8> m_spriteram;

	UINT8 m_scrl_ctrl;
	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_source;

	TIMER_DEVICE_CALLBACK_MEMBER(strnskil_irq);

	DECLARE_READ8_MEMBER(strnskil_d800_r);
	DECLARE_READ8_MEMBER(pettanp_protection_r);
	DECLARE_READ8_MEMBER(banbam_protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(strnskil_videoram_w);
	DECLARE_WRITE8_MEMBER(strnskil_scrl_ctrl_w);

	DECLARE_DRIVER_INIT(banbam);
	DECLARE_DRIVER_INIT(pettanp);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(strnskil);
	UINT32 screen_update_strnskil(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
