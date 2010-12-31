/*************************************************************************

    Xexex

*************************************************************************/

class xexex_state : public driver_device
{
public:
	xexex_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    workram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[4], sprite_colorbase;
	int        layerpri[4];
	int        cur_alpha;

	/* misc */
	UINT16     cur_control2;
	INT32      cur_sound_region, strip_0x1a;
	int        suspension_active, resume_trigger;
	emu_timer  *dmadelay_timer;
	int        frame;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k054539;
	device_t *filter1l;
	device_t *filter1r;
	device_t *filter2l;
	device_t *filter2r;
	device_t *k056832;
	device_t *k053246;
	device_t *k053250;
	device_t *k053251;
	device_t *k053252;
	device_t *k054338;
};


/*----------- defined in video/xexex.c -----------*/

extern void xexex_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void xexex_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START( xexex );
VIDEO_UPDATE( xexex );
