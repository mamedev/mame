/*************************************************************************

    Jack the Giant Killer

*************************************************************************/

typedef struct _jack_state jack_state;
struct _jack_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bg_tilemap;

	/* misc */
	int timer_rate;
	int joinem_snd_bit;
	int question_address;
	int question_rom;
	int remap_address[16];


	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/jack.c -----------*/

WRITE8_HANDLER( jack_videoram_w );
WRITE8_HANDLER( jack_colorram_w );
WRITE8_HANDLER( jack_paletteram_w );
READ8_HANDLER( jack_flipscreen_r );
WRITE8_HANDLER( jack_flipscreen_w );

VIDEO_START( jack );
VIDEO_UPDATE( jack );

PALETTE_INIT( joinem );
VIDEO_START( joinem );
VIDEO_UPDATE( joinem );
