/***************************************************************************

    Blockout

***************************************************************************/

typedef struct _blockout_state blockout_state;
struct _blockout_state
{
	/* memory pointers */
	UINT16 * videoram;
	UINT16 * frontvideoram;
	UINT16 * paletteram;

	/* video-related */
	bitmap_t *tmpbitmap;
	UINT16   color;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/blockout.c -----------*/

WRITE16_HANDLER( blockout_videoram_w );
WRITE16_HANDLER( blockout_paletteram_w );
WRITE16_HANDLER( blockout_frontcolor_w );

VIDEO_START( blockout );
VIDEO_UPDATE( blockout );
