// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/
#ifndef MAME_INCLUDES_BZONE_H
#define MAME_INCLUDES_BZONE_H

#pragma once

#include "audio/redbaron.h"
#include "machine/er2055.h"
#include "machine/mathbox.h"
#include "sound/discrete.h"
#include "screen.h"

#define BZONE_MASTER_CLOCK (XTAL(12'096'000))
#define BZONE_CLOCK_3KHZ   (BZONE_MASTER_CLOCK / 4096)

class bzone_state : public driver_device
{
public:
	bzone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mathbox(*this, "mathbox"),
		m_discrete(*this, "discrete"),
		m_screen(*this, "screen"),
		m_startled(*this, "startled")
	{ }

	DECLARE_READ_LINE_MEMBER(clock_r);
	void bzone(machine_config &config);
	void bradley(machine_config &config);

protected:
	void bzone_coin_counter_w(offs_t offset, uint8_t data);
	uint8_t analog_data_r();
	void analog_select_w(offs_t offset, uint8_t data);
	virtual void machine_start() override;
	INTERRUPT_GEN_MEMBER(bzone_interrupt);
	void bzone_sounds_w(uint8_t data);

	void bzone_base(machine_config &config);
	void bzone_audio(machine_config &config);
	void bzone_map(address_map &map);
	void bradley_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<mathbox_device> m_mathbox;
	optional_device<discrete_device> m_discrete;
	required_device<screen_device> m_screen;
	output_finder<> m_startled;

private:
	uint8_t m_analog_data;
};


class redbaron_state : public bzone_state
{
public:
	redbaron_state(const machine_config &mconfig, device_type type, const char *tag) :
		bzone_state(mconfig, type, tag),
		m_earom(*this, "earom"),
		m_redbaronsound(*this, "custom"),
		m_fake_ports(*this, "FAKE%u", 1U)
	{ }

	void redbaron(machine_config &config);

protected:
	uint8_t redbaron_joy_r();
	void redbaron_joysound_w(uint8_t data);
	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void redbaron_map(address_map &map);

private:
	required_device<er2055_device> m_earom;
	required_device<redbaron_sound_device> m_redbaronsound;
	required_ioport_array<2> m_fake_ports;
	uint8_t m_rb_input_select;
};

#endif // MAME_INCLUDES_BZONE_H
