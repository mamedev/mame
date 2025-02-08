// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_CENTRONICS_ADAPTATOR_H
#define MAME_BUS_CENTRONICS_ADAPTATOR_H

#pragma once

#include "ctronics.h"
#include "bus/vcs_ctrl/ctrl.h"

class adaptator_multitap_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	adaptator_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_strobe(int state) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<vcs_control_port_device, 2> m_joy;
};

// device type definition
DECLARE_DEVICE_TYPE(ADAPTATOR_MULTITAP, adaptator_multitap_device)


#endif // MAME_BUS_CENTRONICS_ADAPTATOR_H
