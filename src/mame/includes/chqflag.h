/*************************************************************************

    Chequered Flag

*************************************************************************/

class chqflag_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, chqflag_state(machine)); }

	chqflag_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k007232_1;
	running_device *k007232_2;
	running_device *k051960;
	running_device *k051316_1;
	running_device *k051316_2;
};

/*----------- defined in video/chqflag.c -----------*/

extern void chqflag_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void chqflag_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void chqflag_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);

VIDEO_START( chqflag );
VIDEO_UPDATE( chqflag );
