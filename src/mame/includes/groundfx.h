struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int pri;
};

class groundfx_state : public driver_device
{
public:
	groundfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram") { }

	required_shared_ptr<UINT32> m_ram;
	required_shared_ptr<UINT32> m_spriteram;

	UINT16 m_coin_word;
	UINT16 m_frame_counter;
	UINT16 m_port_sel;
	struct tempsprite *m_spritelist;
	UINT16 m_rotate_ctrl[8];
	rectangle m_hack_cliprect;

	DECLARE_WRITE32_MEMBER(color_ram_w);
	DECLARE_WRITE32_MEMBER(groundfx_input_w);
	DECLARE_READ32_MEMBER(groundfx_adc_r);
	DECLARE_WRITE32_MEMBER(groundfx_adc_w);
	DECLARE_WRITE32_MEMBER(rotate_control_w);
	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_READ32_MEMBER(irq_speedup_r_groundfx);
	DECLARE_CUSTOM_INPUT_MEMBER(frame_counter_r);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_word_r);
	DECLARE_DRIVER_INIT(groundfx);
	virtual void video_start();
	UINT32 screen_update_groundfx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(groundfx_interrupt);
};
