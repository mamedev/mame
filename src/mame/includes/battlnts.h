/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

typedef struct _battlnts_state battlnts_state;
struct _battlnts_state
{
	/* memory pointers */
//  UINT8 *      paletteram;    // this currently uses generic palette handling

	/* video-related */
	int spritebank;
	int layer_colorbase[2];


	/* devices */
	const device_config *audiocpu;
	const device_config *k007342;
	const device_config *k007420;
};

/*----------- defined in video/battlnts.c -----------*/

WRITE8_HANDLER( battlnts_spritebank_w );

VIDEO_UPDATE( battlnts );

void battlnts_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags);
void battlnts_sprite_callback(running_machine *machine, int *code, int *color);
