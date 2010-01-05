/*************************************************************************

    Crime Fighters

*************************************************************************/

typedef struct _crimfght_state crimfght_state;
struct _crimfght_state
{
	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k007232;
	const device_config *k052109;
	const device_config *k051960;
};

/*----------- defined in video/crimfght.c -----------*/

extern void crimfght_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void crimfght_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( crimfght );
VIDEO_UPDATE( crimfght );
