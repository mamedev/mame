/*************************************************************************

    Labyrinth Runner

*************************************************************************/

class labyrunr_state : public driver_device
{
public:
	labyrunr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
