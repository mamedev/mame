// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

  pcecommn.h

  Headers for the common stuff for NEC PC Engine/TurboGrafx16.

***************************************************************************/

#ifndef PCECOMMON_H
#define PCECOMMON_H

#include "cpu/h6280/h6280.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#define PCE_MAIN_CLOCK      21477270

class pce_common_state : public driver_device
{
public:
	pce_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_huc6260(*this, "huc6260") { }

	void pce_joystick_w(uint8_t data);
	uint8_t pce_joystick_r();

	void init_pce_common();

	required_device<h6280_device> m_maincpu;

	virtual uint8_t joy_read();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<huc6260_device> m_huc6260;

private:
	uint8_t m_io_port_options = 0;    /*driver-specific options for the PCE*/
	int m_joystick_port_select = 0; /* internal index of joystick ports */
	int m_joystick_data_select = 0; /* which nibble of joystick data we want */
};

// used by the Arcade bootlegs.
// Button II is actually on the left of a standard PCE joypad so we need to invert our button layout here.
#define PCE_STANDARD_INPUT_PORT_P1 \
	PORT_START( "JOY" ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button I") PORT_PLAYER(1) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button II") PORT_PLAYER(1) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Select") PORT_PLAYER(1) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )  PORT_NAME("P1 Run") PORT_PLAYER(1) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)


#endif
