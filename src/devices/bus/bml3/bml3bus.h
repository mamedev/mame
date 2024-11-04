// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/***************************************************************************

  bml3bus.h - Hitachi MB-6890 slot bus and card emulation

  Adapted from a2bus by Jonathan Edwards

***************************************************************************/

#ifndef MAME_BUS_BML3_BML3BUS_H
#define MAME_BUS_BML3_BML3BUS_H

#pragma once


#define BML3BUS_MAX_SLOTS 6


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_device;
class device_bml3bus_card_interface;


class bml3bus_slot_device : public device_t,
							public device_single_card_slot_interface<device_bml3bus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	bml3bus_slot_device(machine_config const &mconfig, const char *tag, device_t *owner, T &&nbtag, U &&opts, const char *dflt)
		: bml3bus_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_bml3bus_slot(std::forward<T>(nbtag), tag);
	}

	bml3bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration
	template <typename T>
	void set_bml3bus_slot(T &&tag, const char *slottag)
	{
		m_bml3bus.set_tag(std::forward<T>(tag));
		m_bml3bus_slottag = slottag;
	}

protected:
	bml3bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<bml3bus_device> m_bml3bus;
	const char *m_bml3bus_slottag;
};

// device type definition
DECLARE_DEVICE_TYPE(BML3BUS_SLOT, bml3bus_slot_device)


// ======================> bml3bus_device
class bml3bus_device : public device_t
{
	friend class device_bml3bus_card_interface;
public:
	// construction/destruction
	bml3bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	auto nmi_callback() { return m_out_nmi_cb.bind(); }
	auto irq_callback() { return m_out_irq_cb.bind(); }
	auto firq_callback() { return m_out_firq_cb.bind(); }

	device_bml3bus_card_interface *get_bml3bus_card(int slot);

	void map_exrom(address_space_installer &space);
	void map_io(address_space_installer &space);

	void nmi_w(int state);
	void irq_w(int state);
	void firq_w(int state);

protected:
	bml3bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void add_bml3bus_card(int slot, device_bml3bus_card_interface &card);

	void set_nmi_line(int state);
	void set_irq_line(int state);
	void set_firq_line(int state);

	// line callbacks
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_firq_cb;

	device_bml3bus_card_interface *m_device_list[BML3BUS_MAX_SLOTS];
};


// device type definition
DECLARE_DEVICE_TYPE(BML3BUS, bml3bus_device)

// ======================> device_bml3bus_card_interface

// class representing interface-specific live bml3bus card
class device_bml3bus_card_interface : public device_interface
{
	friend class bml3bus_device;
public:
	// construction/destruction
	virtual ~device_bml3bus_card_interface();

	// inline configuration
	void set_bml3bus(bml3bus_device &bus, const char *slottag);

protected:
	virtual void interface_pre_start() override;

	virtual void map_exrom(address_space_installer &space) { }
	virtual void map_io(address_space_installer &space) { }

	void raise_slot_nmi()  { m_bml3bus->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi()  { m_bml3bus->set_nmi_line(CLEAR_LINE); }
	void raise_slot_irq()  { m_bml3bus->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq()  { m_bml3bus->set_irq_line(CLEAR_LINE); }
	void raise_slot_firq() { m_bml3bus->set_firq_line(ASSERT_LINE); }
	void lower_slot_firq() { m_bml3bus->set_firq_line(CLEAR_LINE); }
	void nmi_w(int state)  { m_bml3bus->set_nmi_line(state); }
	void irq_w(int state)  { m_bml3bus->set_irq_line(state); }
	void firq_w(int state) { m_bml3bus->set_firq_line(state); }

	device_bml3bus_card_interface(const machine_config &mconfig, device_t &device);

private:
	bml3bus_device *m_bml3bus;
	int m_slot;
};

#endif // MAME_BUS_BML3_BML3BUS_H
