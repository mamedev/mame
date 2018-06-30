// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision controller port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_COLECO_CTRL_H
#define MAME_BUS_COLECO_CTRL_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COLECOVISION_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, COLECOVISION_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_COLECOVISION_CONTROL_PORT_IRQ_CALLBACK(_write) \
	devcb = &downcast<colecovision_control_port_device &>(*device).set_irq_wr_callback(DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class colecovision_control_port_device;


// ======================> device_colecovision_control_port_interface

class device_colecovision_control_port_interface : public device_slot_card_interface
{
public:
	virtual uint8_t joy_r() { return 0xff; }
	virtual void common0_w(int state) { m_common0 = state; }
	virtual void common1_w(int state) { m_common1 = state; }

protected:
	// construction/destruction
	device_colecovision_control_port_interface(const machine_config &mconfig, device_t &device);

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
	colecovision_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> devcb_base &set_irq_wr_callback(Object &&cb) { return m_write_irq.set_callback(std::forward<Object>(cb)); }

	// computer interface
	uint8_t read() { uint8_t data = 0xff; if (exists()) data = m_device->joy_r(); return data; }
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
DECLARE_DEVICE_TYPE(COLECOVISION_CONTROL_PORT, colecovision_control_port_device)

void colecovision_control_port_devices(device_slot_interface &device);


#endif // MAME_BUS_COLECO_CTRL_H
