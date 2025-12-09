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
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

private:
	// configuration
	required_device<acorn_bus_device> m_bus;
	device_acorn_bus_interface *m_card;
};

// device type definition
DECLARE_DEVICE_TYPE(ACORN_BUS_SLOT, acorn_bus_slot_device)


// ======================> acorn_bus_device

class acorn_bus_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	acorn_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~acorn_bus_device() { m_device_list.detach_all(); }

	void add_card(device_acorn_bus_interface &card);

	// callbacks
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }
	auto cb1_handler() { return m_cb1_handler.bind(); }
	auto cb2_handler() { return m_cb2_handler.bind(); }

	address_space &memspace() const { return *m_space; }

	void set_blk0(uint8_t blk0) { m_blk0 = blk0 & 0x0f; }
	uint8_t blk0() { return m_blk0; }

	void irq_w(int state) { m_out_irq_cb(state); }
	void nmi_w(int state) { m_out_nmi_cb(state); }

	// from slot
	void cb1_w(int state) { m_cb1_handler(state); }
	void cb2_w(int state) { m_cb2_handler(state); }

	// from host
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t pb_r();
	void pb_w(uint8_t data);
	void write_cb1(int state);
	void write_cb2(int state);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	// internal state
	address_space_config m_space_config;

	address_space *m_space;

	devcb_write_line m_out_irq_cb;
	devcb_write_line m_out_nmi_cb;
	devcb_write_line m_cb1_handler;
	devcb_write_line m_cb2_handler;

	uint8_t m_blk0 = 0x00;

	simple_list<device_acorn_bus_interface> m_device_list;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_BUS, acorn_bus_device)

// ======================> device_acorn_bus_interface

class device_acorn_bus_interface : public device_interface
{
	friend class acorn_bus_device;
	template <class ElementType> friend class simple_list;

public:
	device_acorn_bus_interface *next() const { return m_next; }

	virtual uint8_t pb_r() { return 0xff; }
	virtual void pb_w(uint8_t data) { }
	virtual void write_cb1(int state) { }
	virtual void write_cb2(int state) { }

protected:
	device_acorn_bus_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	acorn_bus_device *m_bus;
	acorn_bus_slot_device *const m_slot;

private:
	device_acorn_bus_interface *m_next;
};


void acorn_bus_devices(device_slot_interface &device);
void atom_bus_devices(device_slot_interface &device);
void atom_pl8_devices(device_slot_interface &device);
void eurocube_bus_devices(device_slot_interface &device);
void cms_bus_devices(device_slot_interface &device);


#endif // MAME_BUS_ACORN_BUS_H
