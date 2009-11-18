/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

typedef struct _arabian_state arabian_state;
struct _arabian_state
{
	/* memory pointers */
	UINT8 *  blitter;
	UINT8 *  custom_cpu_ram;

	UINT8 *  main_bitmap;
	UINT8 *  converted_gfx;

	/* video-related */
	UINT8    video_control;
	UINT8    flip_screen;

	/* misc */
	UINT8    custom_cpu_reset;
	UINT8    custom_cpu_busy;
};


/*----------- defined in video/arabian.c -----------*/

WRITE8_HANDLER( arabian_blitter_w );
WRITE8_HANDLER( arabian_videoram_w );

PALETTE_INIT( arabian );
VIDEO_START( arabian );
VIDEO_UPDATE( arabian );
