/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

typedef struct _yunsung8_state yunsung8_state;
struct _yunsung8_state
{
	/* memory pointers */
	UINT8 *     videoram;

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1;
	UINT8       *videoram_0, *videoram_1;
	int         layers_ctrl;
	int         videobank;

	/* misc */
	int         adpcm;
	int         toggle;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/yunsung8.c -----------*/

WRITE8_HANDLER( yunsung8_videobank_w );
READ8_HANDLER ( yunsung8_videoram_r );
WRITE8_HANDLER( yunsung8_videoram_w );
WRITE8_HANDLER( yunsung8_flipscreen_w );

VIDEO_START( yunsung8 );
VIDEO_UPDATE( yunsung8 );
