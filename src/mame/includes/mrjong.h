/*************************************************************************

    Mr. Jong

*************************************************************************/

typedef struct _mrjong_state mrjong_state;
struct _mrjong_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;

	/* video-related */
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/mrjong.c -----------*/

WRITE8_HANDLER( mrjong_videoram_w );
WRITE8_HANDLER( mrjong_colorram_w );
WRITE8_HANDLER( mrjong_flipscreen_w );

PALETTE_INIT( mrjong );
VIDEO_START( mrjong );
VIDEO_UPDATE( mrjong );
