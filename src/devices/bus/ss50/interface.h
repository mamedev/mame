// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC SS-50 I/O port interface

**********************************************************************/

#ifndef MAME_BUS_SS50_INTERFACE_H
#define MAME_BUS_SS50_INTERFACE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class ss50_card_interface;

// ======================> ss50_interface_port_device

class ss50_interface_port_device : public device_t, public device_single_card_slot_interface<ss50_card_interface>
{
	friend class ss50_card_interface;

public:
	// construction/destruction
	template <typename T>
	ss50_interface_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: ss50_interface_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	ss50_interface_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// static configuration
	auto irq_cb() { return m_irq_cb.bind(); }
	auto firq_cb() { return m_firq_cb.bind(); }

	// memory accesses
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// baud rates
	void f110_w(int state);
	void f150_9600_w(int state);
	void f300_w(int state);
	void f600_4800_w(int state);
	void f600_1200_w(int state);

protected:
	// device-specific overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// output callbacks
	devcb_write_line m_irq_cb;
	devcb_write_line m_firq_cb;

	ss50_card_interface *m_card;
};


// ======================> ss50_card_interface

class ss50_card_interface : public device_interface
{
	friend class ss50_interface_port_device;

protected:
	// construction/destruction
	ss50_card_interface(const machine_config &mconfig, device_t &device);

	// required overrides
	virtual u8 register_read(offs_t offset) = 0;
	virtual void register_write(offs_t offset, u8 data) = 0;

	// optional overrides
	virtual void f110_w(int state) { }
	virtual void f150_9600_w(int state) { }
	virtual void f300_w(int state) { }
	virtual void f600_4800_w(int state) { }
	virtual void f600_1200_w(int state) { }

	// IRQ/FIRQ/NMI outputs
	void write_irq(int state) { m_slot->m_irq_cb(state); }
	void write_firq(int state) { m_slot->m_firq_cb(state); }

private:
	virtual void interface_pre_start() override;

	ss50_interface_port_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(SS50_INTERFACE, ss50_interface_port_device)

void ss50_default_2rs_devices(device_slot_interface &device);
//void ss50_default_4rs_devices(device_slot_interface &device);

#endif // MAME_BUS_SS50_INTERFACE_H
