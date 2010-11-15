class m52_state : public driver_device
{
public:
	m52_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *              videoram;
	UINT8 *              spriteram;
	size_t               spriteram_size;

	UINT8 *              colorram;

	/* video-related */
	tilemap_t*             bg_tilemap;
	UINT8                bg1xpos, bg1ypos;
	UINT8                bg2xpos, bg2ypos;
	UINT8                bgcontrol;
};

/*----------- defined in video/m52.c -----------*/

READ8_HANDLER( m52_protection_r );
WRITE8_HANDLER( m52_scroll_w );
WRITE8_HANDLER( m52_bg1xpos_w );
WRITE8_HANDLER( m52_bg1ypos_w );
WRITE8_HANDLER( m52_bg2xpos_w );
WRITE8_HANDLER( m52_bg2ypos_w );
WRITE8_HANDLER( m52_bgcontrol_w );
WRITE8_HANDLER( m52_flipscreen_w );
WRITE8_HANDLER( alpha1v_flipscreen_w );
WRITE8_HANDLER( m52_videoram_w );
WRITE8_HANDLER( m52_colorram_w );

PALETTE_INIT( m52 );
VIDEO_START( m52 );
VIDEO_UPDATE( m52 );
