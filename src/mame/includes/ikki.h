/*************************************************************************

    Ikki

*************************************************************************/

class ikki_state : public driver_device
{
public:
	ikki_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    scroll;
	size_t     videoram_size;
	size_t     spriteram_size;

	/* video-related */
	bitmap_t   *sprite_bitmap;
	UINT8      flipscreen;
	int        punch_through_pen;
};


/*----------- defined in video/ikki.c -----------*/

WRITE8_HANDLER( ikki_scrn_ctrl_w );

PALETTE_INIT( ikki );
VIDEO_START( ikki );
VIDEO_UPDATE( ikki );
