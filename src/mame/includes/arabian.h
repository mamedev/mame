/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

class arabian_state : public driver_device
{
public:
	arabian_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_custom_cpu_ram(*this, "custom_cpu_ram"),
		m_blitter(*this, "blitter"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_custom_cpu_ram;
	required_shared_ptr<UINT8> m_blitter;

	UINT8 *  m_main_bitmap;
	UINT8 *  m_converted_gfx;

	/* video-related */
	UINT8    m_video_control;
	UINT8    m_flip_screen;

	/* MCU */
	UINT8    m_mcu_port_o;
	UINT8    m_mcu_port_p;
	UINT8    m_mcu_port_r[4];
	DECLARE_READ8_MEMBER(mcu_port_r_r);
	DECLARE_WRITE8_MEMBER(mcu_port_r_w);
	DECLARE_READ8_MEMBER(mcu_portk_r);
	DECLARE_WRITE8_MEMBER(mcu_port_o_w);
	DECLARE_WRITE8_MEMBER(mcu_port_p_w);
	DECLARE_WRITE8_MEMBER(arabian_blitter_w);
	DECLARE_WRITE8_MEMBER(arabian_videoram_w);
	DECLARE_WRITE8_MEMBER(ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(ay8910_portb_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_arabian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
