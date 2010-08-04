/*************************************************************************

    Mikie

*************************************************************************/

class mikie_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mikie_state(machine)); }

	mikie_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int        palettebank;

	/* misc */
	int        last_irq;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
};


/*----------- defined in video/mikie.c -----------*/

WRITE8_HANDLER( mikie_videoram_w );
WRITE8_HANDLER( mikie_colorram_w );
WRITE8_HANDLER( mikie_palettebank_w );
WRITE8_HANDLER( mikie_flipscreen_w );

PALETTE_INIT( mikie );
VIDEO_START( mikie );
VIDEO_UPDATE( mikie );
