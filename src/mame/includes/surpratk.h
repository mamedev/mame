/*************************************************************************

    Surprise Attack

*************************************************************************/

class surpratk_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, surpratk_state(machine)); }

	surpratk_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];

	/* misc */
	int        videobank;

	/* devices */
	running_device *maincpu;
	running_device *k052109;
	running_device *k053244;
	running_device *k053251;
};

/*----------- defined in video/surpratk.c -----------*/

extern void surpratk_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void surpratk_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

VIDEO_UPDATE( surpratk );
