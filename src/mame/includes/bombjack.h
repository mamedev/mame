/*************************************************************************

    Bomb Jack

*************************************************************************/

class bombjack_state : public driver_device
{
public:
	bombjack_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;
	UINT8      m_background_image;

	/* sound-related */
	UINT8      m_latch;
};


/*----------- defined in video/bombjack.c -----------*/

WRITE8_HANDLER( bombjack_videoram_w );
WRITE8_HANDLER( bombjack_colorram_w );
WRITE8_HANDLER( bombjack_background_w );
WRITE8_HANDLER( bombjack_flipscreen_w );

VIDEO_START( bombjack );
SCREEN_UPDATE( bombjack );
