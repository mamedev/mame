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
//  FORWARD DECLARATIONS
//**************************************************************************

class acorn_bus_device;
class device_acorn_bus_interface;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_bus_slot_device : public device_t, public device_single_card_slot_interface<device_acorn_bus_interface>
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
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<acorn_bus_device> m_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(ACORN_BUS_SLOT, acorn_bus_slot_device)



// ======================> acorn_bus_device
class acorn_bus_device : public device_t
{
public:
	// construction/destruction
	acorn_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	address_space &memspace() const { return *m_space; }

	void irq_w(int state);
	void nmi_w(int state);

	void add_slot(acorn_bus_slot_device &slot);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	devcb_write_line m_out_irq_cb;
	devcb_write_line m_out_nmi_cb;

	std::forward_list<acorn_bus_slot_device *> m_slot_list;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_BUS, acorn_bus_device)

// ======================> device_acorn_bus_interface

// class representing interface-specific live acorn bus card
class device_acorn_bus_interface : public device_interface
{
public:
	friend class acorn_bus_device;

	// construction/destruction
	virtual ~device_acorn_bus_interface();

	// inline configuration
	void set_acorn_bus(acorn_bus_device &bus) { assert(!device().started()); m_bus = &bus; }

protected:
	device_acorn_bus_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	acorn_bus_device  *m_bus;
};


void acorn_bus_devices(device_slot_interface &device);
void atom_bus_devices(device_slot_interface &device);
void cms_bus_devices(device_slot_interface &device);


#endif // MAME_BUS_ACORN_BUS_H
