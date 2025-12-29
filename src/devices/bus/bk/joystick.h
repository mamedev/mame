// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Dual joystick interface

***************************************************************************/

#ifndef MAME_BUS_BK_JOYSTICK_H
#define MAME_BUS_BK_JOYSTICK_H

#pragma once

#include "parallel.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_joystick_device

class bk_joystick_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override {};
	virtual void device_reset() override {};

	virtual uint16_t io_r() override;

private:
	required_ioport m_joy;
};

// device type definition
DECLARE_DEVICE_TYPE(BK_JOYSTICK, bk_joystick_device)

#endif // MAME_BUS_BK_JOYSTICK_H
