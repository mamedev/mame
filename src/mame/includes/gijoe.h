/*************************************************************************

    GI Joe

*************************************************************************/

class gijoe_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gijoe_state(machine)); }

	gijoe_state(running_machine &machine) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k054539;
	running_device *k056832;
	running_device *k053246;
	running_device *k053251;
};

/*----------- defined in video/gijoe.c -----------*/

extern void gijoe_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void gijoe_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START( gijoe );
VIDEO_UPDATE( gijoe );
