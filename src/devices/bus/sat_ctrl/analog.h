// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Analog Controller emulation

**********************************************************************/

#pragma once

#ifndef __SATURN_ANALOG__
#define __SATURN_ANALOG__


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_analog_device

class saturn_analog_device : public device_t,
							public device_saturn_control_port_interface
{
public:
	// construction/destruction
	saturn_analog_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_saturn_control_port_interface overrides
	virtual uint8_t read_ctrl(uint8_t offset) override;
	virtual uint8_t read_status() override { return 0xf1; }
	virtual uint8_t read_id(int idx) override { return m_ctrl_id; }

private:
	required_ioport m_joy;
	required_ioport m_anx;
	required_ioport m_any;
	required_ioport m_anz;
};


// device type definition
extern const device_type SATURN_ANALOG;


#endif
