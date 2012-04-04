/*************************************************************************

    Taito Dual Screen Games

*************************************************************************/

class warriorb_state : public driver_device
{
public:
	warriorb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *   m_spriteram;
	size_t     m_spriteram_size;

	/* misc */
	INT32      m_banknum;
	int        m_pandata[4];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_tc0140syt;
	device_t *m_tc0100scn_1;
	device_t *m_tc0100scn_2;
	device_t *m_lscreen;
	device_t *m_rscreen;
	device_t *m_2610_1l;
	device_t *m_2610_1r;
	device_t *m_2610_2l;
	device_t *m_2610_2r;
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(warriorb_sound_w);
	DECLARE_READ16_MEMBER(warriorb_sound_r);
	DECLARE_WRITE8_MEMBER(warriorb_pancontrol);
	DECLARE_WRITE16_MEMBER(tc0100scn_dual_screen_w);
};


/*----------- defined in video/warriorb.c -----------*/

VIDEO_START( warriorb );
SCREEN_UPDATE_IND16( warriorb_left );
SCREEN_UPDATE_IND16( warriorb_right );
