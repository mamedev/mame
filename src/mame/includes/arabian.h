/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

class arabian_state : public driver_device
{
public:
	arabian_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_blitter;
	UINT8 *  m_custom_cpu_ram;

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
};


/*----------- defined in video/arabian.c -----------*/

WRITE8_HANDLER( arabian_blitter_w );
WRITE8_HANDLER( arabian_videoram_w );

PALETTE_INIT( arabian );
VIDEO_START( arabian );
SCREEN_UPDATE_IND16( arabian );
