/*************************************************************************

    Cops 01

*************************************************************************/

typedef struct _cop01_state cop01_state;
struct _cop01_state
{
	/* memory pointers */
	UINT8 *        bgvideoram;
	UINT8 *        fgvideoram;
	UINT8 *        spriteram;
	size_t         spriteram_size;

	/* video-related */
	tilemap        *bg_tilemap, *fg_tilemap;
	UINT8          vreg[4];

	/* sound-related */
	int            pulse;
	int            timer; 	// kludge for ym3526 in mightguy

	/* devices */
	const device_config *audiocpu;
};



/*----------- defined in video/cop01.c -----------*/


PALETTE_INIT( cop01 );
VIDEO_START( cop01 );
VIDEO_UPDATE( cop01 );

WRITE8_HANDLER( cop01_background_w );
WRITE8_HANDLER( cop01_foreground_w );
WRITE8_HANDLER( cop01_vreg_w );
