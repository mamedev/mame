/*************************************************************************

    Atari Asteroids hardware

*************************************************************************/

#include "sound/discrete.h"
#include "video/avgdvg.h"

class asteroid_state : public driver_device
{
public:
	asteroid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram1(*this, "ram1"),
		m_ram2(*this, "ram2"),
		m_discrete(*this, "discrete") ,
		m_maincpu(*this, "maincpu"),
		m_dvg(*this, "dvg") { }

	optional_shared_ptr<UINT8> m_ram1;
	optional_shared_ptr<UINT8> m_ram2;
	required_device<discrete_device> m_discrete;
	DECLARE_WRITE8_MEMBER(astdelux_coin_counter_w);
	DECLARE_WRITE8_MEMBER(llander_led_w);
	DECLARE_READ8_MEMBER(asteroid_IN0_r);
	DECLARE_READ8_MEMBER(asterock_IN0_r);
	DECLARE_READ8_MEMBER(asteroid_IN1_r);
	DECLARE_READ8_MEMBER(asteroid_DSW1_r);
	DECLARE_WRITE8_MEMBER(asteroid_bank_switch_w);
	DECLARE_WRITE8_MEMBER(astdelux_bank_switch_w);
	DECLARE_WRITE8_MEMBER(astdelux_led_w);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	DECLARE_DRIVER_INIT(asterock);
	DECLARE_DRIVER_INIT(asteroidb);
	virtual void machine_reset();
	INTERRUPT_GEN_MEMBER(asteroid_interrupt);
	INTERRUPT_GEN_MEMBER(asterock_interrupt);
	INTERRUPT_GEN_MEMBER(llander_interrupt);
	DECLARE_WRITE8_MEMBER(asteroid_explode_w);
	DECLARE_WRITE8_MEMBER(asteroid_thump_w);
	DECLARE_WRITE8_MEMBER(asteroid_sounds_w);
	DECLARE_WRITE8_MEMBER(astdelux_sounds_w);
	DECLARE_WRITE8_MEMBER(asteroid_noise_reset_w);
	DECLARE_WRITE8_MEMBER(llander_snd_reset_w);
	DECLARE_WRITE8_MEMBER(llander_sounds_w);
	DECLARE_MACHINE_RESET(llander);
	required_device<cpu_device> m_maincpu;
	required_device<dvg_device> m_dvg;
};

/*----------- defined in audio/asteroid.c -----------*/

DISCRETE_SOUND_EXTERN( asteroid );
DISCRETE_SOUND_EXTERN( astdelux );

/*----------- defined in audio/llander.c -----------*/

DISCRETE_SOUND_EXTERN( llander );
