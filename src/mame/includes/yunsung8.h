/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

class yunsung8_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, yunsung8_state(machine)); }

	yunsung8_state(running_machine &machine) { }

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
	running_device *audiocpu;
};


/*----------- defined in video/yunsung8.c -----------*/

WRITE8_HANDLER( yunsung8_videobank_w );
READ8_HANDLER ( yunsung8_videoram_r );
WRITE8_HANDLER( yunsung8_videoram_w );
WRITE8_HANDLER( yunsung8_flipscreen_w );

VIDEO_START( yunsung8 );
VIDEO_UPDATE( yunsung8 );
