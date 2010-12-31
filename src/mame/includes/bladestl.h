/*************************************************************************

    Blades of Steel

*************************************************************************/

class bladestl_state : public driver_device
{
public:
	bladestl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    paletteram;

	/* video-related */
	int        spritebank;
	int        layer_colorbase[2];

	/* misc */
	int        last_track[4];

	/* devices */
	device_t *audiocpu;
	device_t *k007342;
	device_t *k007420;
};



/*----------- defined in video/bladestl.c -----------*/

PALETTE_INIT( bladestl );

VIDEO_UPDATE( bladestl );

void bladestl_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags);
void bladestl_sprite_callback(running_machine *machine, int *code, int *color);
