// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/*************************************************************************

    Atari Asteroids hardware

*************************************************************************/

#include "sound/discrete.h"
#include "video/avgdvg.h"
#include "machine/74153.h"
#include "machine/er2055.h"

class asteroid_state : public driver_device
{
public:
	asteroid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dvg(*this, "dvg"),
		m_earom(*this, "earom"),
		m_discrete(*this, "discrete"),
		m_dsw1(*this, "DSW1"),
		m_dsw_sel(*this, "dsw_sel"),
		m_cocktail(*this, "COCKTAIL"),
		m_ram1(*this, "ram1"),
		m_ram2(*this, "ram2")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<dvg_device> m_dvg;
	optional_device<er2055_device> m_earom;
	required_device<discrete_device> m_discrete;
	required_ioport m_dsw1;
	required_device<ttl153_device> m_dsw_sel;
	optional_ioport m_cocktail;

	/* memory banks */
	optional_memory_bank m_ram1;
	optional_memory_bank m_ram2;

	DECLARE_WRITE_LINE_MEMBER(coin_counter_left_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_center_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_right_w);
	DECLARE_READ8_MEMBER(asteroid_IN0_r);
	DECLARE_READ8_MEMBER(asterock_IN0_r);
	DECLARE_READ8_MEMBER(asteroid_IN1_r);
	DECLARE_READ8_MEMBER(asteroid_DSW1_r);
	DECLARE_WRITE8_MEMBER(asteroid_explode_w);
	DECLARE_WRITE8_MEMBER(asteroid_thump_w);
	DECLARE_WRITE8_MEMBER(asteroid_noise_reset_w);
	DECLARE_WRITE8_MEMBER(llander_snd_reset_w);
	DECLARE_WRITE8_MEMBER(llander_sounds_w);

	DECLARE_READ8_MEMBER(earom_read);
	DECLARE_WRITE8_MEMBER(earom_write);
	DECLARE_WRITE8_MEMBER(earom_control_w);

	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);

	INTERRUPT_GEN_MEMBER(asteroid_interrupt);
	INTERRUPT_GEN_MEMBER(asterock_interrupt);
	INTERRUPT_GEN_MEMBER(llander_interrupt);

	DECLARE_WRITE_LINE_MEMBER(cocktail_inv_w);

	void init_asterock();
	void init_asteroidb();

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void asteroid_base(machine_config &config);
	void asterock(machine_config &config);
	void asteroid(machine_config &config);
	void llander(machine_config &config);
	void astdelux(machine_config &config);
	void asteroid_sound(machine_config &config);
	void astdelux_sound(machine_config &config);
	void llander_sound(machine_config &config);
	void astdelux_map(address_map &map);
	void asteroid_map(address_map &map);
	void llander_map(address_map &map);
};
