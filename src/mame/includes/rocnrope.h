

typedef struct _rocnrope_state rocnrope_state;
struct _rocnrope_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	UINT8 *    spriteram2;
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;
};


/*----------- defined in video/rocnrope.c -----------*/

WRITE8_HANDLER( rocnrope_videoram_w );
WRITE8_HANDLER( rocnrope_colorram_w );
WRITE8_HANDLER( rocnrope_flipscreen_w );

PALETTE_INIT( rocnrope );
VIDEO_START( rocnrope );
VIDEO_UPDATE( rocnrope );
