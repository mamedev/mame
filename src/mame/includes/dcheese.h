// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    HAR MadMax hardware

**************************************************************************/


class dcheese_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER_SCANLINE,
		TIMER_SIGNAL_IRQ
	};

	dcheese_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen") { }

	/* video-related */
	UINT16   m_blitter_color[2];
	UINT16   m_blitter_xparam[16];
	UINT16   m_blitter_yparam[16];
	UINT16   m_blitter_vidparam[32];

	std::unique_ptr<bitmap_ind16> m_dstbitmap;
	emu_timer *m_blitter_timer;

	/* misc */
	UINT8    m_irq_state[5];
	UINT8    m_soundlatch_full;
	UINT8    m_sound_control;
	UINT8    m_sound_msb_latch;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	device_t *m_bsmt;
	DECLARE_WRITE16_MEMBER(eeprom_control_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_WRITE8_MEMBER(bsmt_data_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_color_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_xparam_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_yparam_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_vidparam_w);
	DECLARE_WRITE16_MEMBER(madmax_blitter_unknown_w);
	DECLARE_READ16_MEMBER(madmax_blitter_vidparam_r);
	DECLARE_CUSTOM_INPUT_MEMBER(sound_latch_state_r);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(dcheese);
	UINT32 screen_update_dcheese(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(dcheese_vblank);
	void dcheese_signal_irq(int which);
	void update_irq_state();
	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_scanline_irq();
	void do_clear(  );
	void do_blit(  );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in drivers/dcheese.c -----------*/
