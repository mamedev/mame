/*************************************************************************

    Over Drive

*************************************************************************/

class overdriv_state : public driver_device
{
public:
	overdriv_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT16 *   paletteram;    // currently this uses generic palette handling

	/* video-related */
	int       zoom_colorbase[2], road_colorbase[2], sprite_colorbase;

	/* misc */
	UINT16     cpuB_ctrl;

	/* devices */
	device_t *maincpu;
	device_t *subcpu;
	device_t *audiocpu;
	device_t *k053260_1;
	device_t *k053260_2;
	device_t *k051316_1;
	device_t *k051316_2;
	device_t *k053246;
	device_t *k053251;
};

/*----------- defined in video/overdriv.c -----------*/

extern void overdriv_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
extern void overdriv_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void overdriv_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);

VIDEO_UPDATE( overdriv );
