// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Bank Panic

***************************************************************************/

class bankp_state : public driver_device
{
public:
	bankp_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int     m_scroll_x;
	int     m_priority;

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(bankp_scroll_w);
	DECLARE_WRITE8_MEMBER(bankp_videoram_w);
	DECLARE_WRITE8_MEMBER(bankp_colorram_w);
	DECLARE_WRITE8_MEMBER(bankp_videoram2_w);
	DECLARE_WRITE8_MEMBER(bankp_colorram2_w);
	DECLARE_WRITE8_MEMBER(bankp_out_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(bankp);
	UINT32 screen_update_bankp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
