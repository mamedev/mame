/*************************************************************************

    Mr. Jong

*************************************************************************/

class mrjong_state : public driver_device
{
public:
	mrjong_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/mrjong.c -----------*/

WRITE8_HANDLER( mrjong_videoram_w );
WRITE8_HANDLER( mrjong_colorram_w );
WRITE8_HANDLER( mrjong_flipscreen_w );

PALETTE_INIT( mrjong );
VIDEO_START( mrjong );
SCREEN_UPDATE( mrjong );
