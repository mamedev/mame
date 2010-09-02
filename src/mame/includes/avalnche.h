/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


class avalnche_state : public driver_device
{
public:
	avalnche_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* video-related */
	UINT8 *  videoram;
	size_t   videoram_size;

	UINT8    avalance_video_inverted;
};

/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
WRITE8_DEVICE_HANDLER( avalnche_noise_amplitude_w );
WRITE8_DEVICE_HANDLER( avalnche_attract_enable_w );
WRITE8_DEVICE_HANDLER( avalnche_audio_w );
