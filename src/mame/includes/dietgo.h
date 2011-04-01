/*************************************************************************

    Diet Go Go

*************************************************************************/

class dietgo_state : public driver_device
{
public:
	dietgo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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



/*----------- defined in video/dietgo.c -----------*/

SCREEN_UPDATE( dietgo );
