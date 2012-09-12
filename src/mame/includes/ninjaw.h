/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/

class ninjaw_state : public driver_device
{
public:
	ninjaw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;

	/* misc */
	UINT16     m_cpua_ctrl;
	INT32      m_banknum;
	int        m_pandata[4];

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	cpu_device *m_subcpu;
	device_t *m_tc0140syt;
	device_t *m_tc0100scn_1;
	device_t *m_tc0100scn_2;
	device_t *m_tc0100scn_3;
	device_t *m_lscreen;
	device_t *m_mscreen;
	device_t *m_rscreen;
	device_t *m_2610_1l;
	device_t *m_2610_1r;
	device_t *m_2610_2l;
	device_t *m_2610_2r;
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(ninjaw_sound_w);
	DECLARE_READ16_MEMBER(ninjaw_sound_r);
	DECLARE_WRITE8_MEMBER(ninjaw_pancontrol);
	DECLARE_WRITE16_MEMBER(tc0100scn_triple_screen_w);
};


/*----------- defined in video/ninjaw.c -----------*/

VIDEO_START( ninjaw );
SCREEN_UPDATE_IND16( ninjaw_left );
SCREEN_UPDATE_IND16( ninjaw_middle );
SCREEN_UPDATE_IND16( ninjaw_right );
