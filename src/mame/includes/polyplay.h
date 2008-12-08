#include "sound/samples.h"

/*----------- defined in audio/polyplay.c -----------*/

void polyplay_set_channel1(int active);
void polyplay_set_channel2(int active);
void polyplay_play_channel1(int data);
void polyplay_play_channel2(int data);
SAMPLES_START( polyplay_sh_start );


/*----------- defined in video/polyplay.c -----------*/

extern UINT8 *polyplay_characterram;

PALETTE_INIT( polyplay );
VIDEO_UPDATE( polyplay );
WRITE8_HANDLER( polyplay_characterram_w );
