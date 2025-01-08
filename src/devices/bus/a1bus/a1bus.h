// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a1bus.h - Apple I expansion slot and card emulation

***************************************************************************/

#ifndef MAME_BUS_A1BUS_A1BUS_H
#define MAME_BUS_A1BUS_A1BUS_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1bus_device;
class device_a1bus_card_interface;

class a1bus_slot_device : public device_t, public device_single_card_slot_interface<device_a1bus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	a1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&a1bus_tag, U &&opts, const char *dflt)
		: a1bus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_a1bus.set_tag(std::forward<T>(a1bus_tag));
	}
	a1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a1bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<a1bus_device> m_a1bus;
};

// device type definition
DECLARE_DEVICE_TYPE(A1BUS_SLOT, a1bus_slot_device)


// ======================> a1bus_device
class a1bus_device : public device_t
{
public:
	// construction/destruction
	a1bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	void add_a1bus_card(device_a1bus_card_interface *card);
	device_a1bus_card_interface *get_a1bus_card();

	void set_irq_line(int state);
	void set_nmi_line(int state);

	void install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_bank(offs_t start, offs_t end, uint8_t *data);

	void irq_w(int state);
	void nmi_w(int state);

protected:
	a1bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;

	device_a1bus_card_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(A1BUS, a1bus_device)

// ======================> device_a1bus_card_interface

// class representing interface-specific live a1bus card
class device_a1bus_card_interface : public device_interface
{
	friend class a1bus_device;
public:
	// construction/destruction
	virtual ~device_a1bus_card_interface();

	// inline configuration
	void set_a1bus(a1bus_device *a1bus, const char *slottag) { m_a1bus = a1bus; m_a1bus_slottag = slottag; }
	template <typename T> void set_onboard(T &&a1bus) { m_a1bus_finder.set_tag(std::forward<T>(a1bus)); m_a1bus_slottag = device().tag(); }

protected:
	void raise_slot_irq() { m_a1bus->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_a1bus->set_irq_line(CLEAR_LINE); }
	void raise_slot_nmi() { m_a1bus->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_a1bus->set_nmi_line(CLEAR_LINE); }

	void install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_bank(offs_t start, offs_t end, uint8_t *data);

	device_a1bus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

private:
	optional_device<a1bus_device> m_a1bus_finder;
	a1bus_device *m_a1bus;
	const char *m_a1bus_slottag;
};

#endif  // MAME_BUS_A1BUS_A1BUS_H
