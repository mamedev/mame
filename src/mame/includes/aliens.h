/*************************************************************************

    Aliens

*************************************************************************/

typedef struct _aliens_state aliens_state;
struct _aliens_state
{
	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        palette_selected;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k007232;
	const device_config *k052109;
	const device_config *k051960;
};

/*----------- defined in video/aliens.c -----------*/

extern void aliens_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color, int *flags, int *priority);
extern void aliens_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( aliens );
VIDEO_UPDATE( aliens );
