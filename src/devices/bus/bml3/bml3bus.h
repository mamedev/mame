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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BML3BUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BML3BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<bml3bus_slot_device &>(*device).set_bml3bus_slot(_nbtag, _tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_device;

class bml3bus_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	bml3bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	void set_bml3bus_slot(const char *tag, const char *slottag) { m_bml3bus_tag = tag; m_bml3bus_slottag = slottag; }

protected:
	bml3bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	const char *m_bml3bus_tag, *m_bml3bus_slottag;
};

// device type definition
DECLARE_DEVICE_TYPE(BML3BUS_SLOT, bml3bus_slot_device)


class device_bml3bus_card_interface;
// ======================> bml3bus_device
class bml3bus_device : public device_t
{
public:
	// construction/destruction
	bml3bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <class Object> void set_space(Object &&tag, int spacenum) { m_space.set_tag(std::forward<Object>(tag), spacenum); }
	auto nmi_callback() { return m_out_nmi_cb.bind(); }
	auto irq_callback() { return m_out_irq_cb.bind(); }
	auto firq_callback() { return m_out_firq_cb.bind(); }

	void add_bml3bus_card(int slot, device_bml3bus_card_interface *card);
	device_bml3bus_card_interface *get_bml3bus_card(int slot);

	void set_nmi_line(int state);
	void set_irq_line(int state);
	void set_firq_line(int state);

	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( firq_w );

	address_space &space() const { return *m_space; }

protected:
	bml3bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	required_address_space m_space;

	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_firq_cb;

	device_bml3bus_card_interface *m_device_list[BML3BUS_MAX_SLOTS];
};


// device type definition
DECLARE_DEVICE_TYPE(BML3BUS, bml3bus_device)

// ======================> device_bml3bus_card_interface

// class representing interface-specific live bml3bus card
class device_bml3bus_card_interface : public device_slot_card_interface
{
	friend class bml3bus_device;
public:
	// construction/destruction
	virtual ~device_bml3bus_card_interface();

	device_bml3bus_card_interface *next() const { return m_next; }

	void set_bml3bus_device();

	void raise_slot_nmi() { m_bml3bus->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_bml3bus->set_nmi_line(CLEAR_LINE); }
	void raise_slot_irq() { m_bml3bus->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_bml3bus->set_irq_line(CLEAR_LINE); }
	void raise_slot_firq() { m_bml3bus->set_firq_line(ASSERT_LINE); }
	void lower_slot_firq() { m_bml3bus->set_firq_line(CLEAR_LINE); }

	// inline configuration
	void set_bml3bus_tag(const char *tag, const char *slottag) { m_bml3bus_tag = tag; m_bml3bus_slottag = slottag; }

protected:
	device_bml3bus_card_interface(const machine_config &mconfig, device_t &device);

	bml3bus_device  *m_bml3bus;
	const char *m_bml3bus_tag, *m_bml3bus_slottag;
	int m_slot;
	device_bml3bus_card_interface *m_next;
};

#endif // MAME_BUS_BML3_BML3BUS_H
