/*************************************************************************

    88 Games

*************************************************************************/

class _88games_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, _88games_state(machine)); }

	_88games_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *      ram;
	UINT8 *      banked_rom;
//  UINT8 *      paletteram_1000;   // this currently uses generic palette handling
//  UINT8 *      nvram; // this currently uses generic nvram handling

	/* video-related */
	int          k88games_priority;
	int          layer_colorbase[3], sprite_colorbase, zoom_colorbase;
	int          videobank;
	int          zoomreadroms;
	int          speech_chip;

	/* devices */
	running_device *audiocpu;
	running_device *k052109;
	running_device *k051960;
	running_device *k051316;
	running_device *upd_1;
	running_device *upd_2;
};


/*----------- defined in video/88games.c -----------*/

void _88games_sprite_callback(running_machine *machine, int *code, int *color, int *priority, int *shadow);
void _88games_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
void _88games_zoom_callback(running_machine *machine, int *code, int *color, int *flags);

VIDEO_UPDATE( 88games );
