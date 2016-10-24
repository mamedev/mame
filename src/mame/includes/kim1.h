// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*************************************************************************

    includes/kim1.h

*************************************************************************/

#pragma once

#ifndef __KIM1__
#define __KIM1__

#include "emu.h"
#include "softlist.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "imagedev/cassette.h"
#include "formats/kim1_cas.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kim1_state : public driver_device
{
public:
	kim1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_riot2(*this, "miot_u2"),
		m_cass(*this, "cassette"),
		m_row0(*this, "ROW0"),
		m_row1(*this, "ROW1"),
		m_row2(*this, "ROW2"),
		m_special(*this, "SPECIAL") { }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mos6530_device> m_riot2;
	required_device<cassette_image_device> m_cass;
	uint8_t kim1_u2_read_a(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kim1_u2_write_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kim1_u2_read_b(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kim1_u2_write_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m_u2_port_b;
	uint8_t m_311_output;
	uint32_t m_cassette_high_count;
	uint8_t m_led_time[6];

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void trigger_reset(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void trigger_nmi(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void kim1_cassette_input(timer_device &timer, void *ptr, int32_t param);
	void kim1_update_leds(timer_device &timer, void *ptr, int32_t param);

protected:
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_special;
};

#endif /* KIM1_H */
