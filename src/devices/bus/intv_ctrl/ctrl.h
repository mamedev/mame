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
	intv_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INTV_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, INTV_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



void intv_control_port_devices(device_slot_interface &device);


#endif // MAME_BUS_INTV_CTRL_CTRL_H
