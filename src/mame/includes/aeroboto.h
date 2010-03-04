/***************************************************************************

    Aeroboto

***************************************************************************/

class aeroboto_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, aeroboto_state(machine)); }

	aeroboto_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 * mainram;
	UINT8 * spriteram;
	UINT8 * videoram;
	UINT8 * hscroll;
	UINT8 * vscroll;
	UINT8 * tilecolor;
	UINT8 * starx;
	UINT8 * stary;
	UINT8 * bgcolor;
	size_t  spriteram_size;

	/* stars layout */
	UINT8 * stars_rom;
	int     stars_length;

	/* video-related */
	tilemap_t *bg_tilemap;
	int     charbank, starsoff;
	int     sx, sy;
	UINT8   ox, oy;

	/* misc */
	int count;
	int disable_irq;
};


/*----------- defined in video/aeroboto.c -----------*/

VIDEO_START( aeroboto );
VIDEO_UPDATE( aeroboto );

READ8_HANDLER( aeroboto_in0_r );
WRITE8_HANDLER( aeroboto_3000_w );
WRITE8_HANDLER( aeroboto_videoram_w );
WRITE8_HANDLER( aeroboto_tilecolor_w );
