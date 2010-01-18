/*************************************************************************

    Lethal Enforcers

*************************************************************************/

typedef struct _lethal_state lethal_state;
struct _lethal_state
{
	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[4], sprite_colorbase;

	/* misc */
	UINT8      cur_control2;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k054539;
	running_device *k056832;
	running_device *k053244;
	running_device *k054000;
};

/*----------- defined in video/lethal.c -----------*/

extern void lethalen_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void lethalen_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

WRITE8_HANDLER(lethalen_palette_control);

VIDEO_START(lethalen);
VIDEO_UPDATE(lethalen);
