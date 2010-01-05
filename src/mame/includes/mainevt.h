/*************************************************************************

    The Main Event / Devastators

*************************************************************************/

typedef struct _mainevt_state mainevt_state;
struct _mainevt_state
{
	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        nmi_enable;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *upd;
	const device_config *k007232;
	const device_config *k052109;
	const device_config *k051960;
};

/*----------- defined in video/mainevt.c -----------*/

extern void mainevt_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void dv_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void mainevt_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);
extern void dv_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( mainevt );
VIDEO_START( dv );

VIDEO_UPDATE( mainevt );
VIDEO_UPDATE( dv );
