

class fgoal_state : public driver_device
{
public:
	fgoal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;

	/* video-related */
	bitmap_ind16   m_bgbitmap;
	bitmap_ind16   m_fgbitmap;
	UINT8      m_xpos;
	UINT8      m_ypos;
	int        m_current_color;

	/* misc */
	int        m_fgoal_player;
	UINT8      m_row;
	UINT8      m_col;
	int        m_prev_coin;

	/* devices */
	cpu_device *m_maincpu;
	device_t *m_mb14241;
	DECLARE_READ8_MEMBER(fgoal_analog_r);
	DECLARE_READ8_MEMBER(fgoal_nmi_reset_r);
	DECLARE_READ8_MEMBER(fgoal_irq_reset_r);
	DECLARE_READ8_MEMBER(fgoal_row_r);
	DECLARE_WRITE8_MEMBER(fgoal_row_w);
	DECLARE_WRITE8_MEMBER(fgoal_col_w);
	DECLARE_READ8_MEMBER(fgoal_address_hi_r);
	DECLARE_READ8_MEMBER(fgoal_address_lo_r);
	DECLARE_READ8_MEMBER(fgoal_shifter_r);
	DECLARE_READ8_MEMBER(fgoal_shifter_reverse_r);
	DECLARE_WRITE8_MEMBER(fgoal_sound1_w);
	DECLARE_WRITE8_MEMBER(fgoal_sound2_w);
	DECLARE_WRITE8_MEMBER(fgoal_color_w);
	DECLARE_WRITE8_MEMBER(fgoal_ypos_w);
	DECLARE_WRITE8_MEMBER(fgoal_xpos_w);
	DECLARE_CUSTOM_INPUT_MEMBER(fgoal_80_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_fgoal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/fgoal.c -----------*/





