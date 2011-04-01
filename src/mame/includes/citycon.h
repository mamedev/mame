/*************************************************************************

    City Connection

*************************************************************************/

class citycon_state : public driver_device
{
public:
	citycon_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_linecolor;
	UINT8 *        m_scroll;
	UINT8 *        m_spriteram;
//  UINT8 *        m_paletteram;  // currently this uses generic palette handling
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_image;

	/* devices */
	device_t *m_maincpu;
};


/*----------- defined in video/citycon.c -----------*/

WRITE8_HANDLER( citycon_videoram_w );
WRITE8_HANDLER( citycon_linecolor_w );
WRITE8_HANDLER( citycon_background_w );

SCREEN_UPDATE( citycon );
VIDEO_START( citycon );
