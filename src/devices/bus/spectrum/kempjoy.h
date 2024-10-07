// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Kempston Joystick Interface

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_KEMPJOY_H
#define MAME_BUS_SPECTRUM_KEMPJOY_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_kempjoy_device

class spectrum_kempjoy_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_kempjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t iorq_r(offs_t offset) override;

private:
	required_ioport m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_KEMPJOY, spectrum_kempjoy_device)


#endif // MAME_BUS_SPECTRUM_KEMPJOY_H
