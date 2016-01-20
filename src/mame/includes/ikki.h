// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Ikki

*************************************************************************/

class ikki_state : public driver_device
{
public:
	ikki_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_videoram(*this, "videoram"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	bitmap_ind16 m_sprite_bitmap;
	UINT8      m_flipscreen;
	int        m_punch_through_pen;
	UINT8      m_irq_source;

	DECLARE_READ8_MEMBER(ikki_e000_r);
	DECLARE_WRITE8_MEMBER(ikki_coin_counters);
	DECLARE_WRITE8_MEMBER(ikki_scrn_ctrl_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ikki);
	UINT32 screen_update_ikki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ikki_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
