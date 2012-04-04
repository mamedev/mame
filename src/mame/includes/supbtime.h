/*************************************************************************

    Super Burger Time & China Town

*************************************************************************/

class supbtime_state : public driver_device
{
public:
	supbtime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT16 *  m_spriteram;
//  UINT16 *  m_paletteram;    // currently this uses generic palette handling (in decocomn.c)
	size_t    m_spriteram_size;

	/* video-related */

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_deco_tilegen1;
	DECLARE_READ16_MEMBER(supbtime_controls_r);
	DECLARE_WRITE16_MEMBER(sound_w);
};



/*----------- defined in video/supbtime.c -----------*/

SCREEN_UPDATE_IND16( supbtime );
