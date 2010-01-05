/*************************************************************************

    Xexex

*************************************************************************/

typedef struct _xexex_state xexex_state;
struct _xexex_state
{
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
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k054539;
	const device_config *filter1l;
	const device_config *filter1r;
	const device_config *filter2l;
	const device_config *filter2r;
	const device_config *k056832;
	const device_config *k053246;
	const device_config *k053250;
	const device_config *k053251;
	const device_config *k053252;
	const device_config *k054338;
};


/*----------- defined in video/xexex.c -----------*/

extern void xexex_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void xexex_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START( xexex );
VIDEO_UPDATE( xexex );
