/***************************************************************************

    Blockout

***************************************************************************/

class blockout_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blockout_state(machine)); }

	blockout_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 * videoram;
	UINT16 * frontvideoram;
	UINT16 * paletteram;

	/* video-related */
	bitmap_t *tmpbitmap;
	UINT16   color;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/blockout.c -----------*/

WRITE16_HANDLER( blockout_videoram_w );
WRITE16_HANDLER( blockout_paletteram_w );
WRITE16_HANDLER( blockout_frontcolor_w );

VIDEO_START( blockout );
VIDEO_UPDATE( blockout );
