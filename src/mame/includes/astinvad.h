#include "sound/samples.h"

/*----------- defined in video/astinvad.c -----------*/

VIDEO_UPDATE( astinvad );
VIDEO_UPDATE( spcking2 );
VIDEO_UPDATE( spaceint );

VIDEO_START( spaceint );

WRITE8_HANDLER( spaceint_videoram_w );
WRITE8_HANDLER( spaceint_color_w);

void astinvad_set_screen_red(int data);


/*----------- defined in audio/astinvad.c -----------*/

extern struct Samplesinterface astinvad_samples_interface;

WRITE8_HANDLER( astinvad_sound1_w );
WRITE8_HANDLER( astinvad_sound2_w );
WRITE8_HANDLER( spaceint_sound1_w );
WRITE8_HANDLER( spaceint_sound2_w );

