// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2eauxslot.h - Apple IIe auxiliary slot and card emulation

  by R. Belmont

***************************************************************************/

#ifndef MAME_BUS_A2BUS_A2EAUXSLOT_H
#define MAME_BUS_A2BUS_A2EAUXSLOT_H

#pragma once

#include "a2bus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2eauxslot_device;
class device_a2eauxslot_card_interface;


class a2eauxslot_slot_device : public device_t,
							public device_single_card_slot_interface<device_a2eauxslot_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	a2eauxslot_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slottag, U &&opts, char const *dflt)
		: a2eauxslot_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_a2eauxslot.set_tag(std::forward<T>(slottag));
	}

	a2eauxslot_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override { }

protected:
	a2eauxslot_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	required_device<a2eauxslot_device> m_a2eauxslot;
};

// device type definition
DECLARE_DEVICE_TYPE(A2EAUXSLOT_SLOT, a2eauxslot_slot_device)


// ======================> a2eauxslot_device
class a2eauxslot_device : public device_t
{
public:
	// construction/destruction
	a2eauxslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	void add_a2eauxslot_card(device_a2eauxslot_card_interface *card);
	device_a2eauxslot_card_interface *get_a2eauxslot_card();

	void set_irq_line(int state);
	void set_nmi_line(int state);

	void irq_w(int state);
	void nmi_w(int state);

protected:
	a2eauxslot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;

	device_a2eauxslot_card_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(A2EAUXSLOT, a2eauxslot_device)

// ======================> device_a2eauxslot_card_interface

// class representing interface-specific live a2eauxslot card
class device_a2eauxslot_card_interface : public device_interface
{
	friend class a2eauxslot_device;
public:
	// construction/destruction
	virtual ~device_a2eauxslot_card_interface();

	virtual u8 read_auxram(u16 offset) { printf("a2eauxslot: unhandled auxram read @ %04x\n", offset); return 0xff; }
	virtual void write_auxram(u16 offset, u8 data) { printf("a2eauxslot: unhandled auxram write %02x @ %04x\n", data, offset); }
	virtual void write_c07x(u8 offset, u8 data) {}
	virtual u8 *get_vram_ptr() = 0;
	virtual u8 *get_auxbank_ptr() = 0;
	virtual u16 get_auxbank_mask() = 0;
	virtual bool allow_dhr() { return true; }

	device_a2eauxslot_card_interface *next() const { return m_next; }

	// inline configuration
	void set_a2eauxslot(a2eauxslot_device *a2eauxslot, const char *slottag) { m_a2eauxslot = a2eauxslot; m_a2eauxslot_slottag = slottag; }
	template <typename T> void set_onboard(T &&a2eauxslot) { m_a2eauxslot_finder.set_tag(std::forward<T>(a2eauxslot)); m_a2eauxslot_slottag = device().tag(); }

	void raise_slot_irq() { m_a2eauxslot->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_a2eauxslot->set_irq_line(CLEAR_LINE); }
	void raise_slot_nmi() { m_a2eauxslot->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_a2eauxslot->set_nmi_line(CLEAR_LINE); }

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

protected:
	device_a2eauxslot_card_interface(const machine_config &mconfig, device_t &device);

	optional_device<a2eauxslot_device> m_a2eauxslot_finder;
	a2eauxslot_device  *m_a2eauxslot;
	const char *m_a2eauxslot_slottag;
	int m_slot;
	device_a2eauxslot_card_interface *m_next;
};

#endif  // MAME_BUS_A2BUS_A2EAUXSLOT_H
