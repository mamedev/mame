/*************************************************************************

    Vendetta

*************************************************************************/

typedef struct _vendetta_state vendetta_state;
struct _vendetta_state
{
	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];

	/* misc */
	int        irq_enabled;
	offs_t     video_banking_base;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k053260;
	const device_config *k052109;
	const device_config *k053246;
	const device_config *k053251;
	const device_config *k054000;
};

/*----------- defined in video/vendetta.c -----------*/

extern void vendetta_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void esckids_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void vendetta_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

VIDEO_UPDATE( vendetta );
