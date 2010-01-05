/*************************************************************************

    Parodius

*************************************************************************/

typedef struct _parodius_state parodius_state;
struct _parodius_state
{
	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];

	/* misc */
	int        videobank;
	//int        nmi_enabled;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k053260;
	const device_config *k052109;
	const device_config *k053245;
	const device_config *k053251;
};

/*----------- defined in video/parodius.c -----------*/

extern void parodius_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void parodius_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

VIDEO_UPDATE( parodius );
