/*************************************************************************

    Ultraman

*************************************************************************/

class ultraman_state : public driver_device
{
public:
	ultraman_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT16 *   paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        sprite_colorbase, zoom_colorbase[3];
	int        bank0, bank1, bank2;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k051316_1;
	device_t *k051316_2;
	device_t *k051316_3;
	device_t *k051960;
};



/*----------- defined in video/ultraman.c -----------*/

extern void ultraman_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void ultraman_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void ultraman_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);
extern void ultraman_zoom_callback_2(running_machine *machine, int *code,int *color,int *flags);

WRITE16_HANDLER( ultraman_gfxctrl_w );

VIDEO_START( ultraman );
VIDEO_UPDATE( ultraman );
