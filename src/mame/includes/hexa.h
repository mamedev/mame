

typedef struct _hexa_state hexa_state;
struct _hexa_state
{
	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int      charbank;

};


/* ----------- defined in video/hexa.c -----------*/

WRITE8_HANDLER( hexa_videoram_w );
WRITE8_HANDLER( hexa_d008_w );

VIDEO_START( hexa );
VIDEO_UPDATE( hexa );
