/*************************************************************************

    Blades of Steel

*************************************************************************/

typedef struct _bladestl_state bladestl_state;
struct _bladestl_state
{
	/* memory pointers */
	UINT8 *    paletteram;

	/* video-related */
	int        spritebank;
	int        layer_colorbase[2];

	/* misc */
	int        last_track[4];

	/* devices */
	const device_config *audiocpu;
	const device_config *k007342;
	const device_config *k007420;
};



/*----------- defined in video/bladestl.c -----------*/

PALETTE_INIT( bladestl );

VIDEO_UPDATE( bladestl );

void bladestl_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags);
void bladestl_sprite_callback(running_machine *machine, int *code, int *color);
