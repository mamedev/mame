/*************************************************************************

    Over Drive

*************************************************************************/

typedef struct _overdriv_state overdriv_state;
struct _overdriv_state
{
	/* memory pointers */
//  UINT16 *   paletteram;    // currently this uses generic palette handling

	/* video-related */
	int       zoom_colorbase[2], road_colorbase[2], sprite_colorbase;

	/* misc */
	UINT16     cpuB_ctrl;

	/* devices */
	const device_config *maincpu;
	const device_config *subcpu;
	const device_config *audiocpu;
	const device_config *k053260_1;
	const device_config *k053260_2;
	const device_config *k051316_1;
	const device_config *k051316_2;
	const device_config *k053246;
	const device_config *k053251;
};

/*----------- defined in video/overdriv.c -----------*/

extern void overdriv_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
extern void overdriv_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void overdriv_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);

VIDEO_UPDATE( overdriv );
