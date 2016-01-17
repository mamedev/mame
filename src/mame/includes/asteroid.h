// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/*************************************************************************

    Atari Asteroids hardware

*************************************************************************/

#include "sound/discrete.h"
#include "video/avgdvg.h"

class asteroid_state : public driver_device
{
public:
	asteroid_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dvg(*this, "dvg"),
		m_discrete(*this, "discrete"),
		m_ram1(*this, "ram1"),
		m_ram2(*this, "ram2") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<dvg_device> m_dvg;
	required_device<discrete_device> m_discrete;

	/* memory banks */
	optional_memory_bank m_ram1;
	optional_memory_bank m_ram2;

	DECLARE_WRITE8_MEMBER(astdelux_coin_counter_w);
	DECLARE_WRITE8_MEMBER(llander_led_w);
	DECLARE_READ8_MEMBER(asteroid_IN0_r);
	DECLARE_READ8_MEMBER(asterock_IN0_r);
	DECLARE_READ8_MEMBER(asteroid_IN1_r);
	DECLARE_READ8_MEMBER(asteroid_DSW1_r);
	DECLARE_WRITE8_MEMBER(asteroid_bank_switch_w);
	DECLARE_WRITE8_MEMBER(astdelux_bank_switch_w);
	DECLARE_WRITE8_MEMBER(astdelux_led_w);
	DECLARE_WRITE8_MEMBER(asteroid_explode_w);
	DECLARE_WRITE8_MEMBER(asteroid_thump_w);
	DECLARE_WRITE8_MEMBER(asteroid_sounds_w);
	DECLARE_WRITE8_MEMBER(astdelux_sounds_w);
	DECLARE_WRITE8_MEMBER(asteroid_noise_reset_w);
	DECLARE_WRITE8_MEMBER(llander_snd_reset_w);
	DECLARE_WRITE8_MEMBER(llander_sounds_w);

	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);

	INTERRUPT_GEN_MEMBER(asteroid_interrupt);
	INTERRUPT_GEN_MEMBER(asterock_interrupt);
	INTERRUPT_GEN_MEMBER(llander_interrupt);

	DECLARE_DRIVER_INIT(asterock);
	DECLARE_DRIVER_INIT(asteroidb);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

/*----------- defined in audio/asteroid.c -----------*/

DISCRETE_SOUND_EXTERN( asteroid );
DISCRETE_SOUND_EXTERN( astdelux );

/*----------- defined in audio/llander.c -----------*/

DISCRETE_SOUND_EXTERN( llander );
