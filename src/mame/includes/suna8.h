#include "sound/samples.h"

#define TILEMAPS 0

class suna8_state : public driver_device
{
public:
	suna8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 rombank;
	UINT8 spritebank;
	UINT8 palettebank;
	UINT8 unknown;

	UINT8 protection_val;
	UINT8 nmi_enable;
	UINT8 spritebank_latch;
	UINT8 trash_prot;

	UINT8 *hardhead_ip;
	UINT8 *wram;
	UINT8 *spriteram;

	int text_dim; /* specifies format of text layer */

#if TILEMAPS
	tilemap_t *bg_tilemap;
	int tiles;
	int trombank;
	int page;
#endif

};


/*----------- defined in audio/suna8.c -----------*/

WRITE8_DEVICE_HANDLER( suna8_play_samples_w );
WRITE8_DEVICE_HANDLER( rranger_play_samples_w );
WRITE8_DEVICE_HANDLER( suna8_samples_number_w );
SAMPLES_START( suna8_sh_start );


/*----------- defined in video/suna8.c -----------*/

WRITE8_HANDLER( suna8_spriteram_w );			// for debug
WRITE8_HANDLER( suna8_banked_spriteram_w );	// for debug

READ8_HANDLER( suna8_banked_paletteram_r );
READ8_HANDLER( suna8_banked_spriteram_r );

WRITE8_HANDLER( brickzn_banked_paletteram_w );

VIDEO_START( suna8_textdim0 );
VIDEO_START( suna8_textdim8 );
VIDEO_START( suna8_textdim12 );
VIDEO_UPDATE( suna8 );
