/*************************************************************************

    Son Son

*************************************************************************/

typedef struct _sonson_state sonson_state;
struct _sonson_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     videoram_size;
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;

	/* misc */
	int        last_irq;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/sonson.c -----------*/

WRITE8_HANDLER( sonson_videoram_w );
WRITE8_HANDLER( sonson_colorram_w );
WRITE8_HANDLER( sonson_scrollx_w );
WRITE8_HANDLER( sonson_flipscreen_w );

PALETTE_INIT( sonson );
VIDEO_START( sonson );
VIDEO_UPDATE( sonson );
