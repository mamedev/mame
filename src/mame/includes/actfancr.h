/*************************************************************************

    Act Fancer

*************************************************************************/

class actfancr_state : public driver_device
{
public:
	actfancr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_main_ram;
	UINT16 m_spriteram16[0x800/2]; // a 16-bit copy of spriteram for use with the MXC06 code

	/* video-related */
	int         		m_flipscreen;

	/* misc */
	int            m_trio_control_select;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/actfancr.c -----------*/

WRITE8_HANDLER( actfancr_pf1_data_w );
READ8_HANDLER( actfancr_pf1_data_r );
WRITE8_HANDLER( actfancr_pf1_control_w );
WRITE8_HANDLER( actfancr_pf2_data_w );
READ8_HANDLER( actfancr_pf2_data_r );
WRITE8_HANDLER( actfancr_pf2_control_w );

VIDEO_START( actfancr );
SCREEN_UPDATE_IND16( actfancr );

