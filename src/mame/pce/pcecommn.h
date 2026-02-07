// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

  pcecommn.h

  Headers for the common stuff for NEC PC Engine/TurboGrafx16.

***************************************************************************/

#ifndef MAME_PCE_PCECOMMN_H
#define MAME_PCE_PCECOMMN_H

#pragma once

#include "cpu/h6280/h6280.h"
#include "video/huc6260.h"
#include "video/huc6270.h"

class pce_common_state : public driver_device
{
public:
	void init_pce_common() ATTR_COLD;

protected:
	static constexpr XTAL PCE_MAIN_CLOCK = XTAL(21'477'272);

	pce_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_huc6260(*this, "huc6260")
		, m_io_joy(*this, "JOY")
	{ }

	virtual void machine_start() override ATTR_COLD;

	virtual u8 joy_read();

	void pce_joystick_w(u8 data);
	u8 pce_joystick_r();

	void common_cpu(machine_config &config) ATTR_COLD;
	void common_video(machine_config &config) ATTR_COLD;

	void common_mem_map(address_map &map) ATTR_COLD;
	void common_io_map(address_map &map) ATTR_COLD;

	required_device<h6280_device> m_maincpu;
	required_device<huc6260_device> m_huc6260;
	optional_ioport m_io_joy;

private:
	u8 m_io_port_options = 0;    /*driver-specific options for the PCE*/
	u8 m_joystick_port_select = 0; /* internal index of joystick ports */
	u8 m_joystick_data_select = 0; /* which nibble of joystick data we want */
};

// used by the Arcade bootlegs.
// Button II is actually on the left of a standard PCE joypad so we need to invert our button layout here.
#define PCE_STANDARD_INPUT_PORT_P1 \
	PORT_START("JOY") \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button I") PORT_PLAYER(1) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button II") PORT_PLAYER(1) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Select") PORT_PLAYER(1) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )  PORT_NAME("P1 Run") PORT_PLAYER(1) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)


#endif // MAME_PCE_PCECOMMN_H
