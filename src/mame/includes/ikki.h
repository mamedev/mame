// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Ikki

*************************************************************************/

class ikki_state : public driver_device
{
public:
	ikki_state(const machine_config &mconfig, device_type type, const char *tag)
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
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	bitmap_ind16 m_sprite_bitmap;
	uint8_t      m_flipscreen;
	int        m_punch_through_pen;
	uint8_t      m_irq_source;

	uint8_t ikki_e000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ikki_coin_counters(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ikki_scrn_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_ikki(palette_device &palette);
	uint32_t screen_update_ikki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ikki_irq(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
