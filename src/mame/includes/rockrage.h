/*************************************************************************

    Rock'n Rage

*************************************************************************/

typedef struct _rockrage_state rockrage_state;
struct _rockrage_state
{
	/* memory pointers */
	UINT8 *    paletteram;

	/* video-related */
	int        layer_colorbase[2];
	int        vreg;

	/* devices */
	const device_config *audiocpu;
	const device_config *k007342;
	const device_config *k007420;
};


/*----------- defined in video/rockrage.c -----------*/

WRITE8_HANDLER( rockrage_vreg_w );

VIDEO_UPDATE( rockrage );
PALETTE_INIT( rockrage );

void rockrage_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags);
void rockrage_sprite_callback(running_machine *machine, int *code, int *color);
