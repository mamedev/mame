/*************************************************************************

    Chequered Flag

*************************************************************************/

class chqflag_state : public driver_device
{
public:
	chqflag_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        zoom_colorbase[2], sprite_colorbase;

	/* misc */
	int        k051316_readroms;
	int        last_vreg;
	int        analog_ctrl;
	int        accel, wheel;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k007232_1;
	device_t *k007232_2;
	device_t *k051960;
	device_t *k051316_1;
	device_t *k051316_2;
};

/*----------- defined in video/chqflag.c -----------*/

extern void chqflag_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void chqflag_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void chqflag_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);

VIDEO_START( chqflag );
VIDEO_UPDATE( chqflag );
