/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

class galspnbl_state : public driver_device
{
public:
	galspnbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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
	DECLARE_WRITE16_MEMBER(soundcommand_w);
};


/*----------- defined in video/galspnbl.c -----------*/


PALETTE_INIT( galspnbl );
SCREEN_UPDATE_IND16( galspnbl );
