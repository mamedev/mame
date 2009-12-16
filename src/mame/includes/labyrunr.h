/*************************************************************************

    Labyrinth Runner

*************************************************************************/

typedef struct _labyrunr_state labyrunr_state;
struct _labyrunr_state
{
	/* memory pointers */
	UINT8 *    videoram1;
	UINT8 *    videoram2;
	UINT8 *    scrollram;
	UINT8 *    spriteram;
	UINT8 *    paletteram;

	/* video-related */
	tilemap    *layer0, *layer1;
	rectangle  clip0, clip1;

	/* devices */
	const device_config *k007121;
};


/*----------- defined in video/labyrunr.c -----------*/


WRITE8_HANDLER( labyrunr_vram1_w );
WRITE8_HANDLER( labyrunr_vram2_w );

PALETTE_INIT( labyrunr );
VIDEO_START( labyrunr );
VIDEO_UPDATE( labyrunr );
