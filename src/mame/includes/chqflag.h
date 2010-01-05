/*************************************************************************

    Chequered Flag

*************************************************************************/

typedef struct _chqflag_state chqflag_state;
struct _chqflag_state
{
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
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k007232_1;
	const device_config *k007232_2;
	const device_config *k051960;
	const device_config *k051316_1;
	const device_config *k051316_2;
};

/*----------- defined in video/chqflag.c -----------*/

extern void chqflag_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void chqflag_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
extern void chqflag_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);

VIDEO_START( chqflag );
VIDEO_UPDATE( chqflag );
