/*************************************************************************

    Bottom of the Ninth

*************************************************************************/

class bottom9_state : public driver_device
{
public:
	bottom9_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase, zoom_colorbase;

	/* misc */
	int        video_enable;
	int        zoomreadroms, k052109_selected;
	int        nmienable;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k007232_1;
	device_t *k007232_2;
	device_t *k052109;
	device_t *k051960;
	device_t *k051316;
};

/*----------- defined in video/bottom9.c -----------*/

extern void bottom9_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void bottom9_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);
extern void bottom9_zoom_callback(running_machine *machine, int *code,int *color,int *flags);

VIDEO_START( bottom9 );
VIDEO_UPDATE( bottom9 );
