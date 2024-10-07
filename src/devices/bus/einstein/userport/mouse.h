// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein Mouse

***************************************************************************/

#ifndef MAME_BUS_EINSTEIN_USERPORT_MOUSE_H
#define MAME_BUS_EINSTEIN_USERPORT_MOUSE_H

#pragma once

#include "userport.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> einstein_mouse_device

class einstein_mouse_device : public device_t, public device_einstein_userport_interface
{
public:
	// construction/destruction
	einstein_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read() override;

private:
	required_ioport m_mouse_b;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	int m_x;
	int m_y;
};

// device type definition
DECLARE_DEVICE_TYPE(EINSTEIN_MOUSE, einstein_mouse_device)

#endif // MAME_BUS_EINSTEIN_USERPORT_MOUSE_H
