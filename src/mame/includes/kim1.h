// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*************************************************************************

    includes/kim1.h

*************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_KIM1_H
#define MAME_INCLUDES_KIM1_H

#include "softlist.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "machine/timer.h"
#include "imagedev/cassette.h"
#include "formats/kim1_cas.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kim1_state : public driver_device
{
public:
	kim1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_riot2(*this, "miot_u2"),
		m_cass(*this, "cassette"),
		m_row(*this, "ROW%u", 0U),
		m_special(*this, "SPECIAL"),
		m_digit(*this, "digit%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);
	void kim1(machine_config &config);

private:
	DECLARE_READ8_MEMBER(kim1_u2_read_a);
	DECLARE_WRITE8_MEMBER(kim1_u2_write_a);
	DECLARE_READ8_MEMBER(kim1_u2_read_b);
	DECLARE_WRITE8_MEMBER(kim1_u2_write_b);

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(kim1_cassette_input);
	TIMER_DEVICE_CALLBACK_MEMBER(kim1_update_leds);

	void kim1_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mos6530_device> m_riot2;
	required_device<cassette_image_device> m_cass;

	required_ioport_array<3> m_row;
	required_ioport m_special;
	output_finder<6> m_digit;

	uint8_t m_u2_port_b;
	uint8_t m_311_output;
	uint32_t m_cassette_high_count;
	uint8_t m_led_time[6];
};

#endif // MAME_INCLUDES_KIM1_H
