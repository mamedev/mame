/*************************************************************************

    Bogey Manor

*************************************************************************/

typedef struct _bogeyman_state bogeyman_state;
struct _bogeyman_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    videoram2;
	UINT8 *    colorram;
	UINT8 *    colorram2;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap, *fg_tilemap;

	/* misc */
	int        psg_latch;
	int        last_write;
};


/* defined in video/bogeyman.c */

WRITE8_HANDLER( bogeyman_videoram_w );
WRITE8_HANDLER( bogeyman_colorram_w );
WRITE8_HANDLER( bogeyman_videoram2_w );
WRITE8_HANDLER( bogeyman_colorram2_w );
WRITE8_HANDLER( bogeyman_paletteram_w );

PALETTE_INIT( bogeyman );
VIDEO_START( bogeyman );
VIDEO_UPDATE( bogeyman );
