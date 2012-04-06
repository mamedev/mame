/***************************************************************************

    Sun Electronics Kangaroo hardware

    driver by Ville Laitinen

***************************************************************************/

class kangaroo_state : public driver_device
{
public:
	kangaroo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *      m_video_control;

	/* video-related */
	UINT32       *m_videoram;

	/* misc */
	UINT8        m_clock;
	DECLARE_READ8_MEMBER(mcu_sim_r);
	DECLARE_WRITE8_MEMBER(mcu_sim_w);
	DECLARE_WRITE8_MEMBER(kangaroo_coin_counter_w);
	DECLARE_WRITE8_MEMBER(kangaroo_videoram_w);
	DECLARE_WRITE8_MEMBER(kangaroo_video_control_w);
};




/*----------- defined in video/kangaroo.c -----------*/

VIDEO_START( kangaroo );
SCREEN_UPDATE_RGB32( kangaroo );

