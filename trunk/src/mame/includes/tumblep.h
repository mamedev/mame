/*************************************************************************

    Tumble Pop

*************************************************************************/

class tumblep_state : public driver_device
{
public:
	tumblep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT16 *  m_spriteram;
//  UINT16 *  m_paletteram;    // currently this uses generic palette handling (in decocomn.c)
	size_t    m_spriteram_size;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_deco_tilegen1;
};



/*----------- defined in video/tumblep.c -----------*/

SCREEN_UPDATE( tumblep );
