/*************************************************************************

    Markham

*************************************************************************/

class markham_state : public driver_device
{
public:
	markham_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_xscroll;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
};


/*----------- defined in video/markham.c -----------*/

WRITE8_HANDLER( markham_videoram_w );
WRITE8_HANDLER( markham_flipscreen_w );

PALETTE_INIT( markham );
VIDEO_START( markham );
SCREEN_UPDATE( markham );
