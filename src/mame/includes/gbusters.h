/*************************************************************************

    Gangbusters

*************************************************************************/

typedef struct _gbusters_state gbusters_state;
struct _gbusters_state
{
	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        palette_selected;
	int        priority;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k007232;
	const device_config *k052109;
	const device_config *k051960;
};

/*----------- defined in video/gbusters.c -----------*/

extern void gbusters_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags, int *priority);
extern void gbusters_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( gbusters );
VIDEO_UPDATE( gbusters );
