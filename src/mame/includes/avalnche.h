/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
WRITE8_DEVICE_HANDLER( avalnche_noise_amplitude_w );
WRITE8_DEVICE_HANDLER( avalnche_attract_enable_w );
WRITE8_DEVICE_HANDLER( avalnche_audio_w );
