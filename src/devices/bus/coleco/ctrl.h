// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision controller port emulation

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __COLECOVISION_CONTROL_PORT__
#define __COLECOVISION_CONTROL_PORT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COLECOVISION_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, COLECOVISION_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_COLECOVISION_CONTROL_PORT_IRQ_CALLBACK(_write) \
	devcb = &colecovision_control_port_device::set_irq_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class colecovision_control_port_device;


// ======================> device_colecovision_control_port_interface

class device_colecovision_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_colecovision_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_colecovision_control_port_interface() { }

	virtual UINT8 joy_r() { return 0xff; };
	virtual void common0_w(int state) { m_common0 = state; };
	virtual void common1_w(int state) { m_common1 = state; };

protected:
	colecovision_control_port_device *m_port;

	int m_common0;
	int m_common1;
};


// ======================> colecovision_control_port_device

class colecovision_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	colecovision_control_port_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~colecovision_control_port_device() { }

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<colecovision_control_port_device &>(device).m_write_irq.set_callback(object); }

	// computer interface
	UINT8 read() { UINT8 data = 0xff; if (exists()) data = m_device->joy_r(); return data; }
	DECLARE_READ8_MEMBER( read ) { return read(); }

	DECLARE_WRITE_LINE_MEMBER( common0_w ) { if (exists()) m_device->common0_w(state); }
	DECLARE_WRITE_LINE_MEMBER( common1_w ) { if (exists()) m_device->common1_w(state); }

	bool exists() { return m_device != nullptr; }

	void irq_w(int state) { m_write_irq(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_colecovision_control_port_interface *m_device;

private:
	devcb_write_line m_write_irq;
};


// device type definition
extern const device_type COLECOVISION_CONTROL_PORT;

SLOT_INTERFACE_EXTERN( colecovision_control_port_devices );



#endif
