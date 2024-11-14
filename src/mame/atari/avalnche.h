// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Avalanche hardware

*************************************************************************/

#include "machine/74259.h"
#include "machine/timer.h"
#include "sound/discrete.h"

class avalnche_state : public driver_device
{
public:
	avalnche_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_latch(*this, "latch") { }

	void avalnche_base(machine_config &config);
	void acatch(machine_config &config);
	void acatch_sound(machine_config &config);
	void avalnche(machine_config &config);
	void avalnche_sound(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_videoram;
	optional_device<discrete_device> m_discrete;
	required_device<cpu_device> m_maincpu;
	required_device<f9334_device> m_latch;

	uint8_t m_avalance_video_inverted = 0U;

	void video_invert_w(int state);
	void catch_coin_counter_w(uint8_t data);
	void credit_1_lamp_w(int state);
	void credit_2_lamp_w(int state);
	void start_lamp_w(int state);
	virtual void machine_start() override ATTR_COLD;
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_16v);
	uint32_t screen_update_avalnche(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void avalnche_noise_amplitude_w(uint8_t data);
	void catch_aud0_w(int state);
	void catch_aud1_w(int state);
	void catch_aud2_w(int state);
	void catch_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};
