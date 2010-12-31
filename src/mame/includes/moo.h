/*************************************************************************

    Wild West C.O.W.boys of Moo Mesa / Bucky O'Hare

*************************************************************************/

class moo_state : public driver_device
{
public:
	moo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    workram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         sprite_colorbase;
	int         layer_colorbase[4], layerpri[3];
	int         alpha_enabled;

	/* misc */
	int         game_type;
	UINT16      protram[16];
	UINT16      cur_control2;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k054539;
	device_t *k053246;
	device_t *k053251;
	device_t *k056832;
	device_t *k054338;

    emu_timer *dmaend_timer;
};



/*----------- defined in video/moo.c -----------*/

extern void moo_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
extern void moo_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);

VIDEO_START(moo);
VIDEO_UPDATE(moo);
