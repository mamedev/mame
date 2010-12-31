/*************************************************************************

    Himeshikibu

*************************************************************************/

class himesiki_state : public driver_device
{
public:
	himesiki_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    bg_ram;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *bg_tilemap;
	int 	     scrollx[2];
	int        flipscreen;

	/* devices */
	device_t *subcpu;
};


/*----------- defined in video/himesiki.c -----------*/

VIDEO_START( himesiki );
VIDEO_UPDATE( himesiki );

WRITE8_HANDLER( himesiki_bg_ram_w );
WRITE8_HANDLER( himesiki_scrollx_w );
WRITE8_HANDLER( himesiki_flip_w );
