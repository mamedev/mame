class magmax_state : public driver_device
{
public:
	magmax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_vreg(*this, "vreg"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_y(*this, "scroll_y"){ }

	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_vreg;
	required_shared_ptr<UINT16> m_scroll_x;
	required_shared_ptr<UINT16> m_scroll_y;

	UINT8 m_sound_latch;
	UINT8 m_LS74_clr;
	UINT8 m_LS74_q;
	UINT8 m_gain_control;
	emu_timer *m_interrupt_timer;
	int m_flipscreen;
	UINT32 *m_prom_tab;
	bitmap_ind16 m_bitmap;
	DECLARE_WRITE16_MEMBER(magmax_sound_w);
	DECLARE_READ8_MEMBER(magmax_sound_irq_ack);
	DECLARE_READ8_MEMBER(magmax_sound_r);
	DECLARE_WRITE16_MEMBER(magmax_vreg_w);
	DECLARE_WRITE8_MEMBER(ay8910_portB_0_w);
	DECLARE_WRITE8_MEMBER(ay8910_portA_0_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_magmax(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
};
