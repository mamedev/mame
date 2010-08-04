/*************************************************************************

    The Main Event / Devastators

*************************************************************************/

class mainevt_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mainevt_state(machine)); }

	mainevt_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        nmi_enable;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *upd;
	running_device *k007232;
	running_device *k052109;
	running_device *k051960;
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
