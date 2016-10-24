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
	asteroid_state(const machine_config &mconfig, device_type type, const char *tag)
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

	void astdelux_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void llander_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t asteroid_IN0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t asterock_IN0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t asteroid_IN1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t asteroid_DSW1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void asteroid_bank_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void astdelux_bank_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void astdelux_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asteroid_explode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asteroid_thump_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asteroid_sounds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void astdelux_sounds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asteroid_noise_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void llander_snd_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void llander_sounds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value clock_r(ioport_field &field, void *param);

	void asteroid_interrupt(device_t &device);
	void asterock_interrupt(device_t &device);
	void llander_interrupt(device_t &device);

	void init_asterock();
	void init_asteroidb();

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

/*----------- defined in audio/asteroid.c -----------*/

DISCRETE_SOUND_EXTERN( asteroid );
DISCRETE_SOUND_EXTERN( astdelux );
