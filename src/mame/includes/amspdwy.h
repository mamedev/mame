/*************************************************************************

    American Speedway

*************************************************************************/

class amspdwy_state : public driver_device
{
public:
	amspdwy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_colorram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_flipscreen;

	/* misc */
	UINT8      m_wheel_old[2];
	UINT8      m_wheel_return[2];

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/amspdwy.c -----------*/

WRITE8_HANDLER( amspdwy_videoram_w );
WRITE8_HANDLER( amspdwy_colorram_w );
WRITE8_HANDLER( amspdwy_paletteram_w );
WRITE8_HANDLER( amspdwy_flipscreen_w );

VIDEO_START( amspdwy );
SCREEN_UPDATE( amspdwy );
