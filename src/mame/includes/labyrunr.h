/*************************************************************************

    Labyrinth Runner

*************************************************************************/

class labyrunr_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, labyrunr_state(machine)); }

	labyrunr_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram1;
	UINT8 *    videoram2;
	UINT8 *    scrollram;
	UINT8 *    spriteram;
	UINT8 *    paletteram;

	/* video-related */
	tilemap_t    *layer0, *layer1;
	rectangle  clip0, clip1;

	/* devices */
	running_device *k007121;
};


/*----------- defined in video/labyrunr.c -----------*/


WRITE8_HANDLER( labyrunr_vram1_w );
WRITE8_HANDLER( labyrunr_vram2_w );

PALETTE_INIT( labyrunr );
VIDEO_START( labyrunr );
VIDEO_UPDATE( labyrunr );
