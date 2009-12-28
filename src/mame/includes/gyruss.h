/*************************************************************************

    Gyruss

*************************************************************************/

typedef struct _gyruss_state gyruss_state;
struct _gyruss_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	UINT8 *    flipscreen;

	/* video-related */
	tilemap_t    *tilemap;

	/* devices */
	const device_config *audiocpu;
	const device_config *audiocpu_2;
};


/*----------- defined in video/gyruss.c -----------*/

WRITE8_HANDLER( gyruss_spriteram_w );
READ8_HANDLER( gyruss_scanline_r );

PALETTE_INIT( gyruss );
VIDEO_START( gyruss );
VIDEO_UPDATE( gyruss );
