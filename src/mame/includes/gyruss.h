/*************************************************************************

    Gyruss

*************************************************************************/

class gyruss_state : public driver_device
{
public:
	gyruss_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	UINT8 *    flipscreen;

	/* video-related */
	tilemap_t    *tilemap;

	/* devices */
	cpu_device *audiocpu;
	cpu_device *audiocpu_2;
};


/*----------- defined in video/gyruss.c -----------*/

WRITE8_HANDLER( gyruss_spriteram_w );
READ8_HANDLER( gyruss_scanline_r );

PALETTE_INIT( gyruss );
VIDEO_START( gyruss );
VIDEO_UPDATE( gyruss );
