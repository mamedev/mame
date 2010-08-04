/*************************************************************************

    Block Hole

*************************************************************************/

class blockhl_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blockhl_state(machine)); }

	blockhl_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        palette_selected;
	int        rombank;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k052109;
	running_device *k051960;
};

/*----------- defined in video/blockhl.c -----------*/

extern void blockhl_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color, int *flags, int *priority);
extern void blockhl_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( blockhl );
VIDEO_UPDATE( blockhl );
