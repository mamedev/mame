// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

  pcecommn.h

  Headers for the common stuff for NEC PC Engine/TurboGrafx16.

***************************************************************************/

#ifndef PCECOMMON_H
#define PCECOMMON_H

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

	void pce_joystick_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pce_joystick_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void init_pce_common();

	required_device<cpu_device> m_maincpu;

	virtual uint8_t joy_read();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<huc6260_device> m_huc6260;

private:
	uint8_t m_io_port_options;    /*driver-specific options for the PCE*/
	int m_joystick_port_select; /* internal index of joystick ports */
	int m_joystick_data_select; /* which nibble of joystick data we want */
};
#endif
