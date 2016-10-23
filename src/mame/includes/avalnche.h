// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "sound/discrete.h"


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

	void avalance_video_invert_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void catch_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avalance_credit_1_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avalance_credit_2_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avalance_start_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_avalnche(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void avalnche_noise_amplitude_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avalnche_attract_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void avalnche_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void catch_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	required_device<cpu_device> m_maincpu;
};


/*----------- defined in audio/avalnche.c -----------*/

DISCRETE_SOUND_EXTERN( avalnche );
