/*************************************************************************

    Mr. Flea

*************************************************************************/

class mrflea_state : public driver_device
{
public:
	mrflea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int     m_gfx_bank;

	/* misc */
	int m_io;
	int m_main;
	int m_status;
	int m_select1;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
};


/*----------- defined in video/mrflea.c -----------*/

WRITE8_HANDLER( mrflea_gfx_bank_w );
WRITE8_HANDLER( mrflea_videoram_w );
WRITE8_HANDLER( mrflea_spriteram_w );

SCREEN_UPDATE( mrflea );
