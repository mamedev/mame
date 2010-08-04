/*************************************************************************

    Bomb Jack

*************************************************************************/

class bombjack_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bombjack_state(machine)); }

	bombjack_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *fg_tilemap, *bg_tilemap;
	UINT8      background_image;

	/* sound-related */
	UINT8      latch;
};


/*----------- defined in video/bombjack.c -----------*/

WRITE8_HANDLER( bombjack_videoram_w );
WRITE8_HANDLER( bombjack_colorram_w );
WRITE8_HANDLER( bombjack_background_w );
WRITE8_HANDLER( bombjack_flipscreen_w );

VIDEO_START( bombjack );
VIDEO_UPDATE( bombjack );
