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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k054539;
	running_device *filter1l;
	running_device *filter1r;
	running_device *filter2l;
	running_device *filter2r;
	running_device *k056832;
	running_device *k053246;
	running_device *k053250;
	running_device *k053251;
	running_device *k053252;
	running_device *k054338;
};


/*----------- defined in video/xexex.c -----------*/

extern void xexex_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void xexex_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START( xexex );
VIDEO_UPDATE( xexex );
