// license:BSD-3-Clause
/*********************************************************************

    a2wicotrackball.h

    Implemention of the Wico Apple II Trackball

*********************************************************************/

#ifndef MAME_DEVICES_A2BUS_A2WICO_TRACKBALL_H
#define MAME_DEVICES_A2BUS_A2WICO_TRACKBALL_H

#pragma once

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_wicotrackball_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_wicotrackball_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	a2bus_wicotrackball_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_ioport m_wicotrackballb;
	required_ioport_array<2> m_wicotrackballxy;

private:
	bool m_speed[2];
	uint8_t m_buttons;
	bool m_wraparound;
	uint8_t m_axis[2];
	uint32_t m_last_pos[2];
	uint8_t read_position(int axis);
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_WICOTRACKBALL, a2bus_wicotrackball_device)

#endif // MAME_DEVICES_A2BUS_A2WICO_TRACKBALL_H
