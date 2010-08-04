/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

class battlnts_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, battlnts_state(machine)); }

	battlnts_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
//  UINT8 *      paletteram;    // this currently uses generic palette handling

	/* video-related */
	int spritebank;
	int layer_colorbase[2];


	/* devices */
	running_device *audiocpu;
	running_device *k007342;
	running_device *k007420;
};

/*----------- defined in video/battlnts.c -----------*/

WRITE8_HANDLER( battlnts_spritebank_w );

VIDEO_UPDATE( battlnts );

void battlnts_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags);
void battlnts_sprite_callback(running_machine *machine, int *code, int *color);
