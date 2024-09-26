// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore VIC-1011A/B RS-232C Adapter emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIC1011_H
#define MAME_BUS_VIC20_VIC1011_H

#pragma once

#include "user.h"
#include "bus/rs232/rs232.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1011_device

class vic1011_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	vic1011_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_pet_user_port_interface overrides
	virtual void input_d(int state) override;
	virtual void input_e(int state) override;
	virtual void input_j(int state) override;
	virtual void input_m(int state) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void output_rxd(int state);

	required_device<rs232_port_device> m_rs232;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1011, vic1011_device)

#endif // MAME_BUS_VIC20_VIC1011_H
