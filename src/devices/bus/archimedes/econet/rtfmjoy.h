// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes RTFM Joystick Interface

**********************************************************************/


#ifndef MAME_BUS_ARCHIMEDES_ECONET_RTFMJOY_H
#define MAME_BUS_ARCHIMEDES_ECONET_RTFMJOY_H

#pragma once


#include "slot.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class arc_rtfm_joystick_device:
	public device_t,
	public device_archimedes_econet_interface
{
public:
	// construction/destruction
	arc_rtfm_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 read(offs_t offset) override;

private:
	required_ioport_array<2> m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(ARC_RTFM_JOY, arc_rtfm_joystick_device);


#endif /* MAME_BUS_ARCHIMEDES_ECONET_RTFMJOY_H */
