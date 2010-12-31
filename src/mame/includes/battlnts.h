/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

class battlnts_state : public driver_device
{
public:
	battlnts_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT8 *      paletteram;    // this currently uses generic palette handling

	/* video-related */
	int spritebank;
	int layer_colorbase[2];


	/* devices */
	device_t *audiocpu;
	device_t *k007342;
	device_t *k007420;
};

/*----------- defined in video/battlnts.c -----------*/

WRITE8_HANDLER( battlnts_spritebank_w );

VIDEO_UPDATE( battlnts );

void battlnts_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags);
void battlnts_sprite_callback(running_machine *machine, int *code, int *color);
