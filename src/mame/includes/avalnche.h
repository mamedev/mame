/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


class avalnche_state : public driver_device
{
public:
	avalnche_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	/* video-related */
	required_shared_ptr<UINT8> m_videoram;

	UINT8    m_avalance_video_inverted;
	DECLARE_WRITE8_MEMBER(avalance_video_invert_w);
	DECLARE_WRITE8_MEMBER(catch_coin_counter_w);
	DECLARE_WRITE8_MEMBER(avalance_credit_1_lamp_w);
	DECLARE_WRITE8_MEMBER(avalance_credit_2_lamp_w);
	DECLARE_WRITE8_MEMBER(avalance_start_lamp_w);
	virtual void machine_start();
	virtual void machine_reset();
};

/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
DECLARE_WRITE8_DEVICE_HANDLER( avalnche_noise_amplitude_w );
DECLARE_WRITE8_DEVICE_HANDLER( avalnche_attract_enable_w );
DECLARE_WRITE8_DEVICE_HANDLER( avalnche_audio_w );

DECLARE_WRITE8_HANDLER( catch_audio_w );
