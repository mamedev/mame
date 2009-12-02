
typedef struct _gotya_state gotya_state;
struct _gotya_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  videoram2;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  scroll;

	/* video-related */
	tilemap  *bg_tilemap;
	int      scroll_bit_8;

	/* sound-related */
	int      theme_playing;

	/* devices */
	const device_config *samples;
};


/*----------- defined in audio/gotya.c -----------*/

WRITE8_HANDLER( gotya_soundlatch_w );


/*----------- defined in video/gotya.c -----------*/

WRITE8_HANDLER( gotya_videoram_w );
WRITE8_HANDLER( gotya_colorram_w );
WRITE8_HANDLER( gotya_video_control_w );

PALETTE_INIT( gotya );
VIDEO_START( gotya );
VIDEO_UPDATE( gotya );
