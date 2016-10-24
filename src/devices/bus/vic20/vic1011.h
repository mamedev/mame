// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore VIC-1011A/B RS-232C Adapter emulation

**********************************************************************/

#pragma once

#ifndef __VIC1011__
#define __VIC1011__

#include "user.h"
#include "bus/rs232/rs232.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1011_device

class vic1011_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	vic1011_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_pet_user_port_interface overrides
	virtual void input_d(int state) override;
	virtual void input_e(int state) override;
	virtual void input_j(int state) override;
	virtual void input_m(int state) override;

	void output_rxd(int state);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_device<rs232_port_device> m_rs232;
};


// device type definition
extern const device_type VIC1011;



#endif
