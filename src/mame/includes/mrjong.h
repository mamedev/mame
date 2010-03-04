/*************************************************************************

    Mr. Jong

*************************************************************************/

class mrjong_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mrjong_state(machine)); }

	mrjong_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;

	/* video-related */
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/mrjong.c -----------*/

WRITE8_HANDLER( mrjong_videoram_w );
WRITE8_HANDLER( mrjong_colorram_w );
WRITE8_HANDLER( mrjong_flipscreen_w );

PALETTE_INIT( mrjong );
VIDEO_START( mrjong );
VIDEO_UPDATE( mrjong );
