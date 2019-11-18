// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    4play.h

    4 player digital joystick card for the Apple II by Lukazi
    http://lukazi.blogspot.com/2016/05/apple-ii-4play-joystick-card-revb.html

*********************************************************************/

#ifndef MAME_BUS_A2BUS_4PLAY_H
#define MAME_BUS_A2BUS_4PLAY_H

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_4play_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_4play_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_4play_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t read_c0nx(uint8_t offset) override;

	required_ioport m_p1, m_p2, m_p3, m_p4;

private:
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_4PLAY, a2bus_4play_device)

#endif // MAME_BUS_A2BUS_4PLAY_H
