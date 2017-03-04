// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"

/* Avalanche Discrete Sound Input Nodes */
#define AVALNCHE_AUD0_EN            NODE_01
#define AVALNCHE_AUD1_EN            NODE_02
#define AVALNCHE_AUD2_EN            NODE_03
#define AVALNCHE_SOUNDLVL_DATA      NODE_04
#define AVALNCHE_ATTRACT_EN         NODE_05


class avalnche_state : public driver_device
{
public:
	avalnche_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<uint8_t> m_videoram;
	optional_device<discrete_device> m_discrete;

	uint8_t m_avalance_video_inverted;

	DECLARE_WRITE_LINE_MEMBER(video_invert_w);
	DECLARE_WRITE8_MEMBER(catch_coin_counter_w);
	DECLARE_WRITE_LINE_MEMBER(credit_1_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(credit_2_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(start_lamp_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_avalnche(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(avalnche_noise_amplitude_w);
	DECLARE_WRITE_LINE_MEMBER(catch_aud0_w);
	DECLARE_WRITE_LINE_MEMBER(catch_aud1_w);
	DECLARE_WRITE_LINE_MEMBER(catch_aud2_w);
	required_device<cpu_device> m_maincpu;
};


/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
