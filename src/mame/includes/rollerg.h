/*************************************************************************

    Rollergames

*************************************************************************/

typedef struct _rollerg_state rollerg_state;
struct _rollerg_state
{
	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        sprite_colorbase, zoom_colorbase;

	/* misc */
	int        readzoomroms;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k053260;
	running_device *k053244;
	running_device *k051316;
};

/*----------- defined in video/rollerg.c -----------*/

extern void rollerg_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);
extern void rollerg_zoom_callback(running_machine *machine, int *code,int *color,int *flags);

VIDEO_START( rollerg );
VIDEO_UPDATE( rollerg );
