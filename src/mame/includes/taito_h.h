/*************************************************************************

    Taito H system

*************************************************************************/

class taitoh_state : public driver_device
{
public:
	taitoh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_m68000_mainram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* misc */
	INT32       m_banknum;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_tc0080vco;
	device_t *m_tc0220ioc;
	DECLARE_READ8_MEMBER(syvalion_input_bypass_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
};

/*----------- defined in video/taito_h.c -----------*/

SCREEN_UPDATE_IND16( syvalion );
SCREEN_UPDATE_IND16( recordbr );
SCREEN_UPDATE_IND16( dleague );
