/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


typedef struct _avalnche_state avalnche_state;
struct _avalnche_state
{
	/* video-related */
	UINT8 *  videoram;
	UINT8    avalance_video_inverted;
};

/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
WRITE8_DEVICE_HANDLER( avalnche_noise_amplitude_w );
WRITE8_DEVICE_HANDLER( avalnche_attract_enable_w );
WRITE8_DEVICE_HANDLER( avalnche_audio_w );
