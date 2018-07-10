// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Bus Extension emulation

**********************************************************************/

#ifndef MAME_BUS_ACORN_BUS_H
#define MAME_BUS_ACORN_BUS_H

#pragma once

#include <forward_list>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_bus_device;

class acorn_bus_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T, typename U>
	acorn_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&bus_tag, U &&opts, const char *dflt)
		: acorn_bus_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_bus.set_tag(std::forward<T>(bus_tag));
	}
	acorn_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// configuration
	required_device<device_t> m_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(ACORN_BUS_SLOT, acorn_bus_slot_device)


class device_acorn_bus_interface;

// ======================> acorn_bus_device
class acorn_bus_device : public device_t
{
public:
	// construction/destruction
	acorn_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_cputag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	address_space &memspace() const { return m_maincpu->space(AS_PROGRAM); }

	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_w);

	void add_slot(const char *tag);
	void add_slot(device_slot_interface *slot);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	required_device<cpu_device> m_maincpu;

	devcb_write_line m_out_irq_cb;
	devcb_write_line m_out_nmi_cb;

	std::forward_list<device_slot_interface *> m_slot_list;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_BUS, acorn_bus_device)

// ======================> device_acorn_bus_interface

// class representing interface-specific live acorn bus card
class device_acorn_bus_interface : public device_slot_card_interface
{
	friend class acorn_bus_device;
	template <class ElementType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_acorn_bus_interface();

	device_acorn_bus_interface *next() const { return m_next; }

	void set_acorn_bus_device();

	// inline configuration
	void set_acorn_bus(device_t *acorn_bus_device) { m_bus_dev = acorn_bus_device; }

public:
	device_acorn_bus_interface(const machine_config &mconfig, device_t &device);

	acorn_bus_device  *m_bus;
	device_t          *m_bus_dev;

private:
	device_acorn_bus_interface *m_next;
};


void acorn_bus_devices(device_slot_interface &device);
void atom_bus_devices(device_slot_interface &device);
void cms_bus_devices(device_slot_interface &device);


#endif // MAME_BUS_ACORN_BUS_H
