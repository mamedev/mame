// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/*************************************************************************

    Atari Asteroids hardware

*************************************************************************/
#ifndef MAME_ATARI_ASTEROID_H
#define MAME_ATARI_ASTEROID_H

#pragma once

#include "machine/74153.h"
#include "machine/er2055.h"
#include "video/avgdvg.h"
#include "sound/discrete.h"

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
		m_ram2(*this, "ram2"),
		m_sram1(*this, "ram1", 0x100, ENDIANNESS_LITTLE),
		m_sram2(*this, "ram2", 0x100, ENDIANNESS_LITTLE)
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
	memory_share_creator<u8> m_sram1;
	memory_share_creator<u8> m_sram2;

	void coin_counter_left_w(int state);
	void coin_counter_center_w(int state);
	void coin_counter_right_w(int state);
	uint8_t asteroid_IN0_r(offs_t offset);
	uint8_t asterock_IN0_r(offs_t offset);
	uint8_t asteroid_IN1_r(offs_t offset);
	uint8_t asteroid_DSW1_r(offs_t offset);
	void asteroid_explode_w(uint8_t data);
	void asteroid_thump_w(uint8_t data);
	void asteroid_noise_reset_w(uint8_t data);
	void llander_snd_reset_w(uint8_t data);
	void llander_sounds_w(uint8_t data);

	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	int clock_r();

	INTERRUPT_GEN_MEMBER(asteroid_interrupt);
	INTERRUPT_GEN_MEMBER(asterock_interrupt);
	INTERRUPT_GEN_MEMBER(llander_interrupt);

	void cocktail_inv_w(int state);

	void init_asterock();
	void init_asteroidb();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void asteroid_base(machine_config &config);
	void asterock(machine_config &config);
	void asteroid(machine_config &config);
	void llander(machine_config &config);
	void astdelux(machine_config &config);
	void asteroid_sound(machine_config &config);
	void astdelux_sound(machine_config &config);
	void llander_sound(machine_config &config);
	void astdelux_map(address_map &map) ATTR_COLD;
	void asteroid_map(address_map &map) ATTR_COLD;
	void llander_map(address_map &map) ATTR_COLD;
};

#endif // MAME_ATARI_ASTEROID_H
