// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


class avalnche_state : public driver_device
{
public:
	avalnche_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<UINT8> m_videoram;
	optional_device<discrete_device> m_discrete;

	UINT8 m_avalance_video_inverted;

	DECLARE_WRITE8_MEMBER(avalance_video_invert_w);
	DECLARE_WRITE8_MEMBER(catch_coin_counter_w);
	DECLARE_WRITE8_MEMBER(avalance_credit_1_lamp_w);
	DECLARE_WRITE8_MEMBER(avalance_credit_2_lamp_w);
	DECLARE_WRITE8_MEMBER(avalance_start_lamp_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_avalnche(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(avalnche_noise_amplitude_w);
	DECLARE_WRITE8_MEMBER(avalnche_attract_enable_w);
	DECLARE_WRITE8_MEMBER(avalnche_audio_w);
	DECLARE_WRITE8_MEMBER(catch_audio_w);
	required_device<cpu_device> m_maincpu;
};


/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
