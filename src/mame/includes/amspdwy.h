/*************************************************************************

    American Speedway

*************************************************************************/

typedef struct _amspdwy_state amspdwy_state;
struct _amspdwy_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    colorram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;
	int        flipscreen;

	/* misc */
	UINT8      wheel_old[2];
	UINT8      wheel_return[2];

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/amspdwy.c -----------*/

WRITE8_HANDLER( amspdwy_videoram_w );
WRITE8_HANDLER( amspdwy_colorram_w );
WRITE8_HANDLER( amspdwy_paletteram_w );
WRITE8_HANDLER( amspdwy_flipscreen_w );

VIDEO_START( amspdwy );
VIDEO_UPDATE( amspdwy );
