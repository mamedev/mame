/*************************************************************************

    Markham

*************************************************************************/

class markham_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, markham_state(machine)); }

	markham_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    xscroll;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
};


/*----------- defined in video/markham.c -----------*/

WRITE8_HANDLER( markham_videoram_w );
WRITE8_HANDLER( markham_flipscreen_w );

PALETTE_INIT( markham );
VIDEO_START( markham );
VIDEO_UPDATE( markham );
