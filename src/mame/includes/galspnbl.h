/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

class galspnbl_state : public driver_device
{
public:
	galspnbl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m_videoram;
	UINT16 *    m_bgvideoram;
	UINT16 *    m_colorram;
	UINT16 *    m_scroll;
	UINT16 *    m_spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/galspnbl.c -----------*/


PALETTE_INIT( galspnbl );
SCREEN_UPDATE( galspnbl );
