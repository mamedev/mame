// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   Mattel Intellivision controller port emulation

**********************************************************************/

#ifndef MAME_BUS_INTV_CTRL_CTRL_H
#define MAME_BUS_INTV_CTRL_CTRL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class intv_control_port_device;

// ======================> device_intv_control_port_interface

class device_intv_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_intv_control_port_interface();

	virtual uint8_t read_ctrl() { return 0; };

protected:
	device_intv_control_port_interface(const machine_config &mconfig, device_t &device);

	intv_control_port_device *m_port;
};

// ======================> intv_control_port_device

class intv_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	intv_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: intv_control_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	intv_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~intv_control_port_device();

	DECLARE_READ8_MEMBER( ctrl_r ) { return read_ctrl(); }
	uint8_t read_ctrl();

protected:
	// device-level overrides
	virtual void device_start() override;

	device_intv_control_port_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(INTV_CONTROL_PORT, intv_control_port_device)

void intv_control_port_devices(device_slot_interface &device);


#endif // MAME_BUS_INTV_CTRL_CTRL_H
