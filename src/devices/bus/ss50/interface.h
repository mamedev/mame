// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC SS-50 I/O port interface

**********************************************************************/

#pragma once

#ifndef MAME_DEVICES_BUS_SS50_INTERFACE_H
#define MAME_DEVICES_BUS_SS50_INTERFACE_H


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
	virtual u8 register_read(offs_t offset) = 0;
	virtual void register_write(offs_t offset, u8 data) = 0;

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
