// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 controller port emulation

**********************************************************************/

#ifndef MAME_BUS_PCE_CTRL_PCECTRL_H
#define MAME_BUS_PCE_CTRL_PCECTRL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_control_port_device

class device_pce_control_port_interface;

class pce_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	pce_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pce_control_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	pce_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~pce_control_port_device();

	u8 port_r();

	void sel_w(int state);
	void clr_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_pce_control_port_interface *m_device;
};


// ======================> device_pce_control_port_interface

// class representing interface-specific live pce_control_port card
class device_pce_control_port_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pce_control_port_interface();

	virtual u8 peripheral_r() { return 0xf; }
	virtual void sel_w(int state) {}
	virtual void clr_w(int state) {}

protected:
	device_pce_control_port_interface(const machine_config &mconfig, device_t &device);

	pce_control_port_device *m_port;
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_CONTROL_PORT, pce_control_port_device)


void pce_control_port_devices(device_slot_interface &device);


#endif // MAME_BUS_PCE_CTRL_PCECTRL_H
