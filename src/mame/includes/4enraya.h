/*************************************************************************

    4enraya

*************************************************************************/

typedef struct __4enraya_state _4enraya_state;
struct __4enraya_state
{
	/* memory pointers */
	UINT8 *    videoram;
	size_t     videoram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;

	/* sound-related */
	int        soundlatch;
	int        last_snd_ctrl;
};


/*----------- defined in video/4enraya.c -----------*/

WRITE8_HANDLER( fenraya_videoram_w );

VIDEO_START( 4enraya );
VIDEO_UPDATE( 4enraya );
