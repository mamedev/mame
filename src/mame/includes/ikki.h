/*************************************************************************

    Ikki

*************************************************************************/

class ikki_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ikki_state(machine)); }

	ikki_state(running_machine &machine)
		: driver_data_t(machine) { }

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
