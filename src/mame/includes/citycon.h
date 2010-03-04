/*************************************************************************

    City Connection

*************************************************************************/

class citycon_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, citycon_state(machine)); }

	citycon_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        linecolor;
	UINT8 *        scroll;
	UINT8 *        spriteram;
//  UINT8 *        paletteram;  // currently this uses generic palette handling
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap,*fg_tilemap;
	int            bg_image;

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/citycon.c -----------*/

WRITE8_HANDLER( citycon_videoram_w );
WRITE8_HANDLER( citycon_linecolor_w );
WRITE8_HANDLER( citycon_background_w );

VIDEO_UPDATE( citycon );
VIDEO_START( citycon );
