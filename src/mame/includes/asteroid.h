/*************************************************************************

    Atari Asteroids hardware

*************************************************************************/

#include "sound/discrete.h"

class asteroid_state : public driver_device
{
public:
	asteroid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_ram1;
	UINT8 *m_ram2;
	DECLARE_WRITE8_MEMBER(astdelux_coin_counter_w);
	DECLARE_WRITE8_MEMBER(llander_led_w);
};


/*----------- defined in machine/asteroid.c -----------*/

INTERRUPT_GEN( asteroid_interrupt );
INTERRUPT_GEN( asterock_interrupt );
INTERRUPT_GEN( llander_interrupt );

READ8_HANDLER( asteroid_IN0_r );
READ8_HANDLER( asterock_IN0_r );
READ8_HANDLER( asteroid_IN1_r );
READ8_HANDLER( asteroid_DSW1_r );

WRITE8_HANDLER( asteroid_bank_switch_w );
WRITE8_HANDLER( astdelux_bank_switch_w );
WRITE8_HANDLER( astdelux_led_w );

MACHINE_RESET( asteroid );


/*----------- defined in audio/asteroid.c -----------*/

DISCRETE_SOUND_EXTERN( asteroid );
DISCRETE_SOUND_EXTERN( astdelux );

WRITE8_DEVICE_HANDLER( asteroid_explode_w );
WRITE8_DEVICE_HANDLER( asteroid_thump_w );
WRITE8_DEVICE_HANDLER( asteroid_sounds_w );
WRITE8_DEVICE_HANDLER( asteroid_noise_reset_w );
WRITE8_DEVICE_HANDLER( astdelux_sounds_w );


/*----------- defined in audio/llander.c -----------*/

DISCRETE_SOUND_EXTERN( llander );

WRITE8_DEVICE_HANDLER( llander_snd_reset_w );
WRITE8_DEVICE_HANDLER( llander_sounds_w );
