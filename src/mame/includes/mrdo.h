/*************************************************************************

    Mr. Do

*************************************************************************/

class mrdo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mrdo_state(machine)); }

	mrdo_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    bgvideoram;
	UINT8 *    fgvideoram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t *bg_tilemap, *fg_tilemap;
	int       flipscreen;
};


/*----------- defined in video/mrdo.c -----------*/

WRITE8_HANDLER( mrdo_bgvideoram_w );
WRITE8_HANDLER( mrdo_fgvideoram_w );
WRITE8_HANDLER( mrdo_scrollx_w );
WRITE8_HANDLER( mrdo_scrolly_w );
WRITE8_HANDLER( mrdo_flipscreen_w );

PALETTE_INIT( mrdo );
VIDEO_START( mrdo );
VIDEO_UPDATE( mrdo );
