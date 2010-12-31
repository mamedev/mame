/*************************************************************************

    GI Joe

*************************************************************************/

class gijoe_state : public driver_device
{
public:
	gijoe_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    workram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         avac_bits[4], avac_occupancy[4];
	int         layer_colorbase[4], layer_pri[4];
	int         avac_vrc, sprite_colorbase;

	/* misc */
	UINT16  	cur_control2;
	emu_timer	*dmadelay_timer;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k054539;
	device_t *k056832;
	device_t *k053246;
	device_t *k053251;
};

/*----------- defined in video/gijoe.c -----------*/

extern void gijoe_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void gijoe_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START( gijoe );
VIDEO_UPDATE( gijoe );
