
class gotya_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gotya_state(machine)); }

	gotya_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  videoram2;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  scroll;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int      scroll_bit_8;

	/* sound-related */
	int      theme_playing;

	/* devices */
	running_device *samples;
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
