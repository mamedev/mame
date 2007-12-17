#include "sound/discrete.h"

/*----------- defined in video/blockade.c -----------*/

WRITE8_HANDLER( blockade_videoram_w );

VIDEO_START( blockade );
VIDEO_UPDATE( blockade );

/*----------- defined in audio/blockade.c -----------*/

extern struct Samplesinterface blockade_samples_interface;
DISCRETE_SOUND_EXTERN( blockade );

WRITE8_HANDLER( blockade_sound_freq_w );
WRITE8_HANDLER( blockade_env_on_w );
WRITE8_HANDLER( blockade_env_off_w );
