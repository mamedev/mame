/*************************************************************************

    Mad Motor

*************************************************************************/

class madmotor_state : public driver_device
{
public:
	madmotor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
//  UINT16 *        m_paletteram;     // this currently uses generic palette handlers

	/* video-related */
	int             m_flipscreen;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(madmotor_sound_w);
};


/*----------- defined in video/madmotor.c -----------*/

VIDEO_START( madmotor );
SCREEN_UPDATE_IND16( madmotor );
