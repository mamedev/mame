/*************************************************************************

    S.P.Y.

*************************************************************************/

class spy_state : public driver_device
{
public:
	spy_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    ram;
	UINT8 *    pmcram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        rambank, pmcbank;
	int        video_enable;
	int        old_3f90;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k007232_1;
	device_t *k007232_2;
	device_t *k052109;
	device_t *k051960;
};


/*----------- defined in video/spy.c -----------*/

extern void spy_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void spy_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( spy );
VIDEO_UPDATE( spy );
