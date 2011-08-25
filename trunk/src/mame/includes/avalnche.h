/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


class avalnche_state : public driver_device
{
public:
	avalnche_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8 *  m_videoram;
	size_t   m_videoram_size;

	UINT8    m_avalance_video_inverted;
};

/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
WRITE8_DEVICE_HANDLER( avalnche_noise_amplitude_w );
WRITE8_DEVICE_HANDLER( avalnche_attract_enable_w );
WRITE8_DEVICE_HANDLER( avalnche_audio_w );

WRITE8_HANDLER( catch_audio_w );
