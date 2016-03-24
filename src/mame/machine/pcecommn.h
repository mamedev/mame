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

	DECLARE_WRITE8_MEMBER(pce_joystick_w);
	DECLARE_READ8_MEMBER(pce_joystick_r);

	DECLARE_DRIVER_INIT(pce_common);

	required_device<cpu_device> m_maincpu;

	virtual UINT8 joy_read();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<huc6260_device> m_huc6260;
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
private:
	UINT8 m_io_port_options;    /*driver-specific options for the PCE*/
	int m_joystick_port_select; /* internal index of joystick ports */
	int m_joystick_data_select; /* which nibble of joystick data we want */
};
#endif
