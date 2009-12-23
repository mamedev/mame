/*************************************************************************

    88 Games

*************************************************************************/

typedef struct __88games_state _88games_state;
struct __88games_state
{
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
	const device_config *audiocpu;
	const device_config *k052109;
	const device_config *k051960;
	const device_config *k051316;
	const device_config *upd_1;
	const device_config *upd_2;
};


/*----------- defined in video/88games.c -----------*/

void _88games_sprite_callback(running_machine *machine, int *code, int *color, int *priority, int *shadow);
void _88games_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
void _88games_zoom_callback(running_machine *machine, int *code, int *color, int *flags);

VIDEO_UPDATE( 88games );
