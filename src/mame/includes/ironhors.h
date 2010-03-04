/*************************************************************************

    IronHorse

*************************************************************************/

class ironhors_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ironhors_state(machine)); }

	ironhors_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	UINT8 *    spriteram2;
	UINT8 *    scroll;
	UINT8 *    interrupt_enable;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        palettebank, charbank, spriterambank;

	/* devices */
	running_device *soundcpu;
};


/*----------- defined in video/ironhors.c -----------*/

WRITE8_HANDLER( ironhors_videoram_w );
WRITE8_HANDLER( ironhors_colorram_w );
WRITE8_HANDLER( ironhors_palettebank_w );
WRITE8_HANDLER( ironhors_charbank_w );
WRITE8_HANDLER( ironhors_flipscreen_w );

PALETTE_INIT( ironhors );
VIDEO_START( ironhors );
VIDEO_UPDATE( ironhors );
VIDEO_START( farwest );
VIDEO_UPDATE( farwest );
