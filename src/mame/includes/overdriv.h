/*************************************************************************

    Over Drive

*************************************************************************/

class overdriv_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, overdriv_state(machine)); }

	overdriv_state(running_machine &machine) { }

	/* memory pointers */
//  UINT16 *   paletteram;    // currently this uses generic palette handling

	/* video-related */
	int       zoom_colorbase[2], road_colorbase[2], sprite_colorbase;

	/* misc */
	UINT16     cpuB_ctrl;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
	running_device *audiocpu;
	running_device *k053260_1;
	running_device *k053260_2;
	running_device *k051316_1;
	running_device *k051316_2;
	running_device *k053246;
	running_device *k053251;
};

/*----------- defined in video/overdriv.c -----------*/

extern void overdriv_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
extern void overdriv_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void overdriv_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);

VIDEO_UPDATE( overdriv );
