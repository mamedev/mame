// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Protek Joystick Interface

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_PROTEK_H
#define MAME_BUS_SPECTRUM_PROTEK_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_protek_device

class spectrum_protek_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_protek_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_READ8_MEMBER(port_fe_r) override;

private:
	required_ioport m_exp_line3;
	required_ioport m_exp_line4;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_PROTEK, spectrum_protek_device)


#endif // MAME_BUS_SPECTRUM_PROTEK_H
