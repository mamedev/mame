/*************************************************************************

    Ikki

*************************************************************************/

class ikki_state : public driver_device
{
public:
	ikki_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_scroll;
	size_t     m_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	bitmap_t   *m_sprite_bitmap;
	UINT8      m_flipscreen;
	int        m_punch_through_pen;
};


/*----------- defined in video/ikki.c -----------*/

WRITE8_HANDLER( ikki_scrn_ctrl_w );

PALETTE_INIT( ikki );
VIDEO_START( ikki );
SCREEN_UPDATE( ikki );
