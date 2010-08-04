/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blmbycar_state(machine)); }

	blmbycar_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *    vram_0;
	UINT16 *    scroll_0;
	UINT16 *    vram_1;
	UINT16 *    scroll_1;
	UINT16 *    spriteram;
	UINT16 *    paletteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1;

	/* input-related */
	UINT8       pot_wheel;	// blmbycar
	int         old_val;	// blmbycar
	int         retvalue;	// waterball
};


/*----------- defined in video/blmbycar.c -----------*/

WRITE16_HANDLER( blmbycar_palette_w );

WRITE16_HANDLER( blmbycar_vram_0_w );
WRITE16_HANDLER( blmbycar_vram_1_w );

VIDEO_START( blmbycar );
VIDEO_UPDATE( blmbycar );
