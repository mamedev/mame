// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC SS-50 I/O port interface

**********************************************************************/

#pragma once

#ifndef MAME_DEVICES_BUS_SS50_INTERFACE_H
#define MAME_DEVICES_BUS_SS50_INTERFACE_H

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SS50_INTERFACE_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SS50_INTERFACE, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ss50_##_slot_intf, _def_slot, false)

#define MCFG_SS50_INTERFACE_IRQ_CALLBACK(_devcb) \
	downcast<ss50_interface_port_device &>(*device).set_irq_cb(DEVCB_##_devcb);

#define MCFG_SS50_INTERFACE_FIRQ_CALLBACK(_devcb) \
	downcast<ss50_interface_port_device &>(*device).set_firq_cb(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class ss50_card_interface;

// ======================> ss50_interface_port_device

class ss50_interface_port_device : public device_t, public device_slot_interface
{
	friend class ss50_card_interface;

public:
	// construction/destruction
	ss50_interface_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// static configuration
	template<class Object> devcb_base &set_irq_cb(Object &&object) { return m_irq_cb.set_callback(std::forward<Object>(object)); }
	template<class Object> devcb_base &set_firq_cb(Object &&object) { return m_firq_cb.set_callback(std::forward<Object>(object)); }

	// memory accesses
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	// baud rates
	DECLARE_WRITE_LINE_MEMBER(f110_w);
	DECLARE_WRITE_LINE_MEMBER(f150_9600_w);
	DECLARE_WRITE_LINE_MEMBER(f300_w);
	DECLARE_WRITE_LINE_MEMBER(f600_4800_w);
	DECLARE_WRITE_LINE_MEMBER(f600_1200_w);

protected:
	// device-specific overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

private:
	// output callbacks
	devcb_write_line m_irq_cb;
	devcb_write_line m_firq_cb;

	ss50_card_interface *m_card;
};


// ======================> ss50_card_interface

class ss50_card_interface : public device_slot_card_interface
{
	friend class ss50_interface_port_device;

protected:
	// construction/destruction
	ss50_card_interface(const machine_config &mconfig, device_t &device);

	// required overrides
	virtual DECLARE_READ8_MEMBER(register_read) = 0;
	virtual DECLARE_WRITE8_MEMBER(register_write) = 0;

	// optional overrides
	virtual DECLARE_WRITE_LINE_MEMBER(f110_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(f150_9600_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(f300_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(f600_4800_w) { }
	virtual DECLARE_WRITE_LINE_MEMBER(f600_1200_w) { }

	// IRQ/FIRQ/NMI outputs
	DECLARE_WRITE_LINE_MEMBER(write_irq) { m_slot->m_irq_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(write_firq) { m_slot->m_firq_cb(state); }

private:
	ss50_interface_port_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(SS50_INTERFACE, ss50_interface_port_device)

void ss50_default_2rs_devices(device_slot_interface &device);
//void ss50_default_4rs_devices(device_slot_interface &device);

#endif
