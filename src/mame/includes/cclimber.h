/*----------- defined in machine/cclimber.c -----------*/

#include "sound/samples.h"
#include "sound/ay8910.h"


DRIVER_INIT( cclimber );
DRIVER_INIT( cclimberj );
void cclimberj_decode(running_machine *machine);
void mshuttle_decode(running_machine *machine);
DRIVER_INIT( cannonb );
DRIVER_INIT( cannonb2 );
DRIVER_INIT( ckongb );

/*----------- defined in video/cclimber.c -----------*/

extern UINT8 *cclimber_videoram;
extern UINT8 *cclimber_colorram;
extern UINT8 *cclimber_spriteram;
extern UINT8 *cclimber_bigsprite_videoram;
extern UINT8 *cclimber_bigsprite_control;

extern UINT8 *cclimber_column_scroll;
extern UINT8 *cclimber_flip_screen;

extern UINT8 *swimmer_background_color;
extern UINT8 *swimmer_side_background_enabled;
extern UINT8 *swimmer_palettebank;

extern UINT8 *toprollr_bg_videoram;
extern UINT8 *toprollr_bg_coloram;

WRITE8_HANDLER( cclimber_colorram_w );
WRITE8_HANDLER( cannonb_flip_screen_w );

PALETTE_INIT( cclimber );
VIDEO_START( cclimber );
VIDEO_UPDATE( cclimber );

PALETTE_INIT( swimmer );
VIDEO_START( swimmer );
VIDEO_UPDATE( swimmer );

PALETTE_INIT( yamato );
VIDEO_UPDATE( yamato );

PALETTE_INIT( toprollr );
VIDEO_START( toprollr );
VIDEO_UPDATE( toprollr );


/*----------- defined in audio/cclimber.c -----------*/

extern const ay8910_interface cclimber_ay8910_interface;
extern const samples_interface cclimber_samples_interface;
WRITE8_HANDLER( cclimber_sample_trigger_w );
WRITE8_HANDLER( cclimber_sample_rate_w );
WRITE8_HANDLER( cclimber_sample_volume_w );
