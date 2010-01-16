/*************************************************************************

    GI Joe

*************************************************************************/

typedef struct _gijoe_state gijoe_state;
struct _gijoe_state
{
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
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k054539;
	const device_config *k056832;
	const device_config *k053246;
	const device_config *k053251;
};

/*----------- defined in video/gijoe.c -----------*/

extern void gijoe_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void gijoe_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START( gijoe );
VIDEO_UPDATE( gijoe );
