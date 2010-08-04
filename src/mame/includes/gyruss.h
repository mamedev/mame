/*************************************************************************

    Gyruss

*************************************************************************/

class gyruss_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gyruss_state(machine)); }

	gyruss_state(running_machine &machine)
		: driver_data_t(machine) { }

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
