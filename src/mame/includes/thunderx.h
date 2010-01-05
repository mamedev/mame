/*************************************************************************

    Super Contra / Thunder Cross

*************************************************************************/

typedef struct _thunderx_state thunderx_state;
struct _thunderx_state
{
	/* memory pointers */
	UINT8 *    ram;
	UINT8 *    pmcram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        priority;
	UINT8      _1f98_data;
	int        palette_selected;
	int        rambank, pmcbank;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k007232;
	const device_config *k052109;
	const device_config *k051960;
};


/*----------- defined in video/thunderx.c -----------*/

extern void thunderx_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void thunderx_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( scontra );
VIDEO_UPDATE( scontra );
